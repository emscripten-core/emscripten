#
# This script gets the Emscripten wiki, converts files from markdown to restructure text using pandoc (and also prepends the text with a title/heading based on the filename)
# It also fixes up inline code items to become links to api-reference
#
# It should be called prior to building the site.
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



wiki_repo = 'https://github.com/kripken/emscripten.wiki.git'
wiki_directory = '.\\wiki_static\\'
logfilename='log-get-wiki.txt'




# Add the directory containing your module to the Python path (wants absolute paths)
#sys.path.append(os.path.abspath(scriptpath))
from api_items import get_mapped_items
# GetmMap of code items from api_items.py 
mapped_wiki_inline_code = get_mapped_items()
#Add any additional mapped items that seem reasonable.
mapped_wiki_inline_code['getValue(ptr, type)']=':js:func:`getValue(ptr, type) <getValue>`'
mapped_wiki_inline_code['setValue(ptr, value, type)']=':js:func:`setValue(ptr, value, type) <setValue>`'
"""

"""




wiki_temp_directory = wiki_directory + 'temp\\'
temp_set_of_codemarkup=set()
logfile=open(logfilename,'w')
#snapshot_version_information='.. note:: This is a **snapshot** of the wiki: %s\n\n' % strftime("%a, %d %b %Y %H:%M", gmtime())
snapshot_version_information='.. note:: This article was migrated from the wiki (%s) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!\n\n' % strftime("%a, %d %b %Y %H:%M", gmtime())


def CleanWiki():	
    """
    Delete the wiki clone directory and all contained files.
    """

    def errorhandler(func, path, exc_info):
        # where  func is os.listdir, os.remove, or os.rmdir; path is the argument to that function that caused it to fail; and  exc_info is a tuple returned by  sys.exc_info()
        print func
        print path
        print exc_info
        os.chmod(path, stat.S_IWRITE)
        os.remove(path)

    try:
        shutil.rmtree(wiki_directory, ignore_errors=False, onerror=errorhandler)
        print 'Old wiki clone removed'
    except:
        print 'No directory to clean found'
        



def CloneWiki():	
    """
    Clone the wiki into a temporary location (first cleaning)
    """
    # Clean up existing repo
    CleanWiki()

    # Create directory for output and temporary files
    try:
        os.makedirs(wiki_directory)
        os.makedirs(wiki_temp_directory)
        print 'Created directory'
    except:
        pass
    

    # Clone
    git_clone_command = 'git clone %s %s' % (wiki_repo, wiki_temp_directory)
    #print git_clone_command
    os.system(git_clone_command)
 
    

def ConvertFilesToRst():	
    """
    Add template to specified page object (wikitools)
    """
    indexfiletext='============================\nWiki snapshot (ready-for-review)\n============================\n\n%s\n.. toctree::\n    :maxdepth: 2\n' % snapshot_version_information
    for file in os.listdir(wiki_temp_directory):
        if file.endswith(".md"):
            print file
            #get name of file
            filenamestripped=file.replace('.md','')
            indexfiletext+='\n    %s' % filenamestripped
            inputfilename=wiki_temp_directory+file
            outputfilename=wiki_directory+filenamestripped+'.rst'
            #
            command='pandoc -f markdown -t rst -o %s %s' % (outputfilename,inputfilename)
            print command
            os.system(command)
            title=filenamestripped.replace('-',' ')
            #print title
            logfile.write('title from filename: %s \n' % title)
            #add import message to title
            title+=' (wiki-import)'
            length=len(title)
            #print length
            headerbar=''
            for number in range(length):
                headerbar+='='
            page_reference=filenamestripped
            page_reference_link_text = '.. _%s:\n\n' % page_reference
            titlebar=page_reference_link_text+headerbar+'\n'+title+'\n'+headerbar+'\n'
            textinfile=''
            # Add titlebar to start of the file (from the filename)
            textinfile+=titlebar
            # Add wiki snapshot information
            
            textinfile+=snapshot_version_information
              
            infile=open(outputfilename,'r')
            
            for line in infile:
                textinfile+=line
            infile.close()
            #print textinfile
            
            outfile=open(outputfilename,'w')
            outfile.write(textinfile)
            outfile.close()
            #write the index
            outfile=open(wiki_directory+'index.rst','w')
            outfile.write(indexfiletext)
            outfile.close()


def FixupConvertedRstFiles():	
    """
    Add template to specified page object (wikitools)
    """

    def fixInternalWikiLinks(aOldText):
        """
        Fixes wiki links in [[linkname]] format by changing this to a document link in current directory.
        """
        def fixwikilinks(matchobj):
            #print 'matcobj0: %s' % matchobj.group(0)
            #print 'matcobj1: %s' % matchobj.group(1)
            linktext=matchobj.group(1)
            linktext=linktext.replace(' ','-')
            #linktext=':doc:`%s`' % linktext
            linktext=':ref:`%s`' % linktext #use reference for linking as allows pages to be moved around
            #print 'linkdoc: %s' % linktext
            logfile.write('linkdoc: %s \n' % linktext)
            return linktext
        #print 'fixing wiki links'
        return re.sub(r'\[\[(.+?)\]\]', fixwikilinks, aOldText)


    def fixWikiCodeMarkupToCodeLinks(aOldText):
        """
        Links "known" code objects if they are found in wiki markup.
        """
        def fixcodemarkuplinks(matchobj):
            #print 'Inline code: %s' % matchobj.group(0)
            #print 'matcobj1: %s' % matchobj.group(1)
            temp_set_of_codemarkup.add(matchobj.group(0))
            linktext=matchobj.group(1)
            if linktext in mapped_wiki_inline_code:
                logfile.write('Replace: %s \n' % mapped_wiki_inline_code[linktext]) 
                return mapped_wiki_inline_code[linktext] 

            return matchobj.group(0) #linktext
        #print 'fixing up code markup to code reference'
        return re.sub(r'``(.+?)``', fixcodemarkuplinks, aOldText)


   
    for file in os.listdir(wiki_directory):
        if file.endswith(".rst"):
            input_file=wiki_directory+file
            #print input_file
            infile=open(input_file,'r')
            
            textinfile=''
            for line in infile:
                textinfile+=line
            infile.close()
            #print textinfile

            #fix up broken wiki-page links in files
            textinfile=fixInternalWikiLinks(textinfile)

            #convert codemarkup to links if possible
            textinfile=fixWikiCodeMarkupToCodeLinks(textinfile)

            
            outfile=open(input_file,'w')
            outfile.write(textinfile)
            outfile.close()


    logfile.write('\n\nCODE MARKUP THAT WONT BE LINKED (add entry to mapped_wiki_inline_code if one of these need to be linked. The tool get-api-items.py can be used to generate the list of the documented API items. \n')
    for item in temp_set_of_codemarkup:
        logfile.write('%s\n' % item)



#parser options
parser = OptionParser(version="%prog 0.1.1", usage="Usage: %prog [options] version")
parser.add_option("-c", "--clonewiki", dest="clonewiki", default=True, help="Clean and clone the latest wiki")


(options, args) = parser.parse_args()

print 'Clone wiki: %s' % options.clonewiki

if options.clonewiki==True:
    CloneWiki()   
    input= raw_input('CHECK ALL files were cloned! (look for "error: unable to create file" )\n')
    
ConvertFilesToRst()    
FixupConvertedRstFiles()
print 'See LOG for details: %s ' % logfilename


logfile.close()



