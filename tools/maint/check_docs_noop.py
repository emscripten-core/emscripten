#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Verifies that changes to documentation files are visual/semantic no-ops.

Compares the rendered documentation output (HTML or text) of the current
working directory against a base git revision. For HTML output, it normalizes
whitespace in prose elements while preserving exact formatting in code blocks
and inline literals, eliminating false positives from line re-wrapping.
"""

import argparse
import difflib
import os
import shutil
import subprocess
import sys
from html.parser import HTMLParser
from pathlib import Path

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))


class NormalizedHTMLParser(HTMLParser):
  """Parses HTML and normalizes whitespace outside of pre/code blocks."""

  def __init__(self):
    super().__init__()
    self.lines = []
    self.in_pre = 0

  def handle_starttag(self, tag, attrs):
    if tag in {'pre', 'code'}:
      self.in_pre += 1
    sorted_attrs = sorted((k, v) for k, v in attrs if k != 'href' or not v.startswith('#'))
    attr_str = ' '.join(f'{k}="{v}"' if v is not None else k for k, v in sorted_attrs)
    tag_str = f'<{tag} {attr_str}>' if attr_str else f'<{tag}>'
    self.lines.append(tag_str)

  def handle_endtag(self, tag):
    if tag in {'pre', 'code'}:
      self.in_pre -= 1
      assert self.in_pre >= 0
    self.lines.append(f'</{tag}>')

  def handle_data(self, data):
    if self.in_pre > 0:
      self.lines.append(data)
    else:
      norm = ' '.join(data.split())
      if norm:
        self.lines.append(norm)


def normalize_html(html_content):
  parser = NormalizedHTMLParser()
  parser.feed(html_content)
  return parser.lines


def run_command(cmd, **kwargs):
  return subprocess.run(cmd, check=True, **kwargs)


def get_modified_rst_files(base_ref):
  cmd = ['git', 'diff', '--name-only', base_ref, '--', 'site/source/*.rst', 'site/source/**/*.rst']
  res = subprocess.run(cmd, stdout=subprocess.PIPE, text=True, check=True, cwd=root_dir)
  files = [f.strip() for f in res.stdout.splitlines() if f.strip().endswith('.rst')]
  return files


def rst_to_output_relpath(rst_file, builder_format):
  rel = os.path.relpath(rst_file, os.path.join('site', 'source'))
  ext = '.html' if builder_format == 'html' else '.txt'
  return os.path.splitext(rel)[0] + ext


def build_docs(source_dir, output_dir, builder_format):
  sphinx_build = os.environ.get('SPHINXBUILD', 'sphinx-build')
  cmd = [sphinx_build, '-b', builder_format, '-q', source_dir, output_dir]
  run_command(cmd)


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--base', default='@{upstream}', help='Base git revision to compare against (default: @{upstream})')
  parser.add_argument('--format', choices=['html', 'text'], default='html', help='Doc builder format to compare (default: html)')
  parser.add_argument('files', nargs='*', help='Specific .rst files to check (default: all modified .rst files)')
  args = parser.parse_args()

  if args.files:
    rst_files = [os.path.relpath(f, root_dir) for f in args.files]
  else:
    rst_files = get_modified_rst_files(args.base)

  if not rst_files:
    print(f'No modified .rst files found compared to {args.base}.')
    return 0

  print(f'Checking {len(rst_files)} documentation file(s) against {args.base} ({args.format} format)...')

  out_dir = os.path.join(root_dir, 'out', 'check_docs')
  curr_out = os.path.join(out_dir, 'curr')
  base_out = os.path.join(out_dir, 'base')
  base_worktree = os.path.join(out_dir, 'worktree')

  if os.path.exists(base_worktree):
    subprocess.run(['git', 'worktree', 'remove', '--force', base_worktree], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=root_dir)
  if os.path.exists(out_dir):
    shutil.rmtree(out_dir, ignore_errors=True)
  os.makedirs(out_dir)

  print('Building current working tree docs...')
  build_docs(os.path.join(root_dir, 'site', 'source'), curr_out, args.format)

  print(f'Checking out base revision ({args.base}) into worktree...')
  run_command(['git', 'worktree', 'add', '-q', '--detach', base_worktree, args.base], cwd=root_dir)
  print(f'Building base ({args.base}) docs...')
  build_docs(os.path.join(base_worktree, 'site', 'source'), base_out, args.format)

  has_diff = False
  for rst_file in rst_files:
    rel_out = rst_to_output_relpath(rst_file, args.format)
    curr_file = os.path.join(curr_out, rel_out)
    base_file = os.path.join(base_out, rel_out)

    if not os.path.exists(curr_file) or not os.path.exists(base_file):
      print(f'ERROR: Output file missing for {rst_file} ({rel_out})')
      has_diff = True
      continue

    curr_content = Path(curr_file).read_text(encoding='utf-8')
    base_content = Path(base_file).read_text(encoding='utf-8')

    if args.format == 'html':
      curr_lines = normalize_html(curr_content)
      base_lines = normalize_html(base_content)
    else:
      curr_lines = curr_content.splitlines()
      base_lines = base_content.splitlines()

    if curr_lines != base_lines:
      has_diff = True
      print(f'\nFAIL: Rendered {args.format} output changed for {rst_file}:')
      diff = difflib.unified_diff(
        base_lines,
        curr_lines,
        fromfile=f'{args.base}:{rel_out}',
        tofile=f'working_tree:{rel_out}',
        lineterm='',
      )
      for line in diff:
        print(line)

  if os.path.exists(base_worktree):
    subprocess.run(['git', 'worktree', 'remove', '--force', base_worktree], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=root_dir)
  if os.path.exists(out_dir):
    shutil.rmtree(out_dir, ignore_errors=True)

  if has_diff:
    print('\nDocumentation changes are NOT no-ops!')
    return 1

  print('\nSUCCESS: Rendered documentation output is identical (no-op).')
  return 0


if __name__ == '__main__':
  sys.exit(main())
