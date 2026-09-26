#!/usr/bin/env python3
# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

#
# This script gets the Emscripten wiki, converts files from markdown to
# restructure text using pandoc (and also prepends the text with a title/heading
# based on the filename)
#
# It also fixes up inline code items to become links to api-reference
#
# It should be called prior to building the site.
#

import optparse
import os
import re
import shutil
import stat
import sys
import time
from pathlib import Path

import api_items

wiki_repo = 'https://github.com/emscripten-core/emscripten.wiki.git'
output_dir = './wiki_static/'
logfilename = 'log-get-wiki.txt'

# GetmMap of code items from api_items.py
mapped_wiki_inline_code = api_items.get_mapped_items()
# Add any additional mapped items that seem reasonable.
mapped_wiki_inline_code['getValue(ptr, type)'] = ':js:func:`getValue(ptr, type) <getValue>`'
mapped_wiki_inline_code['setValue(ptr, value, type)'] = ':js:func:`setValue(ptr, value, type) <setValue>`'

wiki_checkout = 'emscripten.wiki/'
temp_set_of_codemarkup = set()
logfile = open(logfilename, 'w', encoding='utf-8')
time_str = time.strftime("%a, %d %b %Y %H:%M", time.gmtime())
snapshot_version_information = f'.. note:: This article was migrated from the wiki ({time_str}) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!\n\n'


def CleanWiki():
    """Delete the wiki clone directory and all contained files."""

    def errorhandler(func, path, exc_info):
        # where  func is os.listdir, os.remove, or os.rmdir; path is the argument to that function that caused it to fail; and  exc_info is a tuple returned by  sys.exc_info()
        print(func)
        print(path)
        print(exc_info)
        os.chmod(path, stat.S_IWRITE)
        os.remove(path)

    try:
        shutil.rmtree(output_dir, ignore_errors=False, onerror=errorhandler)
        print('Old wiki clone removed')
    except OSError:
        print('No directory to clean found')


def CloneWiki():
    """Clone the wiki into a temporary location (first cleaning)."""
    # Clean up existing repo
    CleanWiki()

    # Create directory for output and temporary files
    try:
        os.makedirs(output_dir)
        print('Created directory')
    except OSError:
        pass

    # Clone
    git_clone_command = f'git clone {wiki_repo} {wiki_checkout}'
    print(git_clone_command)
    os.system(git_clone_command)


def ConvertFilesToRst():
    """Add template to specified page object (wikitools)."""
    indexfiletext = f'============================\nWiki snapshot (ready-for-review)\n============================\n\n{snapshot_version_information}\n.. toctree::\n    :maxdepth: 2\n'
    for file in os.listdir(wiki_checkout):
        if not file.endswith(".md"):
            continue

        inputfilename = wiki_checkout + file
        markdown = Path(inputfilename).read_text(encoding='utf-8')
        if 'This article has moved from the wiki to the new site' in markdown:
            continue
        if 'This page has been migrated to the main site' in markdown:
            continue

        print(file)
        # get name of file
        filenamestripped = os.path.splitext(file)[0]
        indexfiletext += f'\n    {filenamestripped}'
        outputfilename = output_dir + filenamestripped + '.rst'

        command = f'pandoc -f markdown -t rst -o "{outputfilename}" "{inputfilename}"'
        print(command)
        if os.system(command):
            sys.exit(1)
        title = filenamestripped.replace('-', ' ')
        # print title
        logfile.write(f'title from filename: {title} \n')
        # add import message to title
        title += ' (wiki-import)'
        length = len(title)
        # print length
        headerbar = ''
        for _ in range(length):
            headerbar += '='
        page_reference = filenamestripped
        page_reference_link_text = f'.. _{page_reference}:\n\n'
        titlebar = page_reference_link_text + headerbar + '\n' + title + '\n' + headerbar + '\n'
        textinfile = ''
        # Add titlebar to start of the file (from the filename)
        textinfile += titlebar
        # Add wiki snapshot information

        textinfile += snapshot_version_information

        with open(outputfilename, encoding='utf-8') as infile:
            for line in infile:
                textinfile += line

        # print textinfile
        with open(outputfilename, 'w', encoding='utf-8') as outfile:
            outfile.write(textinfile)

        # write the index
        with open(output_dir + 'index.rst', 'w', encoding='utf-8') as outfile:
            outfile.write(indexfiletext)


def FixupConvertedRstFiles():
    """Add template to specified page object (wikitools)."""

    def fixInternalWikiLinks(aOldText):
        """Fixes wiki links in [[linkname]] format by changing this to a document link in current directory."""
        def fixwikilinks(matchobj):
            # print 'matcobj0: %s' % matchobj.group(0)
            # print 'matcobj1: %s' % matchobj.group(1)
            linktext = matchobj.group(1)
            linktext = linktext.replace(' ', '-')
            # linktext = ':doc:`%s`' % linktext
            # use reference for linking as allows pages to be moved around
            linktext = f':ref:`{linktext}`'
            # print 'linkdoc: %s' % linktext
            logfile.write(f'linkdoc: {linktext} \n')
            return linktext
        # print 'fixing wiki links'
        return re.sub(r'\[\[(.+?)\]\]', fixwikilinks, aOldText)

    def fixWikiCodeMarkupToCodeLinks(aOldText):
        """Links "known" code objects if they are found in wiki markup."""
        def fixcodemarkuplinks(matchobj):
            # print 'Inline code: %s' % matchobj.group(0)
            # print 'matcobj1: %s' % matchobj.group(1)
            temp_set_of_codemarkup.add(matchobj.group(0))
            linktext = matchobj.group(1)
            if linktext in mapped_wiki_inline_code:
                logfile.write(f'Replace: {mapped_wiki_inline_code[linktext]} \n')
                return mapped_wiki_inline_code[linktext]

            return matchobj.group(0) # linktext
        # print 'fixing up code markup to code reference'
        return re.sub(r'``(.+?)``', fixcodemarkuplinks, aOldText)

    for file in os.listdir(output_dir):
        if file.endswith(".rst"):
            input_file = output_dir + file
            # print input_file
            textinfile = ''
            with open(input_file, encoding='utf-8') as infile:
                for line in infile:
                    textinfile += line

            # print textinfile
            # fix up broken wiki-page links in files
            textinfile = fixInternalWikiLinks(textinfile)

            # convert codemarkup to links if possible
            textinfile = fixWikiCodeMarkupToCodeLinks(textinfile)

            with open(input_file, 'w', encoding='utf-8') as outfile:
                outfile.write(textinfile)

    logfile.write('\n\nCODE MARKUP THAT WONT BE LINKED (add entry to mapped_wiki_inline_code if one of these need to be linked. The tool get-api-items.py can be used to generate the list of the documented API items. \n')
    for item in temp_set_of_codemarkup:
        logfile.write(f'{item}\n')


# parser options
def main():
    parser = optparse.OptionParser(version="%prog 0.1.1", usage="Usage: %prog [options] version")
    parser.add_option("-c", "--clonewiki", action="store_true", default=False, dest="clonewiki", help="Clean and clone the latest wiki")
    options = parser.parse_args()[0]

    print(f'Clone wiki: {options.clonewiki}')
    if options.clonewiki:
        CloneWiki()
        # input = raw_input('CHECK ALL files were cloned! (look for "error: unable to create file" )\n')

    ConvertFilesToRst()
    FixupConvertedRstFiles()
    print(f'See LOG for details: {logfilename} ')


if __name__ == '__main__':
    sys.exit(main())
