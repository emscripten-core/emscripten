# 
# This script gets all the API items defined in the emscripten documentation. These can then be used for automated adding of cross links in other scripts. It writes api_items.py which has function that is imported into get-wiki.py
#

import re #for fixing links in the imported text

import pprint

#for options parsing
from optparse import OptionParser

import shutil #file manipulation
import os
#import glob
import stat

from time import gmtime, strftime
#import time
import time



#the directory, relative to \source for the API reference
api_reference_directory = '.\\docs\\api_reference\\'

#name to write API items to. Note, this is used by the get-wiki.py script, so if you change here, change everywhere.
api_item_filename = 'api_items.py'

api_reference_items = dict()


def parseFiles():	
    """
    Parse api-reference files to extract the code items.

    """

    def addapiitems(matchobj):
        #print 'matcobj0: %s' % matchobj.group(0)
        #print 'matcobj1: %s' % matchobj.group(1)
        #print 'matcobj2: %s' % matchobj.group(2)
        #print 'matcobj3: %s' % matchobj.group(3)
        #print 'matcobj4: %s' % matchobj.group(4)

        lang=matchobj.group(2)
        data_type = matchobj.group(3)
        if data_type=='function':
            data_type='func'
        api_item=matchobj.group(4)
        api_item=api_item.strip()
        api_item=api_item.split('(')[0]
        try:
            api_item=api_item.split(' ')[1]
        except:
            pass

        #print lang
        #print data_type
        #print api_item

        api_reference_items[api_item]=':%s:%s:`%s`' % (lang,data_type,api_item)
        if data_type=='func': #Add additional index for functions declared as func() rather than just func
            api_item_index=api_item+'()'
            api_reference_items[api_item_index]=':%s:%s:`%s`' % (lang,data_type,api_item)
            
        #print api_reference_items[api_item]
        

    for file in os.listdir(api_reference_directory):
        if file.endswith(".rst"):
            filepath=api_reference_directory+file
            print file
            #open file
            infile=open(filepath,'r')
            
            for line in infile:
                #parse line for API items
                re.sub(r'^\.\.\s+((\w+)\:(\w+)\:\:(.*))', addapiitems, line)   
            infile.close()
            

def exportItems():	
    """
    Export the API items into form for use in another script.
    """
    infile=open(api_item_filename,'w')
    #write function lead in
    infile.write("# Auto-generated file (see get-api_items.py)\n#\n\ndef get_mapped_items():\n\tmapped_wiki_inline_code = dict()\n" )
            
    for item in api_reference_items:
        #Write out each API item to add
        infile.write("\tmapped_wiki_inline_code['%s']='%s'\n" % (item, api_reference_items[item]) )

    #write the return fucntion
    infile.write("\treturn mapped_wiki_inline_code" )
    infile.close()


#parser options
parser = OptionParser(version="%prog 0.1.1", usage="Usage: %prog [options] version")
parser.add_option("-s", "--siteapi", dest="siteapi", default="http://www.developer.nokia.com/Community/Wiki/api.php", help="Location of API")


(options, args) = parser.parse_args()

#print 'Site: %s' % options.siteapi




parseFiles()
exportItems()





