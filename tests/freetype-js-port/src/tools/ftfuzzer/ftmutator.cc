// ftmutator.cc
//
//   A custom fuzzer mutator to test for FreeType with libFuzzer.
//
// Copyright 2015-2016 by
// David Turner, Robert Wilhelm, and Werner Lemberg.
//
// This file is part of the FreeType project, and may only be used,
// modified, and distributed under the terms of the FreeType project
// license, LICENSE.TXT.  By continuing to use, modify, or distribute
// this file you indicate that you have read the license and
// understand and accept it fully.


// Since `tar' is not a valid format for input to FreeType, treat any input
// that looks like `tar' as multiple files and mutate them separately.
//
// In the future, a variation of this may be used to guide mutation on a
// logically higher level.


// we use `unique_ptr', `decltype', and other gimmicks defined since C++11
#if __cplusplus < 201103L
#  error "a C++11 compiler is needed"
#endif

#include <cstdint>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <iostream>

#include <memory>
#include <vector>

#include <archive.h>
#include <archive_entry.h>

#include "FuzzerInterface.h"


  using namespace std;


  // This function should be defined by `ftfuzzer.cc'.
  extern "C" int
  LLVMFuzzerTestOneInput( const uint8_t*  Data,
                          size_t          Size );


  static void
  check_result( struct archive*  a,
                int              r )
  {
    if ( r == ARCHIVE_OK )
      return;

    const char*  m = archive_error_string( a );
    write( 1, m, strlen( m ) );
    exit( 1 );
  }


  static int
  archive_read_entry_data( struct archive   *ar,
                           vector<uint8_t>  *vw )
  {
    int             r;
    const uint8_t*  buff;
    size_t          size;
    int64_t         offset;

    for (;;)
    {
      r = archive_read_data_block( ar,
                                   reinterpret_cast<const void**>( &buff ),
                                   &size,
                                   &offset );
      if ( r == ARCHIVE_EOF )
        return ARCHIVE_OK;
      if ( r != ARCHIVE_OK )
        return r;

      vw->insert( vw->end(), buff, buff + size );
    }
  }


  static vector<vector<uint8_t>>
  parse_data( const uint8_t*  data,
              size_t          size )
  {
    struct archive_entry*    entry;
    int                      r;
    vector<vector<uint8_t>>  files;

    unique_ptr<struct  archive,
               decltype ( archive_read_free )*>  a( archive_read_new(),
                                                    archive_read_free );

    // activate reading of uncompressed tar archives
    archive_read_support_format_tar( a.get() );

    // the need for `const_cast' was removed with libarchive commit be4d4dd
    if ( !( r = archive_read_open_memory(
                  a.get(),
                  const_cast<void*>(static_cast<const void*>( data ) ),
                  size ) ) )
    {
      unique_ptr<struct  archive,
                 decltype ( archive_read_close )*>  a_open( a.get(),
                                                            archive_read_close );

      // read files contained in archive
      for (;;)
      {
        r = archive_read_next_header( a_open.get(), &entry );
        if ( r == ARCHIVE_EOF )
          break;
        if ( r != ARCHIVE_OK )
          break;

        vector<uint8_t>  entry_data;
        r = archive_read_entry_data( a.get(), &entry_data );
        if ( entry_data.size() == 0 )
          continue;

        files.push_back( move( entry_data ) );
        if ( r != ARCHIVE_OK )
          break;
      }
    }

    return files;
  }


  class FTFuzzer
  : public fuzzer::UserSuppliedFuzzer
  {

  public:
    FTFuzzer( fuzzer::FuzzerRandomBase*  Rand )
    : fuzzer::UserSuppliedFuzzer( Rand ) {}


    int
    TargetFunction( const uint8_t*  Data,
                    size_t          Size )
    {
      return LLVMFuzzerTestOneInput( Data, Size );
    }


    // Custom mutator.
    virtual size_t
    Mutate( uint8_t*  Data,
            size_t    Size,
            size_t    MaxSize )
    {
      vector<vector<uint8_t>>  files = parse_data( Data, Size );

      // If the file was not recognized as a tar file, treat it as non-tar.
      if ( files.size() == 0 )
        return fuzzer::UserSuppliedFuzzer::Mutate( Data, Size, MaxSize );

      // This is somewhat `white box' on tar.  The tar format uses 512 byte
      // blocks.  One block as header for each file, two empty blocks of 0's
      // at the end.  File data is padded to fill its last block.
      size_t  used_blocks = files.size() + 2;
      for ( const auto&  file : files )
        used_blocks += ( file.size() + 511 ) / 512;

      size_t  max_blocks = MaxSize / 512;

      // If the input is big, it will need to be downsized.  If the original
      // tar file was too big, it may have been clipped to fit.  In this
      // case it may not be possible to properly write out the data, as
      // there may not be enough space for the trailing two blocks.  Start
      // dropping file data or files from the end.
      for ( size_t  i = files.size();
            i-- > 1 && used_blocks > max_blocks; )
      {
        size_t  blocks_to_free = used_blocks - max_blocks;
        size_t  blocks_currently_used_by_file_data =
                  ( files[i].size() + 511 ) / 512;

        if ( blocks_currently_used_by_file_data >= blocks_to_free )
        {
          files[i].resize( ( blocks_currently_used_by_file_data -
                               blocks_to_free ) * 512 );
          used_blocks -= blocks_to_free;
          continue;
        }

        files.pop_back();
        used_blocks -= blocks_currently_used_by_file_data + 1;
      }

      // If we get down to one file, don't use tar.
      if ( files.size() == 1 )
      {
        memcpy( Data, files[0].data(), files[0].size() );
        return fuzzer::UserSuppliedFuzzer::Mutate( Data,
                                                   files[0].size(),
                                                   MaxSize );
      }

      size_t  free_blocks = max_blocks - used_blocks;

      // Allow each file to use up as much of the currently available space
      // it can.  If it uses or gives up blocks, add them or remove them
      // from the pool.
      for ( auto&&  file : files )
      {
        size_t  blocks_currently_used_by_file = ( file.size() + 511 ) / 512;
        size_t  blocks_available = blocks_currently_used_by_file +
                                     free_blocks;
        size_t  max_size = blocks_available * 512;
        size_t  data_size = file.size();

        file.resize( max_size );
        file.resize( fuzzer::UserSuppliedFuzzer::Mutate( file.data(),
                                                         data_size,
                                                         max_size ) );

        size_t  blocks_now_used_by_file = ( file.size() + 511 ) / 512;
        free_blocks = free_blocks +
                        blocks_currently_used_by_file -
                        blocks_now_used_by_file;
      }

      unique_ptr<struct  archive,
                 decltype ( archive_write_free )*>  a( archive_write_new(),
                                                       archive_write_free );

      check_result( a.get(), archive_write_add_filter_none( a.get() ) );
      check_result( a.get(), archive_write_set_format_ustar( a.get() ) );

      // `used' may not be correct until after the archive is closed.
      size_t  used = 0xbadbeef;
      check_result( a.get(), archive_write_open_memory( a.get(),
                                                        Data,
                                                        MaxSize,
                                                        &used ) );

      {
        unique_ptr<struct  archive,
                   decltype ( archive_write_close )*>  a_open( a.get(),
                                                               archive_write_close );

        int  file_index = 0;
        for ( const auto&  file : files )
        {
          unique_ptr<struct  archive_entry,
                     decltype ( archive_entry_free )*>
            e( archive_entry_new2( a_open.get() ),
                                   archive_entry_free );

          char  name_buffer[100];
          snprintf( name_buffer, 100, "file%d", file_index++ );

          archive_entry_set_pathname( e.get(), name_buffer );
          archive_entry_set_size( e.get(), file.size() );
          archive_entry_set_filetype( e.get(), AE_IFREG );
          archive_entry_set_perm( e.get(), 0644 );

          check_result( a_open.get(),
                        archive_write_header( a_open.get(), e.get() ) );
          archive_write_data( a_open.get(), file.data(), file.size() );
          check_result( a_open.get(),
                        archive_write_finish_entry( a_open.get() ) );
        }
      }

      return used;
    }


    // Cross `Data1' and `Data2', write up to `MaxOutSize' bytes into `Out',
    // return the number of bytes written, which should be positive.
    virtual size_t
    CrossOver( const uint8_t*  Data1,
               size_t          Size1,
               const uint8_t*  Data2,
               size_t          Size2,
               uint8_t*        Out,
               size_t          MaxOutSize )
    {
      return fuzzer::UserSuppliedFuzzer::CrossOver( Data1,
                                                    Size1,
                                                    Data2,
                                                    Size2,
                                                    Out,
                                                    MaxOutSize );
    }

  }; // end of FTFuzzer class


  int
  main( int     argc,
        char*  *argv )
  {
    fuzzer::FuzzerRandomLibc  Rand( 0 );
    FTFuzzer                  F( &Rand );

    fuzzer::FuzzerDriver( argc, argv, F );
  }


// END
