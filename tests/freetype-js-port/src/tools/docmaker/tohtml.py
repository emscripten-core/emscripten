#
#  tohtml.py
#
#    A sub-class container of the `Formatter' class to produce HTML.
#
#  Copyright 2002-2016 by
#  David Turner.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

# The parent class is contained in file `formatter.py'.


from sources import *
from content import *
from formatter import *

import time


# The following strings define the HTML header used by all generated pages.
html_header_1 = """\
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>\
"""

html_header_2 = """\
 API Reference</title>
<style type="text/css">
  a:link { color: #0000EF; }
  a:visited { color: #51188E; }
  a:hover { color: #FF0000; }

  body { font-family: Verdana, Geneva, Arial, Helvetica, serif;
         color: #000000;
         background: #FFFFFF;
         width: 87%;
         margin: auto; }

  div.section { width: 75%;
                margin: auto; }
  div.section hr { margin: 4ex 0 1ex 0; }
  div.section h4 { background-color: #EEEEFF;
                   font-size: medium;
                   font-style: oblique;
                   font-weight: bold;
                   margin: 3ex 0 1.5ex 9%;
                   padding: 0.3ex 0 0.3ex 1%; }
  div.section p { margin: 1.5ex 0 1.5ex 10%; }
  div.section pre { margin: 3ex 0 3ex 9%;
                    background-color: #D6E8FF;
                    padding: 2ex 0 2ex 1%; }
  div.section table.fields { width: 90%;
                             margin: 1.5ex 0 1.5ex 10%; }
  div.section table.toc { width: 95%;
                          margin: 1.5ex 0 1.5ex 5%; }
  div.timestamp { text-align: center;
                  font-size: 69%;
                  margin: 1.5ex 0 1.5ex 0; }

  h1 { text-align: center; }
  h3 { font-size: medium;
       margin: 4ex 0 1.5ex 0; }

  p { text-align: justify; }

  pre.colored { color: blue; }

  span.keyword { font-family: monospace;
                 text-align: left;
                 white-space: pre;
                 color: darkblue; }

  table.fields td.val { font-weight: bold;
                        text-align: right;
                        width: 30%;
                        vertical-align: baseline;
                        padding: 1ex 1em 1ex 0; }
  table.fields td.desc { vertical-align: baseline;
                         padding: 1ex 0 1ex 1em; }
  table.fields td.desc p:first-child { margin: 0; }
  table.fields td.desc p { margin: 1.5ex 0 0 0; }
  table.index { margin: 6ex auto 6ex auto;
                border: 0;
                border-collapse: separate;
                border-spacing: 1em 0.3ex; }
  table.index tr { padding: 0; }
  table.index td { padding: 0; }
  table.index-toc-link { width: 100%;
                         border: 0;
                         border-spacing: 0;
                         margin: 1ex 0 1ex 0; }
  table.index-toc-link td.left { padding: 0 0.5em 0 0.5em;
                                 font-size: 83%;
                                 text-align: left; }
  table.index-toc-link td.middle { padding: 0 0.5em 0 0.5em;
                                   font-size: 83%;
                                   text-align: center; }
  table.index-toc-link td.right { padding: 0 0.5em 0 0.5em;
                                  font-size: 83%;
                                  text-align: right; }
  table.synopsis { margin: 6ex auto 6ex auto;
                   border: 0;
                   border-collapse: separate;
                   border-spacing: 2em 0.6ex; }
  table.synopsis tr { padding: 0; }
  table.synopsis td { padding: 0; }
  table.toc td.link { width: 30%;
                      text-align: right;
                      vertical-align: baseline;
                      padding: 1ex 1em 1ex 0; }
  table.toc td.desc { vertical-align: baseline;
                      padding: 1ex 0 1ex 1em;
                      text-align: left; }
  table.toc td.desc p:first-child { margin: 0;
                                    text-align: left; }
  table.toc td.desc p { margin: 1.5ex 0 0 0;
                        text-align: left; }

</style>
</head>
<body>
"""

