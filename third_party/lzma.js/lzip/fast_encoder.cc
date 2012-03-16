#if !DECODER_ONLY

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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lzip.h"
#include "encoder.h"
#include "fast_encoder.h"


bool Fmatchfinder::read_block()
  {
  if( !at_stream_end && stream_pos < buffer_size )
    {
    const int size = buffer_size - stream_pos;
    const int rd = readblock( infd, buffer + stream_pos, size );
    stream_pos += rd;
    if( rd != size && errno ) throw Error( "Read error" );
    at_stream_end = ( rd < size );
    }
  return pos < stream_pos;
  }


Fmatchfinder::Fmatchfinder( const int ifd )
  :
  partial_data_pos( 0 ),
  prev_positions( new int32_t[num_prev_positions] ),
  pos( 0 ),
  cyclic_pos( 0 ),
  key4( 0 ),
  stream_pos( 0 ),
  match_len_limit_( 16 ),
  infd( ifd ),
  at_stream_end( false )
  {
  const int dict_size = 65536;
  const int buffer_size_limit = ( 16 * dict_size ) + before_size + after_size;
  buffer_size = dict_size;
  buffer = (uint8_t *)malloc( buffer_size );
  if( !buffer ) exit(-1);
  if( read_block() && !at_stream_end && buffer_size < buffer_size_limit )
    {
    buffer_size = buffer_size_limit;
    buffer = (uint8_t *)realloc( buffer, buffer_size );
    if( !buffer ) exit(-1);
    read_block();
    }
  if( at_stream_end && stream_pos < dict_size )
    dictionary_size_ = max( (int)min_dictionary_size, stream_pos );
  else dictionary_size_ = dict_size;
  pos_limit = buffer_size;
  if( !at_stream_end ) pos_limit -= after_size;
  prev_pos_chain = new int32_t[dictionary_size_];
  for( int i = 0; i < num_prev_positions; ++i ) prev_positions[i] = -1;
  }


void Fmatchfinder::reset()
  {
  const int size = stream_pos - pos;
  if( size > 0 ) memmove( buffer, buffer + pos, size );
  partial_data_pos = 0;
  stream_pos -= pos;
  pos = 0;
  cyclic_pos = 0;
  key4 = 0;
  for( int i = 0; i < num_prev_positions; ++i ) prev_positions[i] = -1;
  read_block();
  }


void Fmatchfinder::move_pos()
  {
  if( ++cyclic_pos >= dictionary_size_ ) cyclic_pos = 0;
  if( ++pos >= pos_limit )
    {
    if( pos > stream_pos )
      internal_error( "pos > stream_pos in Fmatchfinder::move_pos" );
    if( !at_stream_end )
      {
      const int offset = pos - dictionary_size_ - before_size;
      const int size = stream_pos - offset;
      memmove( buffer, buffer + offset, size );
      partial_data_pos += offset;
      pos -= offset;
      stream_pos -= offset;
      for( int i = 0; i < num_prev_positions; ++i )
        if( prev_positions[i] >= 0 ) prev_positions[i] -= offset;
      for( int i = 0; i < dictionary_size_; ++i )
        if( prev_pos_chain[i] >= 0 ) prev_pos_chain[i] -= offset;
      read_block();
      }
    }
  }


int Fmatchfinder::longest_match_len( int * const distance )
  {
  int len_limit = match_len_limit_;
  if( len_limit > available_bytes() )
    {
    len_limit = available_bytes();
    if( len_limit < 4 ) return 0;
    }

  const uint8_t * const data = buffer + pos;
  key4 = ( ( key4 << 4 ) ^ data[3] ) & ( num_prev_positions - 1 );

  int newpos = prev_positions[key4];
  prev_positions[key4] = pos;

  int32_t * ptr0 = prev_pos_chain + cyclic_pos;
  int maxlen = 0;

  for( int count = 4; ; )
    {
    if( newpos < (pos - dictionary_size_ + 1) || newpos < 0 || --count < 0 )
      { *ptr0 = -1; break; }
    const uint8_t * const newdata = buffer + newpos;
    int len = 0;
    if( newdata[maxlen] == data[maxlen] )
      while( len < len_limit && newdata[len] == data[len] ) ++len;

    const int delta = pos - newpos;
    if( maxlen < len ) { maxlen = len; *distance = delta - 1; }

    int32_t * const newptr = prev_pos_chain +
      ( cyclic_pos - delta +
          ( ( cyclic_pos >= delta ) ? 0 : dictionary_size_ ) );

    if( len < len_limit )
      {
      *ptr0 = newpos;
      ptr0 = newptr;
      newpos = *ptr0;
      }
    else
      {
      *ptr0 = *newptr;
      break;
      }
    }
  return maxlen;
  }


