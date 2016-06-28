#
#  sources.py
#
#    Convert source code comments to multi-line blocks (library file).
#
#  Copyright 2002-2016 by
#  David Turner.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

#
# This library file contains definitions of classes needed to decompose C
# source code files into a series of multi-line `blocks'.  There are two
# kinds of blocks.
#
#   - Normal blocks, which contain source code or ordinary comments.
#
#   - Documentation blocks, which have restricted formatting, and whose text
#     always start with a documentation markup tag like `<Function>',
#     `<Type>', etc.
#
# The routines to process the content of documentation blocks are contained
# in file `content.py'; the classes and methods found here only deal with
# text parsing and basic documentation block extraction.
#


import fileinput, re, sys, os, string


################################################################
##
##  SOURCE BLOCK FORMAT CLASS
##
##  A simple class containing compiled regular expressions to detect
##  potential documentation format block comments within C source code.
##
##  The `column' pattern must contain a group to `unbox' the content of
##  documentation comment blocks.
##
##  Later on, paragraphs are converted to long lines, which simplifies the
##  regular expressions that act upon the text.
##
class  SourceBlockFormat:

    def  __init__( self, id, start, column, end ):
        """Create a block pattern, used to recognize special documentation
           blocks."""
        self.id     = id
        self.start  = re.compile( start, re.VERBOSE )
        self.column = re.compile( column, re.VERBOSE )
        self.end    = re.compile( end, re.VERBOSE )


#
# Format 1 documentation comment blocks.
#
#    /************************************/ (at least 2 asterisks)
#    /*                                  */
#    /*                                  */
#    /*                                  */
#    /************************************/ (at least 2 asterisks)
#
start = r'''
  \s*      # any number of whitespace
  /\*{2,}/ # followed by '/' and at least two asterisks then '/'
  \s*$     # probably followed by whitespace
'''

column = r'''
  \s*      # any number of whitespace
  /\*{1}   # followed by '/' and precisely one asterisk
  ([^*].*) # followed by anything (group 1)
  \*{1}/   # followed by one asterisk and a '/'
  \s*$     # probably followed by whitespace
'''

re_source_block_format1 = SourceBlockFormat( 1, start, column, start )


#
# Format 2 documentation comment blocks.
#
#    /************************************ (at least 2 asterisks)
#     *
#     *                                    (1 asterisk)
#     *
#     */                                   (1 or more asterisks)
#
start = r'''
  \s*     # any number of whitespace
  /\*{2,} # followed by '/' and at least two asterisks
  \s*$    # probably followed by whitespace
'''

column = r'''
  \s*           # any number of whitespace
  \*{1}(?![*/]) # followed by precisely one asterisk not followed by `/'
  (.*)          # then anything (group1)
'''

end = r'''
  \s*  # any number of whitespace
  \*+/ # followed by at least one asterisk, then '/'
'''

re_source_block_format2 = SourceBlockFormat( 2, start, column, end )


#
# The list of supported documentation block formats.  We could add new ones
# quite easily.
#
re_source_block_formats = [re_source_block_format1, re_source_block_format2]


#
# The following regular expressions correspond to markup tags within the
# documentation comment blocks.  They are equivalent despite their different
# syntax.
#
# A markup tag consists of letters or character `-', to be found in group 1.
#
# Notice that a markup tag _must_ begin a new paragraph.
#
re_markup_tag1 = re.compile( r'''\s*<((?:\w|-)*)>''' )  # <xxxx> format
re_markup_tag2 = re.compile( r'''\s*@((?:\w|-)*):''' )  # @xxxx: format

#
# The list of supported markup tags.  We could add new ones quite easily.
#
re_markup_tags = [re_markup_tag1, re_markup_tag2]


#
# A regular expression to detect a cross reference, after markup tags have
# been stripped off.
#
# Two syntax forms are supported:
#
#   @<name>
#   @<name>[<id>]
#
# where both `<name>' and `<id>' consist of alphanumeric characters, `_',
# and `-'.  Use `<id>' if there are multiple, valid `<name>' entries.
#
# Example: @foo[bar]
#
re_crossref = re.compile( r"""
                            @
                            (?P<name>(?:\w|-)+
                                     (?:\[(?:\w|-)+\])?)
                            (?P<rest>.*)
                          """, re.VERBOSE )

#
# Two regular expressions to detect italic and bold markup, respectively.
# Group 1 is the markup, group 2 the rest of the line.
#
# Note that the markup is limited to words consisting of letters, digits,
# the characters `_' and `-', or an apostrophe (but not as the first
# character).
#
re_italic = re.compile( r"_((?:\w|-)(?:\w|'|-)*)_(.*)" )     #  _italic_
re_bold   = re.compile( r"\*((?:\w|-)(?:\w|'|-)*)\*(.*)" )   #  *bold*

#
# This regular expression code to identify an URL has been taken from
#
#   http://mail.python.org/pipermail/tutor/2002-September/017228.html
#
# (with slight modifications).
#
urls = r'(?:https?|telnet|gopher|file|wais|ftp)'
ltrs = r'\w'
gunk = r'/#~:.?+=&%@!\-'
punc = r'.:?\-'
any  = "%(ltrs)s%(gunk)s%(punc)s" % { 'ltrs' : ltrs,
                                      'gunk' : gunk,
                                      'punc' : punc }
