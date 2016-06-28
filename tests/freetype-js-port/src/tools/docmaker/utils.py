#
#  utils.py
#
#    Auxiliary functions for the `docmaker' tool (library file).
#
#  Copyright 2002-2016 by
#  David Turner.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.


import string, sys, os, glob, itertools


# current output directory
#
output_dir = None


# A function that generates a sorting key.  We want lexicographical order
# (primary key) except that capital letters are sorted before lowercase
# ones (secondary key).
#
# The primary key is implemented by lowercasing the input.  The secondary
# key is simply the original data appended, character by character.  For
# example, the sort key for `FT_x' is `fFtT__xx', while the sort key for
# `ft_X' is `fftt__xX'.  Since ASCII codes of uppercase letters are
# numerically smaller than the codes of lowercase letters, `fFtT__xx' gets
# sorted before `fftt__xX'.
#
def  index_key( s ):
    return string.join( itertools.chain( *zip( s.lower(), s ) ) )


# Sort `input_list', placing the elements of `order_list' in front.
#
def  sort_order_list( input_list, order_list ):
    new_list = order_list[:]
    for id in input_list:
        if not id in order_list:
            new_list.append( id )
    return new_list


# Divert standard output to a given project documentation file.  Use
# `output_dir' to determine the filename location if necessary and save the
# old stdout handle in a tuple that is returned by this function.
#
def  open_output( filename ):
    global output_dir

    if output_dir and output_dir != "":
        filename = output_dir + os.sep + filename

    old_stdout = sys.stdout
    new_file   = open( filename, "w" )
    sys.stdout = new_file

    return ( new_file, old_stdout )


# Close the output that was returned by `open_output'.
#
def  close_output( output ):
    output[0].close()
    sys.stdout = output[1]


# Check output directory.
#
def  check_output():
    global output_dir
    if output_dir:
        if output_dir != "":
            if not os.path.isdir( output_dir ):
                sys.stderr.write( "argument"
                                  + " '" + output_dir + "' "
                                  + "is not a valid directory\n" )
                sys.exit( 2 )
        else:
            output_dir = None


def  file_exists( pathname ):
    """Check that a given file exists."""
    result = 1
    try:
        file = open( pathname, "r" )
        file.close()
    except:
        result = None
        sys.stderr.write( pathname + " couldn't be accessed\n" )

    return result


def  make_file_list( args = None ):
    """Build a list of input files from command-line arguments."""
    file_list = []
    # sys.stderr.write( repr( sys.argv[1 :] ) + '\n' )

    if not args:
        args = sys.argv[1:]

    for pathname in args:
        if string.find( pathname, '*' ) >= 0:
            newpath = glob.glob( pathname )
            newpath.sort()  # sort files -- this is important because
                            # of the order of files
        else:
            newpath = [pathname]

        file_list.extend( newpath )

    if len( file_list ) == 0:
        file_list = None
    else:
        # now filter the file list to remove non-existing ones
        file_list = filter( file_exists, file_list )

    return file_list

# eof