void Fmatchfinder::longest_match_len()
  {
  int len_limit = match_len_limit_;
  if( len_limit > available_bytes() )
    {
    len_limit = available_bytes();
    if( len_limit < 4 ) return;
    }

  const uint8_t * const data = buffer + pos;
  key4 = ( ( key4 << 4 ) ^ data[3] ) & ( num_prev_positions - 1 );

  const int newpos = prev_positions[key4];
  prev_positions[key4] = pos;

  int32_t * const ptr0 = prev_pos_chain + cyclic_pos;

  if( newpos < (pos - dictionary_size_ + 1) || newpos < 0 ) *ptr0 = -1;
  else
    {
    const uint8_t * const newdata = buffer + newpos;
    if( newdata[len_limit-1] != data[len_limit-1] ||
        memcmp( newdata, data, len_limit - 1 ) ) *ptr0 = newpos;
    else
      {
      int idx = cyclic_pos - pos + newpos;
      if( idx < 0 ) idx += dictionary_size_;
      *ptr0 = prev_pos_chain[idx];
      }
    }
  }


// Return value == number of bytes advanced (len).
// *disp returns the distance to encode.
// ( *disp == -1 && len == 1 ) means literal.
int FLZ_encoder::sequence_optimizer( const int reps[num_rep_distances],
                                     int * const disp, const State & state )
  {
  const int main_len = read_match_distances();

  int replen = 0;
  int rep_index = 0;
  for( int i = 0; i < num_rep_distances; ++i )
    {
    const int len = fmatchfinder.true_match_len( 0, reps[i] + 1, max_match_len );
    if( len > replen ) { replen = len; rep_index = i; }
    }
  if( replen > min_match_len && replen + 4 > main_len )
    {
    *disp = rep_index;
    move_pos( replen, true );
    return replen;
    }

  if( main_len > min_match_len ||
      ( main_len == min_match_len && match_distance < modeled_distances ) )
    {
    *disp = num_rep_distances + match_distance;
    move_pos( main_len, true );
    return main_len;
    }

  const uint8_t cur_byte = fmatchfinder[0];
  const uint8_t match_byte = fmatchfinder[-reps[0]-1];

  *disp = -1;
  if( match_byte == cur_byte )
    {
    const uint8_t prev_byte = fmatchfinder[-1];
    const int pos_state = fmatchfinder.data_position() & pos_state_mask;
    int price = price0( bm_match[state()][pos_state] );
    if( state.is_char() )
      price += literal_encoder.price_symbol( prev_byte, cur_byte );
    else
      price += literal_encoder.price_matched( prev_byte, cur_byte, match_byte );
    const int short_rep_price = price1( bm_match[state()][pos_state] ) +
                                price1( bm_rep[state()] ) +
                                price0( bm_rep0[state()] ) +
                                price0( bm_len[state()][pos_state] );
    if( short_rep_price < price ) *disp = 0;
    }

  fmatchfinder.move_pos();
  return 1;
  }


     // End Of Stream mark => (dis == 0xFFFFFFFFU, len == min_match_len)
