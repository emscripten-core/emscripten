// runinput.cc
//
//   A `main' function for `ftfuzzer.cc'.
//
// Copyright 2015-2016 by
// David Turner, Robert Wilhelm, and Werner Lemberg.
//
// This file is part of the FreeType project, and may only be used,
// modified, and distributed under the terms of the FreeType project
// license, LICENSE.TXT.  By continuing to use, modify, or distribute
// this file you indicate that you have read the license and
// understand and accept it fully.


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


  extern "C" void
  LLVMFuzzerTestOneInput( const uint8_t*  data,
                          size_t          size );


  unsigned char a[1 << 24];


  int
  main( int     argc,
        char*  *argv )
  {
    assert( argc >= 2 );

    for ( int i = 1; i < argc; i++ )
    {
      fprintf( stderr, "%s\n", argv[i] );

      FILE*  f = fopen( argv[i], "r" );
      assert( f );

      size_t  n = fread( a, 1, sizeof ( a ), f );
      fclose( f );
      if ( !n )
        continue;

      unsigned char*  b = (unsigned char*)malloc( n );
      memcpy( b, a, n );

      LLVMFuzzerTestOneInput( b, n );

      free( b );
    }
  }


// END
