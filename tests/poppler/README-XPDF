Xpdf
====

version 3.03
2011-aug-15

The Xpdf software and documentation are
copyright 1996-2011 Glyph & Cog, LLC.

Email: derekn@foolabs.com
WWW: http://www.foolabs.com/xpdf/

The PDF data structures, operators, and specification are
copyright 1985-2006 Adobe Systems Inc.


What is Xpdf?
-------------

Xpdf is an open source viewer for Portable Document Format (PDF)
files.  (These are also sometimes also called 'Acrobat' files, from
the name of Adobe's PDF software.)  The Xpdf project also includes a
PDF text extractor, PDF-to-PostScript converter, and various other
utilities.

Xpdf runs under the X Window System on UNIX, VMS, and OS/2.  The non-X
components (pdftops, pdftotext, etc.) also run on Windows and Mac OSX
systems and should run on pretty much any system with a decent C++
compiler.  Xpdf will run on 32-bit and 64-bit machines.


License & Distribution
----------------------

Xpdf is licensed under the GNU General Pulbic License (GPL), version 2
or 3.  This means that you can distribute derivatives of Xpdf under
any of the following:
  - GPL v2 only
  - GPL v3 only
  - GPL v2 or v3

The Xpdf source package includes the text of both GPL versions:
COPYING for GPL v2, COPYING3 for GPL v3.

Please note that Xpdf is NOT licensed under "any later version" of the
GPL, as I have no idea what those versions will look like.

If you are redistributing unmodified copies of Xpdf (or any of the
Xpdf tools) in binary form, you need to include all of the
documentation: README, man pages (or help files), COPYING, and
COPYING3.

If you want to incorporate the Xpdf source code into another program
(or create a modified version of Xpdf), and you are distributing that
program, you have two options: release your program under the GPL (v2
and/or v3), or purchase a commercial Xpdf source license.

If you're interested in commercial licensing, please see the Glyph &
Cog web site:

    http://www.glyphandcog.com/


Compatibility
-------------

Xpdf is developed and tested on Linux.

In addition, it has been compiled by others on Solaris, AIX, HP-UX,
Digital Unix, Irix, and numerous other Unix implementations, as well
as VMS and OS/2.  It should work on pretty much any system which runs
X11 and has Unix-like libraries.  You'll need ANSI C++ and C compilers
to compile it.

The non-X components of Xpdf (pdftops, pdftotext, pdfinfo, pdffonts,
pdfdetach, pdftoppm, and pdfimages) can also be compiled on Windows
and Mac OSX systems.  See the Xpdf web page for details.

If you compile Xpdf for a system not listed on the web page, please
let me know.  If you're willing to make your binary available by ftp
or on the web, I'll be happy to add a link from the Xpdf web page.  I
have decided not to host any binaries I didn't compile myself (for
disk space and support reasons).

If you can't get Xpdf to compile on your system, send me email and
I'll try to help.

Xpdf has been ported to the Acorn, Amiga, BeOS, and EPOC.  See the
Xpdf web page for links.


Getting Xpdf
------------

The latest version is available from:

  http://www.foolabs.com/xpdf/

or:

  ftp://ftp.foolabs.com/pub/xpdf/

Source code and several precompiled executables are available.

Announcements of new versions are posted to comp.text.pdf and emailed
to a list of people.  If you'd like to receive email notification of
new versions, just let me know.


Running Xpdf
------------

To run xpdf, simply type:

  xpdf file.pdf

To generate a PostScript file, hit the "print" button in xpdf, or run
pdftops:

  pdftops file.pdf

To generate a plain text file, run pdftotext:

  pdftotext file.pdf

There are five additional utilities (which are fully described in
their man pages):

  pdfinfo -- dumps a PDF file's Info dictionary (plus some other
             useful information)
  pdffonts -- lists the fonts used in a PDF file along with various
              information for each font
  pdfdetach -- lists or extracts embedded files (attachments) from a
               PDF file
  pdftoppm -- converts a PDF file to a series of PPM/PGM/PBM-format
              bitmaps
  pdfimages -- extracts the images from a PDF file

Command line options and many other details are described in the man
pages (xpdf(1), etc.) and the VMS help files (xpdf.hlp, etc.).

