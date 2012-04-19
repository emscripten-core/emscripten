/*  Lzip - Data compressor based on the LZMA algorithm
    Copyright (C) 2008, 2009, 2010, 2011 Antonio Diaz Diaz.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
    Return values: 0 for a normal exit, 1 for environmental problems
    (file not found, invalid flags, I/O errors, etc), 2 to indicate a
    corrupt or invalid input file, 3 for an internal consistency error
    (eg, bug) which caused lzip to panic.
*/

#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#if defined(__MSVCRT__)
#include <io.h>
#define fchmod(x,y) 0
#define fchown(x,y,z) 0
#define SIGHUP SIGTERM
#define S_ISSOCK(x) 0
#define S_IRGRP 0
#define S_IWGRP 0
#define S_IROTH 0
#define S_IWOTH 0
#endif
#if defined(__OS2__)
#include <io.h>
#endif

#include "lzip.h"
#include "decoder.h"

#if !DECODER_ONLY
#include "encoder.h"
#include "fast_encoder.h"
#endif

#if CHAR_BIT != 8
#error "Environments where CHAR_BIT != 8 are not supported."
#endif

#ifndef LLONG_MAX
#define LLONG_MAX  0x7FFFFFFFFFFFFFFFLL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN  (-LLONG_MAX - 1LL)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX 0xFFFFFFFFFFFFFFFFULL
#endif


void pp(const char *p) { if (p) fputs(p, stderr); }

namespace {

const char * const Program_name = "Lzip";
const char * const program_name = "lzip";
const char * const program_year = "2011";
const char * invocation_name = 0;

#ifdef O_BINARY
const int o_binary = O_BINARY;
#else
const int o_binary = 0;
#endif

struct { const char * from; const char * to; } const known_extensions[] = {
{ ".lz",  ""   },
{ ".tlz", ".tar" },
{ 0,      0    } };

struct Lzma_options
{
  int dictionary_size;		// 4KiB..512MiB
  int match_len_limit;		// 5..273
};

enum Mode { m_compress, m_decompress, m_test };

int outfd = -1;
int verbosity = 0;
bool delete_output_on_interrupt = false;


void show_help()
{
  printf( "%s - Data compressor based on the LZMA algorithm.\n", Program_name );
  printf( "<< Most of these are unsupported. Compressing/decompressing from stdin to stdout is the right way! >>\n" );
  printf( "\nUsage: %s [options] [files]\n", invocation_name );
  printf( "\nOptions:\n" );
  printf( "  -h, --help                 display this help and exit\n" );
  printf( "  -V, --version              output version information and exit\n" );
  printf( "  -b, --member-size=<n>      set member size limit in bytes\n" );
  printf( "  -c, --stdout               send output to standard output\n" );
  printf( "  -d, --decompress           decompress\n" );
  printf( "  -f, --force                overwrite existing output files\n" );
  printf( "  -F, --recompress           force recompression of compressed files\n" );
  printf( "  -k, --keep                 keep (don't delete) input files\n" );
  printf( "  -m, --match-length=<n>     set match length limit in bytes [36]\n" );
  printf( "  -o, --output=<file>        if reading stdin, place the output into <file>\n" );
  printf( "  -q, --quiet                suppress all messages\n" );
  printf( "  -s, --dictionary-size=<n>  set dictionary size limit in bytes [8MiB]\n" );
  printf( "  -S, --volume-size=<n>      set volume size limit in bytes\n" );
  printf( "  -t, --test                 test compressed file integrity\n" );
  printf( "  -v, --verbose              be verbose (a 2nd -v gives more)\n" );
  printf( "  -0 .. -9                   set compression level [default 6]\n" );
  printf( "      --fast                 alias for -0\n" );
  printf( "      --best                 alias for -9\n" );
  printf( "If no file names are given, %s compresses or decompresses\n", program_name );
  printf( "from standard input to standard output.\n" );
  printf( "Numbers may be followed by a multiplier: k = kB = 10^3 = 1000,\n" );
  printf( "Ki = KiB = 2^10 = 1024, M = 10^6, Mi = 2^20, G = 10^9, Gi = 2^30, etc...\n" );
  printf( "\nReport bugs to lzip-bug@nongnu.org\n" );
  printf( "Lzip home page: http://www.nongnu.org/lzip/lzip.html\n" );
}


void show_version()
{
  printf( "%s %s\n", Program_name, PROGVERSION );
  printf( "Copyright (C) %s Antonio Diaz Diaz.\n", program_year );
  printf( "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n" );
  printf( "This is free software: you are free to change and redistribute it.\n" );
  printf( "There is NO WARRANTY, to the extent permitted by law.\n" );
}


const char * format_num( long long num )
{
  const char * const prefix[8] =
  { "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi" };
  enum { buf_size = 16, factor = 1024 };
  static char buf[buf_size];
  const char *p = "";

  for( int i = 0; i < 8 && ( llabs( num ) > 9999 ||
       ( llabs( num ) >= factor && num % factor == 0 ) ); ++i )
  { num /= factor; p = prefix[i]; }
  snprintf( buf, buf_size, "%lld %s", num, p );
  return buf;
}


bool open_outstream( const bool force )
{ 
  return false;
}


bool check_tty( const int infd, const Mode program_mode )
{
  if( program_mode == m_compress && outfd >= 0 && isatty( outfd ) )
  {
    show_error( "I won't write compressed data to a terminal.", 0, true );
    return false;
  }
  if( ( program_mode == m_decompress || program_mode == m_test ) &&
      isatty( infd ) )
  {
    show_error( "I won't read compressed data from a terminal.", 0, true );
    return false;
  }
  return true;
}


void cleanup_and_fail( const int retval )
{
  exit( retval );
}


