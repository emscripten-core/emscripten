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

#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "lzip.h"
#include "decoder.h"


const CRC32 crc32;


// Returns the number of bytes really read.
// If (returned value < size) and (errno == 0), means EOF was reached.
//
int readblock( const int fd, uint8_t * const buf, const int size )
  {
  int rest = size;
  errno = 0;
  while( rest > 0 )
    {
    errno = 0;
    const int n = read( fd, buf + size - rest, rest );
    if( n > 0 ) rest -= n;
    else if( n == 0 ) break;
    else if( errno != EINTR && errno != EAGAIN ) break;
    }
  return ( rest > 0 ) ? size - rest : size;
  }


// Returns the number of bytes really written.
// If (returned value < size), it is always an error.
//
int writeblock( const int fd, const uint8_t * const buf, const int size )
  {
  int rest = size;
  errno = 0;
  while( rest > 0 )
    {
    errno = 0;
    const int n = write( fd, buf + size - rest, rest );
    if( n > 0 ) rest -= n;
    else if( errno && errno != EINTR && errno != EAGAIN ) break;
    }
  return ( rest > 0 ) ? size - rest : size;
  }


bool Range_decoder::read_block()
  {
  if( !at_stream_end )
    {
    stream_pos = readblock( infd, buffer, buffer_size );
    if( stream_pos != buffer_size && errno ) exit(-1);
    at_stream_end = ( stream_pos < buffer_size );
    partial_member_pos += pos;
    pos = 0;
    }
  return pos < stream_pos;
  }


void LZ_decoder::flush_data()
  {
  const int size = pos - stream_pos;
  if( size > 0 )
    {
    crc32.update( crc_, buffer + stream_pos, size );
    if( outfd >= 0 &&
        writeblock( outfd, buffer + stream_pos, size ) != size )
      exit(-1);
    if( pos >= buffer_size ) { partial_data_pos += pos; pos = 0; }
    stream_pos = pos;
    }
  }


bool LZ_decoder::verify_trailer() const
  {
  File_trailer trailer;
  const int trailer_size = File_trailer::size( member_version );
  const long long member_size = range_decoder.member_position() + trailer_size;
  bool error = false;

  for( int i = 0; i < trailer_size && !error; ++i )
    {
    if( !range_decoder.finished() )
      trailer.data[i] = range_decoder.get_byte();
    else
      {
      error = true;
      for( ; i < trailer_size; ++i ) trailer.data[i] = 0;
      }
    }
  if( member_version == 0 ) trailer.member_size( member_size );
  if( !range_decoder.code_is_zero() )
    {
    error = true;
    pp( "Range decoder final code is not zero" );
    }
  if( trailer.data_crc() != crc() )
    {
    error = true;
    }
  if( trailer.data_size() != data_position() )
    {
    error = true;
    }
  if( trailer.member_size() != member_size )
    {
    error = true;
    }
  return !error;
  }


// Return value: 0 = OK, 1 = decoder error, 2 = unexpected EOF,
//               3 = trailer error, 4 = unknown marker found.
int LZ_decoder::decode_member()
  {
  Bit_model bm_match[State::states][pos_states];
  Bit_model bm_rep[State::states];
  Bit_model bm_rep0[State::states];
  Bit_model bm_rep1[State::states];
  Bit_model bm_rep2[State::states];
  Bit_model bm_len[State::states][pos_states];
  Bit_model bm_dis_slot[max_dis_states][1<<dis_slot_bits];
  Bit_model bm_dis[modeled_distances-end_dis_model+1];
  Bit_model bm_align[dis_align_size];

  unsigned int rep0 = 0;	// rep[0-3] latest four distances
  unsigned int rep1 = 0;	// used for efficient coding of
  unsigned int rep2 = 0;	// repeated distances
  unsigned int rep3 = 0;

  Len_decoder len_decoder;
  Len_decoder rep_match_len_decoder;
  Literal_decoder literal_decoder;
  State state;
  range_decoder.load();

  while( true )
    {
    if( range_decoder.finished() ) { flush_data(); return 2; }
    const int pos_state = data_position() & pos_state_mask;
    if( range_decoder.decode_bit( bm_match[state()][pos_state] ) == 0 )
      {
      const uint8_t prev_byte = get_prev_byte();
      if( state.is_char() )
        put_byte( literal_decoder.decode( range_decoder, prev_byte ) );
      else
        put_byte( literal_decoder.decode_matched( range_decoder, prev_byte,
                                                  get_byte( rep0 ) ) );
      state.set_char();
      }
    else
      {
      int len;
      if( range_decoder.decode_bit( bm_rep[state()] ) == 1 )
        {
        len = 0;
        if( range_decoder.decode_bit( bm_rep0[state()] ) == 1 )
          {
          unsigned int distance;
          if( range_decoder.decode_bit( bm_rep1[state()] ) == 0 )
            distance = rep1;
          else
            {
            if( range_decoder.decode_bit( bm_rep2[state()] ) == 0 )
              distance = rep2;
            else { distance = rep3; rep3 = rep2; }
            rep2 = rep1;
            }
          rep1 = rep0;
          rep0 = distance;
          }
        else
          {
          if( range_decoder.decode_bit( bm_len[state()][pos_state] ) == 0 )
            { state.set_short_rep(); len = 1; }
          }
        if( len == 0 )
          {
          state.set_rep();
          len = min_match_len + rep_match_len_decoder.decode( range_decoder, pos_state );
          }
        }
      else
        {
        const unsigned int rep0_saved = rep0;
        len = min_match_len + len_decoder.decode( range_decoder, pos_state );
        const int dis_slot = range_decoder.decode_tree( bm_dis_slot[get_dis_state(len)], dis_slot_bits );
        if( dis_slot < start_dis_model ) rep0 = dis_slot;
        else
          {
          const int direct_bits = ( dis_slot >> 1 ) - 1;
          rep0 = ( 2 | ( dis_slot & 1 ) ) << direct_bits;
          if( dis_slot < end_dis_model )
            rep0 += range_decoder.decode_tree_reversed( bm_dis + rep0 - dis_slot, direct_bits );
          else
            {
            rep0 += range_decoder.decode( direct_bits - dis_align_bits ) << dis_align_bits;
            rep0 += range_decoder.decode_tree_reversed( bm_align, dis_align_bits );
            if( rep0 == 0xFFFFFFFFU )		// Marker found
              {
              rep0 = rep0_saved;
              range_decoder.normalize();
              flush_data();
              if( len == min_match_len )	// End Of Stream marker
                {
                if( verify_trailer() ) return 0; else return 3;
                }
              if( len == min_match_len + 1 )	// Sync Flush marker
                {
                range_decoder.load(); continue;
                }
              return 4;
              }
            }
          }
        rep3 = rep2; rep2 = rep1; rep1 = rep0_saved;
        state.set_match();
        if( rep0 >= (unsigned int)dictionary_size ||
            ( rep0 >= (unsigned int)pos && !partial_data_pos ) )
          { flush_data(); return 1; }
        }
      copy_block( rep0, len );
      }
    }
  }