html_header_3l = """
<table class="index-toc-link"><tr><td class="left">[<a href="\
"""

html_header_3r = """
<table class="index-toc-link"><tr><td class="right">[<a href="\
"""

html_header_4 = """\
">Index</a>]</td><td class="right">[<a href="\
"""

html_header_5t = """\
">TOC</a>]</td></tr></table>
<h1>\
"""

html_header_5i = """\
">Index</a>]</td></tr></table>
<h1>\
"""

html_header_6 = """\
 API Reference</h1>
"""


# The HTML footer used by all generated pages.
html_footer = """\
</body>
</html>\
"""

# The header and footer used for each section.
section_title_header1 = '<h1 id="'
section_title_header2 = '">'
section_title_footer = "</h1>"

# The header and footer used for code segments.
code_header = '<pre class="colored">'
code_footer = '</pre>'

# Paragraph header and footer.
para_header = "<p>"
para_footer = "</p>"

# Block header and footer.
block_header        = '<div class="section">'
block_footer_start  = """\
<hr>
<table class="index-toc-link"><tr><td class="left">[<a href="\
"""
block_footer_middle = """\
">Index</a>]</td>\
<td class="middle">[<a href="#">Top</a>]</td>\
<td class="right">[<a href="\
"""
block_footer_end    = """\
">TOC</a>]</td></tr></table></div>
"""

# Description header/footer.
description_header = ""
description_footer = ""

# Marker header/inter/footer combination.
marker_header = "<h4>"
marker_inter  = "</h4>"
marker_footer = ""

# Header location header/footer.
header_location_header = "<p>"
header_location_footer = "</p>"

# Source code extracts header/footer.
source_header = "<pre>"
source_footer = "</pre>"

# Chapter header/inter/footer.
chapter_header = """\
<div class="section">
<h2>\
"""
chapter_inter  = '</h2>'
chapter_footer = '</div>'

# Index footer.
index_footer_start = """\
<hr>
<table class="index-toc-link"><tr><td class="right">[<a href="\
"""
index_footer_end = """\
">TOC</a>]</td></tr></table>
"""

# TOC footer.
toc_footer_start = """\
<hr>
<table class="index-toc-link"><tr><td class="left">[<a href="\
"""
toc_footer_end = """\
">Index</a>]</td></tr></table>
"""


# Source language keyword coloration and styling.
keyword_prefix = '<span class="keyword">'
keyword_suffix = '</span>'

section_synopsis_header = '<h2>Synopsis</h2>'
section_synopsis_footer = ''


# Translate a single line of source to HTML.  This converts `<', `>', and
# `&' into `&lt;',`&gt;', and `&amp;'.
#
def  html_quote( line ):
    result = string.replace( line,   "&", "&amp;" )
    result = string.replace( result, "<", "&lt;"  )
    result = string.replace( result, ">", "&gt;"  )
    return result