url  = r"""
         (
           \b                    # start at word boundary
           %(urls)s :            # need resource and a colon
           [%(any)s] +?          # followed by one or more of any valid
                                 # character, but be conservative and
                                 # take only what you need to...
           (?=                   # [look-ahead non-consumptive assertion]
             [%(punc)s]*         # either 0 or more punctuation
             (?:                 # [non-grouping parentheses]
               [^%(any)s] | $    # followed by a non-url char
                                 # or end of the string
             )
           )
         )
        """ % {'urls' : urls,
               'any'  : any,
               'punc' : punc }

re_url = re.compile( url, re.VERBOSE | re.MULTILINE )

#
# A regular expression that stops collection of comments for the current
# block.
#
re_source_sep = re.compile( r'\s*/\*\s*\*/' )   #  /* */

#
# A regular expression to find possible C identifiers while outputting
# source code verbatim, covering things like `*foo' or `(bar'.  Group 1 is
# the prefix, group 2 the identifier -- since we scan lines from left to
# right, sequentially splitting the source code into prefix and identifier
# is fully sufficient for our purposes.
#
re_source_crossref = re.compile( r'(\W*)(\w*)' )

#
# A regular expression that matches a list of reserved C source keywords.
#
re_source_keywords = re.compile( '''\\b ( typedef   |
                                          struct    |
                                          enum      |
                                          union     |
                                          const     |
                                          char      |
                                          int       |
                                          short     |
                                          long      |
                                          void      |
                                          signed    |
                                          unsigned  |
                                          \#include |
                                          \#define  |
                                          \#undef   |
                                          \#if      |
                                          \#ifdef   |
                                          \#ifndef  |
                                          \#else    |
                                          \#endif   ) \\b''', re.VERBOSE )


################################################################
##
##  SOURCE BLOCK CLASS
##
##  There are two important fields in a `SourceBlock' object.
##
##    self.lines
##      A list of text lines for the corresponding block.
##
##    self.content
##      For documentation comment blocks only, this is the block content
##      that has been `unboxed' from its decoration.  This is `None' for all
##      other blocks (i.e., sources or ordinary comments with no starting
##      markup tag)
##
class  SourceBlock:

    def  __init__( self, processor, filename, lineno, lines ):
        self.processor = processor
        self.filename  = filename
        self.lineno    = lineno
        self.lines     = lines[:]
        self.format    = processor.format
        self.content   = []

        if self.format == None:
            return

        words = []

        # extract comment lines
        lines = []

        for line0 in self.lines:
            m = self.format.column.match( line0 )
            if m:
                lines.append( m.group( 1 ) )

        # now, look for a markup tag
        for l in lines:
            l = string.strip( l )
            if len( l ) > 0:
                for tag in re_markup_tags:
                    if tag.match( l ):
                        self.content = lines
                        return

    def  location( self ):
        return "(" + self.filename + ":" + repr( self.lineno ) + ")"

    # debugging only -- not used in normal operations
    def  dump( self ):
        if self.content:
            print "{{{content start---"
            for l in self.content:
                print l
            print "---content end}}}"
            return

        fmt = ""
        if self.format:
            fmt = repr( self.format.id ) + " "

        for line in self.lines:
            print line


################################################################
##
##  SOURCE PROCESSOR CLASS
##
##  The `SourceProcessor' is in charge of reading a C source file and
##  decomposing it into a series of different `SourceBlock' objects.
##
##  A SourceBlock object consists of the following data.
##
##    - A documentation comment block using one of the layouts above.  Its
##      exact format will be discussed later.
##
##    - Normal sources lines, including comments.
##
##
class  SourceProcessor:

    def  __init__( self ):
        """Initialize a source processor."""
        self.blocks   = []
        self.filename = None
        self.format   = None
        self.lines    = []

    def  reset( self ):
        """Reset a block processor and clean up all its blocks."""
        self.blocks = []
        self.format = None

    def  parse_file( self, filename ):
        """Parse a C source file and add its blocks to the processor's
           list."""
        self.reset()

        self.filename = filename

        fileinput.close()
        self.format = None
        self.lineno = 0
        self.lines  = []

        for line in fileinput.input( filename ):
            # strip trailing newlines, important on Windows machines!
            if line[-1] == '\012':
                line = line[0:-1]

            if self.format == None:
                self.process_normal_line( line )
            else:
                if self.format.end.match( line ):
                    # A normal block end.  Add it to `lines' and create a
                    # new block
                    self.lines.append( line )
                    self.add_block_lines()
                elif self.format.column.match( line ):
                    # A normal column line.  Add it to `lines'.
                    self.lines.append( line )
                else:
                    # An unexpected block end.  Create a new block, but
                    # don't process the line.
                    self.add_block_lines()

                    # we need to process the line again
                    self.process_normal_line( line )

        # record the last lines
        self.add_block_lines()

    def  process_normal_line( self, line ):
        """Process a normal line and check whether it is the start of a new
           block."""
        for f in re_source_block_formats:
            if f.start.match( line ):
                self.add_block_lines()
                self.format = f
                self.lineno = fileinput.filelineno()

        self.lines.append( line )

    def  add_block_lines( self ):
        """Add the current accumulated lines and create a new block."""
        if self.lines != []:
            block = SourceBlock( self,
                                 self.filename,
                                 self.lineno,
                                 self.lines )

            self.blocks.append( block )
            self.format = None
            self.lines  = []

    # debugging only, not used in normal operations
    def  dump( self ):
        """Print all blocks in a processor."""
        for b in self.blocks:
            b.dump()

# eof
