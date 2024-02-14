#!/usr/bin/env python3
# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

#
# This script gets all the API items defined in the emscripten documentation.
# These can then be used for automated adding of cross links in other scripts.
# It writes api_items.py which has function that is imported into get-wiki.py
#

import optparse
import os
import re
import sys

# the directory, relative to \source for the API reference
api_reference_directory = './docs/api_reference/'

# name to write API items to. Note, this is used by the get-wiki.py script, so
# if you change here, change everywhere.
api_item_filename = 'api_items.py'

api_reference_items = dict()


def parseFiles():
    """Parse api-reference files to extract the code items.
    """

    def addapiitems(matchobj):
        # print 'matcobj0: %s' % matchobj.group(0)
        # print 'matcobj1: %s' % matchobj.group(1)
        # print 'matcobj2: %s' % matchobj.group(2)
        # print 'matcobj3: %s' % matchobj.group(3)
        # print 'matcobj4: %s' % matchobj.group(4)

        lang = matchobj.group(2)
        data_type = matchobj.group(3)
        if data_type == 'function':
            data_type = 'func'
        api_item = matchobj.group(4)
        api_item = api_item.strip()
        api_item = api_item.split('(')[0]
        try:
            api_item = api_item.split(' ')[1]
        except IndexError:
            pass

        # print lang
        # print data_type
        # print api_item

        api_reference_items[api_item] = ':%s:%s:`%s`' % (lang, data_type, api_item)
        # Add additional index for functions declared as func() rather than just func
        if data_type == 'func':
            api_item_index = api_item + '()'
            api_reference_items[api_item_index] = ':%s:%s:`%s`' % (lang, data_type, api_item)

        # print api_reference_items[api_item]

    for file in os.listdir(api_reference_directory):
        if file.endswith(".rst"):
            filepath = api_reference_directory + file
            print(file)
            # open file
            with open(filepath, 'r') as infile:
                for line in infile:
                    # parse line for API items
                    re.sub(r'^\.\.\s+((\w+)\:(\w+)\:\:(.*))', addapiitems, line)


def exportItems():
    """Export the API items into form for use in another script.
    """
    with open(api_item_filename, 'w') as infile:
        # write function lead in
        infile.write("# Auto-generated file (see get_api_items.py)\n\ndef get_mapped_items():\n    mapped_wiki_inline_code = dict()\n")

        items = list((key, value) for key, value in api_reference_items.items())
        items.sort()
        for key, value in items:
            # Write out each API item to add
            infile.write("    mapped_wiki_inline_code['%s'] = '%s'\n" % (key, value))

        # write the return function
        infile.write("    return mapped_wiki_inline_code\n")


def main():
    parser = optparse.OptionParser(usage="Usage: %prog [options] version")
    parser.add_option("-s", "--siteapi", dest="siteapi", default="http://www.developer.nokia.com/Community/Wiki/api.php", help="Location of API")
    (options, args) = parser.parse_args()
    # print 'Site: %s' % options.siteapi
    parseFiles()
    exportItems()
    return 0


if __name__ == '__main__':
    sys.exit(main())