void FLZ_encoder::full_flush( const State & state )
  {
  const int pos_state = fmatchfinder.data_position() & pos_state_mask;
  range_encoder.encode_bit( bm_match[state()][pos_state], 1 );
  range_encoder.encode_bit( bm_rep[state()], 0 );
  encode_pair( 0xFFFFFFFFU, min_match_len, pos_state );
  range_encoder.flush();
  File_trailer trailer;
  trailer.data_crc( crc() );
  trailer.data_size( fmatchfinder.data_position() );
  trailer.member_size( range_encoder.member_position() + File_trailer::size() );
  for( int i = 0; i < File_trailer::size(); ++i )
    range_encoder.put_byte( trailer.data[i] );
  range_encoder.flush_data();
  }


FLZ_encoder::FLZ_encoder( Fmatchfinder & mf, const File_header & header,
                          const int outfd )
  :
  crc_( 0xFFFFFFFFU ),
  fmatchfinder( mf ),
  range_encoder( outfd ),
  len_encoder( fmatchfinder.match_len_limit() ),
  rep_match_len_encoder( fmatchfinder.match_len_limit() ),
  num_dis_slots( 2 * real_bits( fmatchfinder.dictionary_size() - 1 ) )
  {
  for( int i = 0; i < File_header::size; ++i )
    range_encoder.put_byte( header.data[i] );
  }


bool FLZ_encoder::encode_member( const long long member_size )
  {
  const long long member_size_limit =
    member_size - File_trailer::size() - max_marker_size;
  int rep_distances[num_rep_distances];
  State state;
  for( int i = 0; i < num_rep_distances; ++i ) rep_distances[i] = 0;

  if( fmatchfinder.data_position() != 0 ||
      range_encoder.member_position() != File_header::size )
    return false;			// can be called only once

  if( !fmatchfinder.finished() )		// encode first byte
    {
    const uint8_t prev_byte = 0;
    const uint8_t cur_byte = fmatchfinder[0];
    range_encoder.encode_bit( bm_match[state()][0], 0 );
    literal_encoder.encode( range_encoder, prev_byte, cur_byte );
    crc32.update( crc_, cur_byte );
    move_pos( 1 );
    }

  while( true )
    {
    if( fmatchfinder.finished() ) { full_flush( state ); return true; }

    const int pos_state = fmatchfinder.data_position() & pos_state_mask;
    int dis;
    const int len = sequence_optimizer( rep_distances, &dis, state );
    if( len <= 0 ) return false;

    bool bit = ( dis < 0 && len == 1 );
    range_encoder.encode_bit( bm_match[state()][pos_state], !bit );
    if( bit )				// literal byte
      {
      const uint8_t prev_byte = fmatchfinder[-len-1];
      const uint8_t cur_byte = fmatchfinder[-len];
      crc32.update( crc_, cur_byte );
      if( state.is_char() )
        literal_encoder.encode( range_encoder, prev_byte, cur_byte );
      else
        {
        const uint8_t match_byte = fmatchfinder[-len-rep_distances[0]-1];
        literal_encoder.encode_matched( range_encoder,
                                        prev_byte, cur_byte, match_byte );
        }
      state.set_char();
      }
    else				// match or repeated match
      {
      crc32.update( crc_, fmatchfinder.ptr_to_current_pos() - len, len );
      mtf_reps( dis, rep_distances );
      bit = ( dis < num_rep_distances );
      range_encoder.encode_bit( bm_rep[state()], bit );
      if( bit )
        {
        bit = ( dis == 0 );
        range_encoder.encode_bit( bm_rep0[state()], !bit );
        if( bit )
          range_encoder.encode_bit( bm_len[state()][pos_state], len > 1 );
        else
          {
          range_encoder.encode_bit( bm_rep1[state()], dis > 1 );
          if( dis > 1 )
            range_encoder.encode_bit( bm_rep2[state()], dis > 2 );
          }
        if( len == 1 ) state.set_short_rep();
        else
          {
          rep_match_len_encoder.encode( range_encoder, len, pos_state );
          state.set_rep();
          }
        }
      else
        {
        encode_pair( dis - num_rep_distances, len, pos_state );
        state.set_match();
        }
      }
    if( range_encoder.member_position() >= member_size_limit )
      {
      full_flush( state );
      return true;
      }
    }
  }

#endif