All of these utilities read an optional configuration file: see the
xpdfrc(5) man page.


Upgrading from Xpdf 3.02 (and earlier)
--------------------------------------

The font configuration system has been changed.  Previous versions
used mostly separate commands to configure fonts for display and for
PostScript output.  As of 3.03, configuration options that make sense
for both display and PS output have been unified.

The following xpdfrc commands have been removed:
* displayFontT1, displayFontTT: replaced with fontFile
* displayNamedCIDFontT1, displayNamedCIDFontTT: replaced with fontFile
* displayCIDFontT1, displayCIDFontTT: replaced with fontFileCC
* psFont: replaced with psResidentFont
* psNamedFont16: replaced with psResidentFont16
* psFont16: replaced with psResidentFontCC

See the xpdfrc(5) man page for more information on the new commands.

Pdftops will now embed external 16-bit fonts (configured with the
fontFileCC command) when the PDF file refers to a non-embedded font.
It does not do any subsetting (yet), so the resulting PS files will be
large.


Compiling Xpdf
--------------

See the separate file, INSTALL.


Bugs
----

If you find a bug in Xpdf, i.e., if it prints an error message,
crashes, or incorrectly displays a document, and you don't see that
bug listed here, please send me email, with a pointer (URL, ftp site,
etc.) to the PDF file.


Acknowledgments
---------------

Thanks to:

* Patrick Voigt for help with the remote server code.
* Patrick Moreau, Martin P.J. Zinser, and David Mathog for the VMS
  port.
* David Boldt and Rick Rodgers for sample man pages.
* Brendan Miller for the icon idea.
* Olly Betts for help testing pdftotext.
* Peter Ganten for the OS/2 port.
* Michael Richmond for the Win32 port of pdftops and pdftotext and the
  xpdf/cygwin/XFree86 build instructions.
* Frank M. Siegert for improvements in the PostScript code.
* Leo Smiers for the decryption patches.
* Rainer Menzner for creating t1lib, and for helping me adapt it to
  xpdf.
* Pine Tree Systems A/S for funding the OPI and EPS support in
  pdftops.
* Easy Software Products for funding several improvements to the
  PostScript output code.
* Tom Kacvinsky for help with FreeType and for being my interface to
  the FreeType team.
* Theppitak Karoonboonyanan for help with Thai support.
* Leonard Rosenthol for help and contributions on a bunch of things.
* Alexandros Diamantidis and Maria Adaloglou for help with Greek
  support.
* Lawrence Lai for help with the CJK Unicode maps.

Various people have contributed modifications made for use by the
pdftex project:

* Han The Thanh
* Martin Schröder of ArtCom GmbH


References
----------

Adobe Systems Inc., _PDF Reference, sixth edition: Adobe Portable
Document Format version 1.7_.
http://www.adobe.com/devnet/pdf/pdf_reference.html
[The manual for PDF version 1.7.]

Adobe Systems Inc., "Errata for the PDF Reference, sixth edition,
version 1.7", October 16, 2006.
http://www.adobe.com/devnet/pdf/pdf_reference.html
[The errata for the PDF 1.7 spec.]

Adobe Systems Inc., _PostScript Language Reference_, 3rd ed.
Addison-Wesley, 1999, ISBN 0-201-37922-8.
[The official PostScript manual.]

Adobe Systems, Inc., _The Type 42 Font Format Specification_,
Adobe Developer Support Technical Specification #5012.  1998.
http://partners.adobe.com/asn/developer/pdfs/tn/5012.Type42_Spec.pdf
[Type 42 is the format used to embed TrueType fonts in PostScript
files.]

Adobe Systems, Inc., _Adobe CMap and CIDFont Files Specification_,
Adobe Developer Support Technical Specification #5014.  1995.
http://www.adobe.com/supportservice/devrelations/PDFS/TN/5014.CIDFont_Spec.pdf
[CMap file format needed for Japanese and Chinese font support.]

Adobe Systems, Inc., _Adobe-Japan1-4 Character Collection for
CID-Keyed Fonts_, Adobe Developer Support Technical Note #5078.
2000.
http://partners.adobe.com/asn/developer/PDFS/TN/5078.CID_Glyph.pdf
[The Adobe Japanese character set.]