     // Set permissions, owner and times.
void close_and_set_permissions( const struct stat * const in_statsp )
{
  bool error = false;
  if( in_statsp )
  {
    if( ( fchown( outfd, in_statsp->st_uid, in_statsp->st_gid ) != 0 &&
          errno != EPERM ) ||
        fchmod( outfd, in_statsp->st_mode ) != 0 )
      error = true;
    // fchown will in many cases return with EPERM, which can be safely ignored.
  }
  if( close( outfd ) == 0 ) outfd = -1;
  else cleanup_and_fail( 1 );
  delete_output_on_interrupt = false;
  if( !in_statsp ) return;
  if( !error )
  {
    struct utimbuf t;
    t.actime = in_statsp->st_atime;
    t.modtime = in_statsp->st_mtime;
    //if( utime( output_filename.c_str(), &t ) != 0 ) error = true;
  }
  if( error )
  {
    show_error( "Can't change output file attributes." );
    cleanup_and_fail( 1 );
  }
}


bool next_filename()
{
  return false;
}

#if !DECODER_ONLY
int compress( const long long member_size, const long long volume_size,
              const Lzma_options & encoder_options, const int infd,
              const struct stat * const in_statsp )
{
  File_header header;
  header.set_magic();
  if( !header.dictionary_size( encoder_options.dictionary_size ) ||
      encoder_options.match_len_limit < min_match_len_limit ||
      encoder_options.match_len_limit > max_match_len )
    internal_error( "invalid argument to encoder" );
  int retval = 0;

    Matchfinder matchfinder( header.dictionary_size(),
                             encoder_options.match_len_limit, infd );
    header.dictionary_size( matchfinder.dictionary_size() );

    long long in_size = 0, out_size = 0, partial_volume_size = 0;
    while( true )		// encode one member per iteration
    {
      LZ_encoder encoder( matchfinder, header, outfd );
      const long long size =
        min( member_size, volume_size - partial_volume_size );
      if( !encoder.encode_member( size ) )
      { pp( "Encoder error" ); retval = 1; break; }
      in_size += matchfinder.data_position();
      out_size += encoder.member_position();
      if( matchfinder.finished() ) break;
      partial_volume_size += encoder.member_position();
      if( partial_volume_size >= volume_size - min_dictionary_size )
      {
        partial_volume_size = 0;
        if( delete_output_on_interrupt )
        {
          close_and_set_permissions( in_statsp );
          if( !next_filename() )
          { pp( "Too many volume files" ); retval = 1; break; }
          if( !open_outstream( true ) ) { retval = 1; break; }
          delete_output_on_interrupt = true;
        }
      }
      matchfinder.reset();
    }

    if( retval == 0 && verbosity >= 1 )
    {
      if( in_size <= 0 || out_size <= 0 )
        fprintf( stderr, "No data compressed.\n" );
      else
        fprintf( stderr, "%6.3f:1, %6.3f bits/byte, "
                              "%5.2f%% saved, %lld in, %lld out.\n",
                      (double)in_size / out_size,
                      ( 8.0 * out_size ) / in_size,
                      100.0 * ( 1.0 - ( (double)out_size / in_size ) ),
                      in_size, out_size );
    }
  return retval;
}


int fcompress( const long long member_size, const long long volume_size,
               const int infd,
               const struct stat * const in_statsp )
{
  if( verbosity >= 1 ) pp();
  File_header header;
  header.set_magic();
  int retval = 0;

    Fmatchfinder fmatchfinder( infd );
    header.dictionary_size( fmatchfinder.dictionary_size() );

    long long in_size = 0, out_size = 0, partial_volume_size = 0;
    while( true )		// encode one member per iteration
    {
      FLZ_encoder encoder( fmatchfinder, header, outfd );
      const long long size =
        min( member_size, volume_size - partial_volume_size );
      if( !encoder.encode_member( size ) )
      { pp( "Encoder error" ); retval = 1; break; }
      in_size += fmatchfinder.data_position();
      out_size += encoder.member_position();
      if( fmatchfinder.finished() ) break;
      partial_volume_size += encoder.member_position();
      if( partial_volume_size >= volume_size - min_dictionary_size )
      {
        partial_volume_size = 0;
        if( delete_output_on_interrupt )
        {
          close_and_set_permissions( in_statsp );
          if( !next_filename() )
          { pp( "Too many volume files" ); retval = 1; break; }
          if( !open_outstream( true ) ) { retval = 1; break; }
          delete_output_on_interrupt = true;
        }
      }
      fmatchfinder.reset();
    }

    if( retval == 0 && verbosity >= 1 )
    {
      if( in_size <= 0 || out_size <= 0 )
        fprintf( stderr, "No data compressed.\n" );
      else
        fprintf( stderr, "%6.3f:1, %6.3f bits/byte, "
                              "%5.2f%% saved, %lld in, %lld out.\n",
                      (double)in_size / out_size,
                      ( 8.0 * out_size ) / in_size,
                      100.0 * ( 1.0 - ( (double)out_size / in_size ) ),
                      in_size, out_size );
    }
  return retval;
}
#endif

int decompress( const int infd, const bool testing )
{
  int retval = 0;

    Range_decoder rdec( infd );
    long long partial_file_pos = 0;
    for( bool first_member = true; ; first_member = false )
    {
      File_header header;
      int size;
      rdec.reset_member_position();
      for( size = 0; size < File_header::size && !rdec.finished(); ++size )
         header.data[size] = rdec.get_byte();
      if( rdec.finished() )			// End Of File
      {
        if( first_member )
        { pp( "Error reading member header" ); retval = 1; }
        break;
      }
      if( !header.verify_magic() )
      {
        if( first_member )
        { pp( "Bad magic number (file not in lzip format)" ); retval = 2; }
        break;
      }
      if( !header.verify_version() )
      {
        if( verbosity >= 0 )
        { pp();
            fprintf( stderr, "Version %d member format not supported.\n",
                          header.version() ); }
        retval = 2; break;
      }
      if( header.dictionary_size() < min_dictionary_size ||
          header.dictionary_size() > max_dictionary_size )
      { pp( "Invalid dictionary size in member header" ); retval = 2; break; }

      if( verbosity >= 2 || ( verbosity == 1 && first_member ) )
      {
        pp();
        if( verbosity >= 2 )
          fprintf( stderr, "version %d, dictionary size %7sB.  ",
                        header.version(),
                        format_num( header.dictionary_size() ) );
      }
      LZ_decoder decoder( header, rdec, outfd );

      const int result = decoder.decode_member();
      partial_file_pos += rdec.member_position();
      if( result != 0 )
      {
        if( verbosity >= 0 && result <= 2 )
        {
          pp();
          if( result == 2 )
            fprintf( stderr, "File ends unexpectedly at pos %lld\n",
                          partial_file_pos );
          else
            fprintf( stderr, "Decoder error at pos %lld\n",
                          partial_file_pos );
        }
        retval = 2; break;
      }
      if( verbosity >= 2 )
      { if( testing ) fprintf( stderr, "ok\n" );
          else fprintf( stderr, "done\n" ); }
    }
  if( verbosity == 1 && retval == 0 )
  { if( testing ) fprintf( stderr, "ok\n" );
      else fprintf( stderr, "done\n" ); }
  return retval;
}


} // end namespace


