#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# SPDX-License-Identifier: Apache-2.0

"""Download the Cross-Origin Storage Chrome extension for COS browser tests.

The COS extension polyfills navigator.crossOriginStorage in Chrome so that
automated browser tests can exercise the cache-miss and cache-hit paths without
requiring a native browser implementation of the API.

Usage
-----

Run once to download and unpack the extension::

  python3 test/setup_cos_extension.py

Then pass the printed path as EMTEST_COS_EXTENSION_PATH when running the
browser tests::

  EMTEST_COS_EXTENSION_PATH=$(python3 test/setup_cos_extension.py) \\
    python3 test/runner.py browser.test_cross_origin_storage_miss_then_hit

Or, with --print-path suppressed so only the path is printed (suitable for
shell variable assignment), use --quiet::

  ext=$(python3 test/setup_cos_extension.py --quiet)
  EMTEST_COS_EXTENSION_PATH=$ext python3 test/runner.py \\
      browser.test_cross_origin_storage_fallback \\
      browser.test_cross_origin_storage_miss_then_hit

The extension is downloaded from its GitHub source repository and unpacked into
out/cos_extension/ (relative to the Emscripten root).  Re-run with --force to
refresh an existing download.

Source: https://github.com/web-ai-community/cross-origin-storage-extension
"""

import argparse
import io
import os
import sys
import urllib.request
import zipfile

# Archive of the main branch of the COS extension source repository.
EXTENSION_ARCHIVE_URL = (
    'https://github.com/web-ai-community/cross-origin-storage-extension'
    '/archive/refs/heads/main.zip'
)

# Default destination relative to the Emscripten root (i.e. two levels up from
# this script, which lives in test/).
_SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
DEFAULT_DEST = os.path.join(_SCRIPT_DIR, '..', 'out', 'cos_extension')


def _find_manifest(root):
    """Return the directory containing manifest.json, searching up to 2 levels.

    Returns None if root does not exist or no manifest.json is found.
    """
    if not os.path.isdir(root):
        return None
    if os.path.exists(os.path.join(root, 'manifest.json')):
        return root
    for entry in os.scandir(root):
        if entry.is_dir():
            candidate = os.path.join(entry.path, 'manifest.json')
            if os.path.exists(candidate):
                return entry.path
    return None


def download_and_unpack(dest_dir, quiet=False):
    """Download the extension archive from GitHub and unpack it into dest_dir.

    Returns the path of the directory that contains manifest.json.
    """
    if not quiet:
        print('Downloading COS extension from GitHub...', file=sys.stderr)
    req = urllib.request.Request(
        EXTENSION_ARCHIVE_URL,
        headers={'User-Agent': 'emscripten-test-setup'},
    )
    with urllib.request.urlopen(req) as response:
        data = response.read()

    if not quiet:
        print(f'Unpacking to {dest_dir} ...', file=sys.stderr)
    os.makedirs(dest_dir, exist_ok=True)

    with zipfile.ZipFile(io.BytesIO(data)) as zf:
        # GitHub archives wrap everything in a top-level directory, e.g.
        # "cross-origin-storage-extension-main/".  Strip that prefix.
        names = zf.namelist()
        prefix = names[0].split('/')[0] + '/' if names else ''
        for member in names:
            if member == prefix:
                continue
            rel = member[len(prefix):]
            if not rel:
                continue
            target = os.path.join(dest_dir, rel)
            if member.endswith('/'):
                os.makedirs(target, exist_ok=True)
            else:
                os.makedirs(os.path.dirname(target), exist_ok=True)
                with zf.open(member) as src, open(target, 'wb') as dst:
                    dst.write(src.read())

    extension_dir = _find_manifest(dest_dir)
    if extension_dir is None:
        print(
            f'ERROR: manifest.json not found anywhere under {dest_dir}.\n'
            'The extension repository structure may have changed.\n'
            'Please report this at https://github.com/emscripten-core/emscripten/issues',
            file=sys.stderr,
        )
        sys.exit(1)

    return extension_dir


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        '--dest',
        default=DEFAULT_DEST,
        help='Directory to unpack the extension into (default: out/cos_extension)',
    )
    parser.add_argument(
        '--force',
        action='store_true',
        help='Re-download even if the extension is already present',
    )
    parser.add_argument(
        '--quiet',
        action='store_true',
        help='Suppress informational messages; print only the extension path',
    )
    args = parser.parse_args()

    dest = os.path.realpath(args.dest)

    extension_dir = _find_manifest(dest)
    if extension_dir and not args.force:
        if not args.quiet:
            print(
                f'Extension already present at {extension_dir} '
                f'(use --force to re-download)',
                file=sys.stderr,
            )
    else:
        extension_dir = download_and_unpack(dest, quiet=args.quiet)
        if not args.quiet:
            print(f'Extension ready at {extension_dir}', file=sys.stderr)

    # Always print the path as the last line so callers can capture it with
    # $(...) or --quiet.
    print(extension_dir)


if __name__ == '__main__':
    main()