################################################################
##
##  HTML FORMATTER CLASS
##
class  HtmlFormatter( Formatter ):

    def  __init__( self, processor, project_title, file_prefix ):
        Formatter.__init__( self, processor )

        global html_header_1
        global html_header_2
        global html_header_3l, html_header_3r
        global html_header_4
        global html_header_5t, html_header_5i
        global html_header_6
        global html_footer

        if file_prefix:
            file_prefix = file_prefix + "-"
        else:
            file_prefix = ""

        self.headers       = processor.headers
        self.project_title = project_title
        self.file_prefix   = file_prefix
        self.html_header   = (
          html_header_1 + project_title
          + html_header_2
          + html_header_3l + file_prefix + "index.html"
          + html_header_4 + file_prefix + "toc.html"
          + html_header_5t + project_title
          + html_header_6 )
        self.html_index_header = (
          html_header_1 + project_title
          + html_header_2
          + html_header_3r + file_prefix + "toc.html"
          + html_header_5t + project_title
          + html_header_6 )
        self.html_toc_header = (
          html_header_1 + project_title
          + html_header_2
          + html_header_3l + file_prefix + "index.html"
          + html_header_5i + project_title
          + html_header_6 )
        self.html_footer = (
          '<div class="timestamp">generated on '
          + time.asctime( time.localtime( time.time() ) )
          + "</div>" + html_footer )

        self.columns = 3

    def  make_section_url( self, section ):
        return self.file_prefix + section.name + ".html"

    def  make_block_url( self, block, name = None ):
        if name == None:
            name = block.name

        try:
            section_url = self.make_section_url( block.section )
        except:
            # we already have a section
            section_url = self.make_section_url( block )

        return section_url + "#" + name

    def  make_html_word( self, word ):
        """Analyze a simple word to detect cross-references and markup."""
        # handle cross-references
        m = re_crossref.match( word )
        if m:
            try:
                name = m.group( 'name' )
                rest = m.group( 'rest' )
                block = self.identifiers[name]
                url   = self.make_block_url( block )
                # display `foo[bar]' as `foo'
                name = re.sub( r'\[.*\]', '', name )
                # normalize url, following RFC 3986
                url = string.replace( url, "[", "(" )
                url = string.replace( url, "]", ")" )

                try:
                    # for sections, display title
                    url = ( '&lsquo;<a href="' + url + '">'
                            + block.title + '</a>&rsquo;'
                            + rest )
                except:
                    url = ( '<a href="' + url + '">'
                            + name + '</a>'
                            + rest )

                return url
            except:
                # we detected a cross-reference to an unknown item
                sys.stderr.write( "WARNING: undefined cross reference"
                                  + " '" + name + "'.\n" )
                return '?' + name + '?' + rest

        # handle markup for italic and bold
        m = re_italic.match( word )
        if m:
            name = m.group( 1 )
            rest = m.group( 2 )
            return '<i>' + name + '</i>' + rest

        m = re_bold.match( word )
        if m:
            name = m.group( 1 )
            rest = m.group( 2 )
            return '<b>' + name + '</b>' + rest

        return html_quote( word )

    def  make_html_para( self, words ):
        """Convert words of a paragraph into tagged HTML text.  Also handle
           cross references."""
        line = ""
        if words:
            line = self.make_html_word( words[0] )
            for word in words[1:]:
                line = line + " " + self.make_html_word( word )
            # handle hyperlinks
            line = re_url.sub( r'<a href="\1">\1</a>', line )
            # convert `...' quotations into real left and right single quotes
            line = re.sub( r"(^|\W)`(.*?)'(\W|$)",
                           r'\1&lsquo;\2&rsquo;\3',
                           line )
            # convert tilde into non-breakable space
            line = string.replace( line, "~", "&nbsp;" )

        return para_header + line + para_footer

    def  make_html_code( self, lines ):
        """Convert a code sequence to HTML."""
        line = code_header + '\n'
        for l in lines:
            line = line + html_quote( l ) + '\n'

        return line + code_footer

    def  make_html_items( self, items ):
        """Convert a field's content into HTML."""
        lines = []
        for item in items:
            if item.lines:
                lines.append( self.make_html_code( item.lines ) )
            else:
                lines.append( self.make_html_para( item.words ) )

        return string.join( lines, '\n' )

    def  print_html_items( self, items ):
        print self.make_html_items( items )

    def  print_html_field( self, field ):
        if field.name:
            print( '<table><tr valign="top"><td><b>'
                   + field.name
                   + "</b></td><td>" )

        print self.make_html_items( field.items )

        if field.name:
            print "</td></tr></table>"

    def  html_source_quote( self, line, block_name = None ):
        result = ""
        while line:
            m = re_source_crossref.match( line )
            if m:
                name   = m.group( 2 )
                prefix = html_quote( m.group( 1 ) )
                length = len( m.group( 0 ) )

                if name == block_name:
                    # this is the current block name, if any
                    result = result + prefix + '<b>' + name + '</b>'
                elif re_source_keywords.match( name ):
                    # this is a C keyword
                    result = ( result + prefix
                               + keyword_prefix + name + keyword_suffix )
                elif name in self.identifiers:
                    # this is a known identifier
                    block = self.identifiers[name]
                    id = block.name

                    # link to a field ID if possible
                    try:
                      for markup in block.markups:
                          if markup.tag == 'values':
                              for field in markup.fields:
                                  if field.name:
                                      id = name

                      result = ( result + prefix
                                 + '<a href="'
                                 + self.make_block_url( block, id )
                                 + '">' + name + '</a>' )
                    except:
                      # sections don't have `markups'; however, we don't
                      # want references to sections here anyway
                      result = result + html_quote( line[:length] )

                else:
                    result = result + html_quote( line[:length] )

                line = line[length:]
            else:
                result = result + html_quote( line )
                line   = []

        return result

    def  print_html_field_list( self, fields ):
        print '<table class="fields">'
        for field in fields:
            print ( '<tr><td class="val" id="' + field.name + '">'
                    + field.name
                    + '</td><td class="desc">' )
            self.print_html_items( field.items )
            print "</td></tr>"
        print "</table>"

    def  print_html_markup( self, markup ):
        table_fields = []
        for field in markup.fields:
            if field.name:
                # We begin a new series of field or value definitions.  We
                # record them in the `table_fields' list before outputting
                # all of them as a single table.
                table_fields.append( field )
            else:
                if table_fields:
                    self.print_html_field_list( table_fields )
                    table_fields = []

                self.print_html_items( field.items )

        if table_fields:
            self.print_html_field_list( table_fields )

    #
    # formatting the index
    #
    def  index_enter( self ):
        print self.html_index_header
        self.index_items = {}

    def  index_name_enter( self, name ):
        block = self.identifiers[name]
        url   = self.make_block_url( block )
        self.index_items[name] = url

    def  index_exit( self ):
        # `block_index' already contains the sorted list of index names
        count = len( self.block_index )
        rows  = ( count + self.columns - 1 ) // self.columns

        print '<table class="index">'
        for r in range( rows ):
            line = "<tr>"
            for c in range( self.columns ):
                i = r + c * rows
                if i < count:
                    bname = self.block_index[r + c * rows]
                    url   = self.index_items[bname]
                    # display `foo[bar]' as `foo (bar)'
                    bname = string.replace( bname, "[", " (" )
                    bname = string.replace( bname, "]", ")"  )
                    # normalize url, following RFC 3986
                    url = string.replace( url, "[", "(" )
                    url = string.replace( url, "]", ")" )
                    line  = ( line + '<td><a href="' + url + '">'
                              + bname + '</a></td>' )
                else:
                    line = line + '<td></td>'
            line = line + "</tr>"
            print line

        print "</table>"

        print( index_footer_start
               + self.file_prefix + "toc.html"
               + index_footer_end )

        print self.html_footer

        self.index_items = {}

    def  index_dump( self, index_filename = None ):
        if index_filename == None:
            index_filename = self.file_prefix + "index.html"

        Formatter.index_dump( self, index_filename )

    #
    # formatting the table of contents
    #
    def  toc_enter( self ):
        print self.html_toc_header
        print "<h1>Table of Contents</h1>"

    def  toc_chapter_enter( self, chapter ):
        print chapter_header + string.join( chapter.title ) + chapter_inter
        print '<table class="toc">'

    def  toc_section_enter( self, section ):
        print ( '<tr><td class="link">'
                + '<a href="' + self.make_section_url( section ) + '">'
                + section.title + '</a></td><td class="desc">' )
        print self.make_html_para( section.abstract )

    def  toc_section_exit( self, section ):
        print "</td></tr>"

    def  toc_chapter_exit( self, chapter ):
        print "</table>"
        print chapter_footer

    def  toc_index( self, index_filename ):
        print( chapter_header
               + '<a href="' + index_filename + '">Global Index</a>'
               + chapter_inter + chapter_footer )

    def  toc_exit( self ):
        print( toc_footer_start
               + self.file_prefix + "index.html"
               + toc_footer_end )

        print self.html_footer

    def  toc_dump( self, toc_filename = None, index_filename = None ):
        if toc_filename == None:
            toc_filename = self.file_prefix + "toc.html"

        if index_filename == None:
            index_filename = self.file_prefix + "index.html"

        Formatter.toc_dump( self, toc_filename, index_filename )

    #
    # formatting sections
    #
    def  section_enter( self, section ):
        print self.html_header

        print ( section_title_header1 + section.name + section_title_header2
                + section.title
                + section_title_footer )

        maxwidth = 0
        for b in section.blocks.values():
            if len( b.name ) > maxwidth:
                maxwidth = len( b.name )

        width = 70  # XXX magic number
        if maxwidth > 0:
            # print section synopsis
            print section_synopsis_header
            print '<table class="synopsis">'

            columns = width // maxwidth
            if columns < 1:
                columns = 1

            count = len( section.block_names )
            # don't handle last entry if it is empty
            if section.block_names[-1] == "/empty/":
                count -= 1
            rows  = ( count + columns - 1 ) // columns

            for r in range( rows ):
                line = "<tr>"
                for c in range( columns ):
                    i = r + c * rows
                    line = line + '<td>'
                    if i < count:
                        name = section.block_names[i]
                        if name == "/empty/":
                            # it can happen that a complete row is empty, and
                            # without a proper `filler' the browser might
                            # collapse the row to a much smaller height (or
                            # even omit it completely)
                            line = line + "&nbsp;"
                        else:
                            url = name
                            # display `foo[bar]' as `foo'
                            name = re.sub( r'\[.*\]', '', name )
                            # normalize url, following RFC 3986
                            url = string.replace( url, "[", "(" )
                            url = string.replace( url, "]", ")" )
                            line = ( line + '<a href="#' + url + '">'
                                     + name + '</a>' )

                    line = line + '</td>'
                line = line + "</tr>"
                print line

            print "</table>"
            print section_synopsis_footer

        print description_header
        print self.make_html_items( section.description )
        print description_footer

    def  block_enter( self, block ):
        print block_header

        # place html anchor if needed
        if block.name:
            url = block.name
            # display `foo[bar]' as `foo'
            name = re.sub( r'\[.*\]', '', block.name )
            # normalize url, following RFC 3986
            url = string.replace( url, "[", "(" )
            url = string.replace( url, "]", ")" )
            print( '<h3 id="' + url + '">' + name + '</h3>' )

        # dump the block C source lines now
        if block.code:
            header = ''
            for f in self.headers.keys():
                if block.source.filename.find( f ) >= 0:
                    header = self.headers[f] + ' (' + f + ')'
                    break

#           if not header:
#               sys.stderr.write(
#                 "WARNING: No header macro for"
#                 + " '" + block.source.filename + "'.\n" )

            if header:
                print ( header_location_header
                        + 'Defined in ' + header + '.'
                        + header_location_footer )

            print source_header
            for l in block.code:
                print self.html_source_quote( l, block.name )
            print source_footer

    def  markup_enter( self, markup, block ):
        if markup.tag == "description":
            print description_header
        else:
            print marker_header + markup.tag + marker_inter

        self.print_html_markup( markup )

    def  markup_exit( self, markup, block ):
        if markup.tag == "description":
            print description_footer
        else:
            print marker_footer

    def  block_exit( self, block ):
        print( block_footer_start + self.file_prefix + "index.html"
               + block_footer_middle + self.file_prefix + "toc.html"
               + block_footer_end )

    def  section_exit( self, section ):
        print html_footer

    def  section_dump_all( self ):
        for section in self.sections:
            self.section_dump( section,
                               self.file_prefix + section.name + '.html' )

# eof