Adobe Systems, Inc., _Adobe-GB1-4 Character Collection for
CID-Keyed Fonts_, Adobe Developer Support Technical Note #5079.
2000.
http://partners.adobe.com/asn/developer/pdfs/tn/5079.Adobe-GB1-4.pdf
[The Adobe Chinese GB (simplified) character set.]

Adobe Systems, Inc., _Adobe-CNS1-3 Character Collection for
CID-Keyed Fonts_, Adobe Developer Support Technical Note #5080.
2000.
http://partners.adobe.com/asn/developer/PDFS/TN/5080.CNS_CharColl.pdf
[The Adobe Chinese CNS (traditional) character set.]

Adobe Systems Inc., _Supporting the DCT Filters in PostScript Level
2_, Adobe Developer Support Technical Note #5116.  1992.
http://www.adobe.com/supportservice/devrelations/PDFS/TN/5116.PS2_DCT.PDF
[Description of the DCTDecode filter parameters.]

Adobe Systems Inc., _Open Prepress Interface (OPI) Specification -
Version 2.0_, Adobe Developer Support Technical Note #5660.  2000.
http://partners.adobe.com/asn/developer/PDFS/TN/5660.OPI_2.0.pdf

Adobe Systems Inc., CMap files.
ftp://ftp.oreilly.com/pub/examples/nutshell/cjkv/adobe/
[The actual CMap files for the 16-bit CJK encodings.]

Adobe Systems Inc., Unicode glyph lists.
http://partners.adobe.com/asn/developer/type/unicodegn.html
http://partners.adobe.com/asn/developer/type/glyphlist.txt
http://partners.adobe.com/asn/developer/type/corporateuse.txt
http://partners.adobe.com/asn/developer/type/zapfdingbats.txt
[Mappings between character names to Unicode.]

Adobe Systems Inc., OpenType Specification v. 1.4.
http://partners.adobe.com/public/developer/opentype/index_spec.html
[The OpenType font format spec.]

Aldus Corp., _OPI: Open Prepress Interface Specification 1.3_.  1993.
http://partners.adobe.com/asn/developer/PDFS/TN/OPI_13.pdf

Anonymous, RC4 source code.
ftp://ftp.ox.ac.uk/pub/crypto/misc/rc4.tar.gz
ftp://idea.sec.dsi.unimi.it/pub/crypt/code/rc4.tar.gz
[This is the algorithm used to encrypt PDF files.]

T. Boutell, et al., "PNG (Portable Network Graphics) Specification,
Version 1.0".  RFC 2083.
[PDF uses the PNG filter algorithms.]

CCITT, "Information Technology - Digital Compression and Coding of
Continuous-tone Still Images - Requirements and Guidelines", CCITT
Recommendation T.81.
http://www.w3.org/Graphics/JPEG/
[The official JPEG spec.]

A. Chernov, "Registration of a Cyrillic Character Set".  RFC 1489.
[Documentation for the KOI8-R Cyrillic encoding.]

Roman Czyborra, "The ISO 8859 Alphabet Soup".
http://czyborra.com/charsets/iso8859.html
[Documentation on the various ISO 859 encodings.]

L. Peter Deutsch, "ZLIB Compressed Data Format Specification version
3.3".  RFC 1950.
[Information on the general format used in FlateDecode streams.]

L. Peter Deutsch, "DEFLATE Compressed Data Format Specification
version 1.3".  RFC 1951.
[The definition of the compression algorithm used in FlateDecode
streams.]

Morris Dworkin, "Recommendation for Block Cipher Modes of Operation",
National Institute of Standards, NIST Special Publication 800-38A,
2001.
[The cipher block chaining (CBC) mode used with AES in PDF files.]

Federal Information Processing Standards Publication 197 (FIPS PUBS
197), "Advanced Encryption Standard (AES)", November 26, 2001.
[AES encryption, used in PDF 1.6.]

Jim Flowers, "X Logical Font Description Conventions", Version 1.5, X
Consortium Standard, X Version 11, Release 6.1.
ftp://ftp.x.org/pub/R6.1/xc/doc/hardcopy/XLFD/xlfd.PS.Z
[The official specification of X font descriptors, including font
transformation matrices.]