void show_error( const char * const msg, const int errcode, const bool help )
{
  if( verbosity >= 0 )
  {
    if( msg && msg[0] )
    {
      fprintf( stderr, "%s: %s", program_name, msg );
      if( errcode > 0 )
        fprintf( stderr, ": %s", strerror( errcode ) );
      fprintf( stderr, "\n" );
    }
    if( help && invocation_name && invocation_name[0] )
      fprintf( stderr, "Try `%s --help' for more information.\n",
                    invocation_name );
  }
}


void internal_error( const char * const msg )
{
  if( verbosity >= 0 )
    fprintf( stderr, "%s: internal error: %s.\n", program_name, msg );
  exit( 3 );
}


int main( const int argc, const char * const argv[] )
{
  // Mapping from gzip/bzip2 style 1..9 compression modes
  // to the corresponding LZMA compression modes.
  const Lzma_options option_mapping[] =
  {
  { 1 << 16,  16 },		// -0 entry values not used
  { 1 << 20,   5 },		// -1
  { 3 << 19,   6 },		// -2
  { 1 << 21,   8 },		// -3
  { 3 << 20,  12 },		// -4
  { 1 << 22,  20 },		// -5
  { 1 << 23,  36 },		// -6
  { 1 << 24,  68 },		// -7
  { 3 << 23, 132 },		// -8
  { 1 << 25, 273 } };		// -9
  Lzma_options encoder_options = option_mapping[6];	// default = "-6"
  long long member_size = LLONG_MAX;
  long long volume_size = LLONG_MAX;
  int infd = -1;
  Mode program_mode = m_compress;
  bool keep_input_files = false;
  bool to_stdout = false;
  bool zero = false;
  invocation_name = argv[0];

  // Greatly simplified argument parsing
  int argind = 1;
  for( ; argind < argc; ++argind )
  {
    const int code = argv[argind][1];
    switch( code )
    {
      case 'c': to_stdout = true; break;
      case 'd': program_mode = m_decompress; break;
      case 'h': show_help(); return 0;
      case 'k': keep_input_files = true; break;
      case 'q': verbosity = -1; break;
                zero = false; break;
      case 'v': if( verbosity < 4 ) ++verbosity; break;
      case 'V': show_version(); return 0;
      default : internal_error( "uncaught option" );
    }
  } // end process options

#if defined(__MSVCRT__) || defined(__OS2__)
  _setmode( STDIN_FILENO, O_BINARY );
  _setmode( STDOUT_FILENO, O_BINARY );
#endif

  if( program_mode == m_test )
    outfd = -1;
#if !DECODER_ONLY
  else if( program_mode == m_compress )
  {
    dis_slots.init();
    prob_prices.init();
  }
#endif

  int retval = 0;
  {
    struct stat in_stats;

    infd = STDIN_FILENO;
    outfd = STDOUT_FILENO;

    if( !check_tty( infd, program_mode ) ) return 1;

    const struct stat * const in_statsp = 0;
    //pp.set_name( "-" );
    int tmp = 0;
#if !DECODER_ONLY
    if( program_mode == m_compress )
    {
      if( zero )
        tmp = fcompress( member_size, volume_size, infd, in_statsp );
      else
        tmp = compress( member_size, volume_size, encoder_options, infd,
                        in_statsp );
    }
    else
#endif
      tmp = decompress( infd, program_mode == m_test );
    if( tmp > retval ) retval = tmp;
    //if( tmp && program_mode != m_test ) cleanup_and_fail( retval );

    if( delete_output_on_interrupt )
      close_and_set_permissions( in_statsp );
  }
  if( outfd >= 0 && close( outfd ) != 0 )
  {
    show_error( "Can't close stdout", errno );
    if( retval < 1 ) retval = 1;
  }
  return retval;
}