Foley, van Dam, Feiner, and Hughes, _Computer Graphics: Principles and
Practice_, 2nd ed.  Addison-Wesley, 1990, ISBN 0-201-12110-7.
[Colorspace conversion functions, Bezier spline math.]

Robert L. Hummel, _Programmer's Technical Reference: Data and Fax
Communications_.  Ziff-Davis Press, 1993, ISBN 1-56276-077-7.
[CCITT Group 3 and 4 fax decoding.]

ISO/IEC, _Information technology -- Lossy/lossless coding of bi-level
images_.  ISO/IEC 14492, First edition (2001-12-15).
http://webstore.ansi.org/
[The official JBIG2 standard.  The final draft of this spec is
available from http://www.jpeg.org/jbighomepage.html.]

ISO/IEC, _Information technology -- JPEG 2000 image coding system --
Part 1: Core coding system_.  ISO/IEC 15444-1, First edition
(2000-12-15).
http://webstore.ansi.org/
[The official JPEG 2000 standard.  The final committee draft of this
spec is available from http://www.jpeg.org/JPEG2000.html, but there
were changes made to the bitstream format between that draft and the
published spec.]

ITU, "Standardization of Group 3 facsimile terminals for document
transmission", ITU-T Recommendation T.4, 1999.
ITU, "Facsimile coding schemes and coding control functions for Group 4
facsimile apparatus", ITU-T Recommendation T.6, 1993.
http://www.itu.int/
[The official Group 3 and 4 fax standards - used by the CCITTFaxDecode
stream, as well as the JBIG2Decode stream.]

B. Kaliski, "PKCS #5: Password-Based Cryptography Specification,
Version 2.0".  RFC 2898.
[Defines the padding scheme used with AES encryption in PDF files.]

Christoph Loeffler, Adriaan Ligtenberg, George S. Moschytz, "Practical
Fast 1-D DCT Algorithms with 11 Multiplications".  IEEE Intl. Conf. on
Acoustics, Speech & Signal Processing, 1989, 988-991.
[The fast IDCT algorithm used in the DCTDecode filter.]

Microsoft, _TrueType 1.0 Font Files_, rev. 1.66.  1995.
http://www.microsoft.com/typography/tt/tt.htm
[The TrueType font spec (in MS Word format, naturally).]

V. Ostromoukhov, R.D. Hersch, "Stochastic Clustered-Dot Dithering",
Conf. Color Imaging: Device-Independent Color, Color Hardcopy, and
Graphic Arts IV, 1999, SPIE Vol. 3648, 496-505.
http://diwww.epfl.ch/w3lsp/publications/colour/scd.html
[The stochastic dithering algorithm used in Xpdf.]

P. Peterlin, "ISO 8859-2 (Latin 2) Resources".
http://sizif.mf.uni-lj.si/linux/cee/iso8859-2.html
[This is a web page with all sorts of useful Latin-2 character set and
font information.]

Charles Poynton, "Color FAQ".
http://www.inforamp.net/~poynton/ColorFAQ.html
[The mapping from the CIE 1931 (XYZ) color space to RGB.]

R. Rivest, "The MD5 Message-Digest Algorithm".  RFC 1321.
[MD5 is used in PDF document encryption.]

Thai Industrial Standard, "Standard for Thai Character Codes for
Computers", TIS-620-2533 (1990).
http://www.nectec.or.th/it-standards/std620/std620.htm
[The TIS-620 Thai encoding.]

Unicode Consortium, "Unicode Home Page".
http://www.unicode.org/
[Online copy of the Unicode spec.]

W3C Recommendation, "PNG (Portable Network Graphics) Specification
Version 1.0".
http://www.w3.org/Graphics/PNG/
[Defines the PNG image predictor.]

Gregory K. Wallace, "The JPEG Still Picture Compression Standard".
ftp://ftp.uu.net/graphics/jpeg/wallace.ps.gz
[Good description of the JPEG standard.  Also published in CACM, April
1991, and submitted to IEEE Transactions on Consumer Electronics.]

F. Yergeau, "UTF-8, a transformation format of ISO 10646".  RFC 2279.
[A commonly used Unicode encoding.]
