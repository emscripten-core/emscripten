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


Dis_slots dis_slots;
Prob_prices prob_prices;


bool Matchfinder::read_block()
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


Matchfinder::Matchfinder( const int dict_size, const int len_limit,
                          const int ifd )
  :
  partial_data_pos( 0 ),
  prev_positions( new int32_t[num_prev_positions] ),
  pos( 0 ),
  cyclic_pos( 0 ),
  stream_pos( 0 ),
  match_len_limit_( len_limit ),
  cycles( ( len_limit < max_match_len ) ? 16 + ( len_limit / 2 ) : 256 ),
  infd( ifd ),
  at_stream_end( false )
  {
  const int buffer_size_limit = ( 2 * dict_size ) + before_size + after_size;
  buffer_size = max( 65536, dict_size );
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
  prev_pos_tree = new int32_t[2*dictionary_size_];
  for( int i = 0; i < num_prev_positions; ++i ) prev_positions[i] = -1;
  }


void Matchfinder::reset()
  {
  const int size = stream_pos - pos;
  if( size > 0 ) memmove( buffer, buffer + pos, size );
  partial_data_pos = 0;
  stream_pos -= pos;
  pos = 0;
  cyclic_pos = 0;
  for( int i = 0; i < num_prev_positions; ++i ) prev_positions[i] = -1;
  read_block();
  }


void Matchfinder::move_pos()
  {
  if( ++cyclic_pos >= dictionary_size_ ) cyclic_pos = 0;
  if( ++pos >= pos_limit )
    {
    if( pos > stream_pos )
      internal_error( "pos > stream_pos in Matchfinder::move_pos" );
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
      for( int i = 0; i < 2 * dictionary_size_; ++i )
        if( prev_pos_tree[i] >= 0 ) prev_pos_tree[i] -= offset;
      read_block();
      }
    }
  }


int Matchfinder::longest_match_len( int * const distances ) throw()
  {
  int len_limit = match_len_limit_;
  if( len_limit > available_bytes() )
    {
    len_limit = available_bytes();
    if( len_limit < 4 ) return 0;
    }

  int maxlen = min_match_len - 1;
  const int min_pos = (pos >= dictionary_size_) ?
                      (pos - dictionary_size_ + 1) : 0;
  const uint8_t * const data = buffer + pos;
  const int key2 = num_prev_positions4 + num_prev_positions3 +
                   ( ( (int)data[0] << 8 ) | data[1] );
  const uint32_t tmp = crc32[data[0]] ^ data[1] ^ ( (uint32_t)data[2] << 8 );
  const int key3 = num_prev_positions4 +
                   (int)( tmp & ( num_prev_positions3 - 1 ) );
  const int key4 = (int)( ( tmp ^ ( crc32[data[3]] << 5 ) ) &
                          ( num_prev_positions4 - 1 ) );

  if( distances )
    {
    int np = prev_positions[key2];
    if( np >= min_pos )
      { distances[2] = pos - np - 1; maxlen = 2; }
    else distances[2] = 0x7FFFFFFF;
    np = prev_positions[key3];
    if( np >= min_pos && buffer[np] == data[0] )
      { distances[3] = pos - np - 1; maxlen = 3; }
    else distances[3] = 0x7FFFFFFF;
    distances[4] = 0x7FFFFFFF;
    }

  prev_positions[key2] = pos;
  prev_positions[key3] = pos;
  int newpos = prev_positions[key4];
  prev_positions[key4] = pos;

  int32_t * ptr0 = prev_pos_tree + ( cyclic_pos << 1 );
  int32_t * ptr1 = ptr0 + 1;
  int len = 0, len0 = 0, len1 = 0;

  for( int count = cycles; ; )
    {
    if( newpos < min_pos || --count < 0 ) { *ptr0 = *ptr1 = -1; break; }
    const uint8_t * const newdata = buffer + newpos;
    while( len < len_limit && newdata[len] == data[len] ) ++len;

    const int delta = pos - newpos;
    if( distances ) while( maxlen < len ) distances[++maxlen] = delta - 1;

    int32_t * const newptr = prev_pos_tree +
      ( ( cyclic_pos - delta +
          ( ( cyclic_pos >= delta ) ? 0 : dictionary_size_ ) ) << 1 );

    if( len < len_limit )
      {
      if( newdata[len] < data[len] )
        {
        *ptr0 = newpos;
        ptr0 = newptr + 1;
        newpos = *ptr0;
        len0 = len; if( len1 < len ) len = len1;
        }
      else
        {
        *ptr1 = newpos;
        ptr1 = newptr;
        newpos = *ptr1;
        len1 = len; if( len0 < len ) len = len0;
        }
      }
    else
      {
      *ptr0 = newptr[0];
      *ptr1 = newptr[1];
      break;
      }
    }
  if( distances )
    {
    if( distances[3] > distances[4] ) distances[3] = distances[4];
    if( distances[2] > distances[3] ) distances[2] = distances[3];
    }
  return maxlen;
  }


void Range_encoder::flush_data()
  {
  if( pos > 0 )
    {
    if( outfd >= 0 && writeblock( outfd, buffer, pos ) != pos )
      throw Error( "Write error" );
    partial_member_pos += pos;
    pos = 0;
    }
  }


void Len_encoder::encode( Range_encoder & range_encoder, int symbol,
                          const int pos_state )
  {
  symbol -= min_match_len;
  if( symbol < len_low_symbols )
    {
    range_encoder.encode_bit( choice1, 0 );
    range_encoder.encode_tree( bm_low[pos_state], symbol, len_low_bits );
    }
  else
    {
    range_encoder.encode_bit( choice1, 1 );
    if( symbol < len_low_symbols + len_mid_symbols )
      {
      range_encoder.encode_bit( choice2, 0 );
      range_encoder.encode_tree( bm_mid[pos_state], symbol - len_low_symbols, len_mid_bits );
      }
    else
      {
      range_encoder.encode_bit( choice2, 1 );
      range_encoder.encode_tree( bm_high, symbol - len_low_symbols - len_mid_symbols, len_high_bits );
      }
    }
  if( --counters[pos_state] <= 0 ) update_prices( pos_state );
  }


void LZ_encoder::fill_align_prices() throw()
  {
  for( int i = 0; i < dis_align_size; ++i )
    align_prices[i] = price_symbol_reversed( bm_align, i, dis_align_bits );
  align_price_count = dis_align_size;
  }


void LZ_encoder::fill_distance_prices() throw()
  {
  for( int dis = start_dis_model; dis < modeled_distances; ++dis )
    {
    const int dis_slot = dis_slots.table( dis );
    const int direct_bits = ( dis_slot >> 1 ) - 1;
    const int base = ( 2 | ( dis_slot & 1 ) ) << direct_bits;
    const int price =
      price_symbol_reversed( bm_dis + base - dis_slot, dis - base, direct_bits );
    for( int dis_state = 0; dis_state < max_dis_states; ++dis_state )
      dis_prices[dis_state][dis] = price;
    }

  for( int dis_state = 0; dis_state < max_dis_states; ++dis_state )
    {
    int * const dsp = dis_slot_prices[dis_state];
    const Bit_model * const bmds = bm_dis_slot[dis_state];
    int slot = 0;
    for( ; slot < end_dis_model && slot < num_dis_slots; ++slot )
      dsp[slot] = price_symbol( bmds, slot, dis_slot_bits );
    for( ; slot < num_dis_slots; ++slot )
      dsp[slot] = price_symbol( bmds, slot, dis_slot_bits ) +
                  (((( slot >> 1 ) - 1 ) - dis_align_bits ) << price_shift );

    int * const dp = dis_prices[dis_state];
    int dis = 0;
    for( ; dis < start_dis_model; ++dis )
      dp[dis] = dsp[dis];
    for( ; dis < modeled_distances; ++dis )
      dp[dis] += dsp[dis_slots.table( dis )];
    }
  }


// Return value == number of bytes advanced (ahead).
// trials[0]..trials[retval-1] contain the steps to encode.
// ( trials[0].dis == -1 && trials[0].price == 1 ) means literal.
int LZ_encoder::sequence_optimizer( const int reps[num_rep_distances],
                                    const State & state )
  {
  int main_len;
  if( longest_match_found > 0 )		// from previous call
    {
    main_len = longest_match_found;
    longest_match_found = 0;
    }
  else main_len = read_match_distances();

  int replens[num_rep_distances];
  int rep_index = 0;
  for( int i = 0; i < num_rep_distances; ++i )
    {
    replens[i] = matchfinder.true_match_len( 0, reps[i] + 1, max_match_len );
    if( replens[i] > replens[rep_index] ) rep_index = i;
    }
  if( replens[rep_index] >= matchfinder.match_len_limit() )
    {
    trials[0].dis = rep_index;
    trials[0].price = replens[rep_index];
    move_pos( replens[rep_index], true );
    return replens[rep_index];
    }

  if( main_len >= matchfinder.match_len_limit() )
    {
    trials[0].dis = match_distances[matchfinder.match_len_limit()] +
                    num_rep_distances;
    trials[0].price = main_len;
    move_pos( main_len, true );
    return main_len;
    }

  {
  const int pos_state = matchfinder.data_position() & pos_state_mask;
  const uint8_t prev_byte = matchfinder[-1];
  const uint8_t cur_byte = matchfinder[0];
  const uint8_t match_byte = matchfinder[-reps[0]-1];

  trials[0].state = state;
  for( int i = 0; i < num_rep_distances; ++i ) trials[0].reps[i] = reps[i];
  trials[1].dis = -1;
  trials[1].prev_index = 0;
  trials[1].price = price0( bm_match[state()][pos_state] );
  if( state.is_char() )
    trials[1].price += literal_encoder.price_symbol( prev_byte, cur_byte );
  else
    trials[1].price += literal_encoder.price_matched( prev_byte, cur_byte, match_byte );

  const int match_price = price1( bm_match[state()][pos_state] );
  const int rep_match_price = match_price + price1( bm_rep[state()] );

  if( match_byte == cur_byte )
    trials[1].update( 0, 0, rep_match_price + price_rep_len1( state, pos_state ) );

  if( main_len < min_match_len )
    {
    trials[0].dis = trials[1].dis;
    trials[0].price = 1;
    matchfinder.move_pos();
    return 1;
    }

  if( main_len <= replens[rep_index] )
    {
    main_len = replens[rep_index];
    for( int len = min_match_len; len <= main_len; ++len )
      trials[len].price = infinite_price;
    }
  else
    {
    const int normal_match_price = match_price + price0( bm_rep[state()] );
    for( int len = min_match_len; len <= main_len; ++len )
      {
      trials[len].dis = match_distances[len] + num_rep_distances;
      trials[len].prev_index = 0;
      trials[len].price = normal_match_price +
                          price_pair( match_distances[len], len, pos_state );
      }
    }

  for( int rep = 0; rep < num_rep_distances; ++rep )
    {
    const int price = rep_match_price +
                      price_rep( rep, state, pos_state );
    for( int len = min_match_len; len <= replens[rep]; ++len )
      trials[len].update( rep, 0, price +
                                  rep_match_len_encoder.price( len, pos_state ) );
    }
  }

  int cur = 0;
  int num_trials = main_len;
  matchfinder.move_pos();

  while( true )
    {
    if( ++cur >= num_trials )		// no more initialized trials
      {
      backward( cur );
      return cur;
      }
    const int newlen = read_match_distances();
    if( newlen >= matchfinder.match_len_limit() )
      {
      longest_match_found = newlen;
      backward( cur );
      return cur;
      }

    Trial & cur_trial = trials[cur];
    const int prev_index = cur_trial.prev_index;

    cur_trial.state = trials[prev_index].state;

    for( int i = 0; i < num_rep_distances; ++i )
      cur_trial.reps[i] = trials[prev_index].reps[i];
    if( prev_index == cur - 1 )
      {
      if( cur_trial.dis == 0 ) cur_trial.state.set_short_rep();
      else cur_trial.state.set_char();
      }
    else
      {
      if( cur_trial.dis < num_rep_distances ) cur_trial.state.set_rep();
      else cur_trial.state.set_match();
      mtf_reps( cur_trial.dis, cur_trial.reps );
      }

    const int pos_state = matchfinder.data_position() & pos_state_mask;
    const uint8_t prev_byte = matchfinder[-1];
    const uint8_t cur_byte = matchfinder[0];
    const uint8_t match_byte = matchfinder[-cur_trial.reps[0]-1];

    int next_price = cur_trial.price +
                     price0( bm_match[cur_trial.state()][pos_state] );
    if( cur_trial.state.is_char() )
      next_price += literal_encoder.price_symbol( prev_byte, cur_byte );
    else
      next_price += literal_encoder.price_matched( prev_byte, cur_byte, match_byte );
    matchfinder.move_pos();

    Trial & next_trial = trials[cur+1];

    next_trial.update( -1, cur, next_price );

    const int match_price = cur_trial.price + price1( bm_match[cur_trial.state()][pos_state] );
    const int rep_match_price = match_price + price1( bm_rep[cur_trial.state()] );

    if( match_byte == cur_byte && next_trial.dis != 0 )
      next_trial.update( 0, cur, rep_match_price +
                                 price_rep_len1( cur_trial.state, pos_state ) );

    const int len_limit = min( min( max_num_trials - 1 - cur,
              matchfinder.available_bytes() ), matchfinder.match_len_limit() );
    if( len_limit < min_match_len ) continue;

    for( int rep = 0; rep < num_rep_distances; ++rep )
      {
      const int dis = cur_trial.reps[rep] + 1;
      int len = 0;
      const uint8_t * const data = matchfinder.ptr_to_current_pos() - 1;
      while( len < len_limit && data[len] == data[len-dis] ) ++len;
      if( len >= min_match_len )
        {
        const int price = rep_match_price +
                          price_rep( rep, cur_trial.state, pos_state );
        while( num_trials < cur + len )
          trials[++num_trials].price = infinite_price;
        for( ; len >= min_match_len; --len )
          trials[cur+len].update( rep, cur, price +
                                  rep_match_len_encoder.price( len, pos_state ) );
        }
      }

    if( newlen <= len_limit &&
        ( newlen > min_match_len ||
          ( newlen == min_match_len &&
            match_distances[min_match_len] < modeled_distances ) ) )
      {
      const int normal_match_price = match_price +
                                     price0( bm_rep[cur_trial.state()] );
      while( num_trials < cur + newlen )
        trials[++num_trials].price = infinite_price;

      int dis = match_distances[min_match_len];
      int dis_state = get_dis_state( min_match_len );
      int dis_price = infinite_price;
      if( dis < modeled_distances )
        trials[cur+min_match_len].update( dis + num_rep_distances, cur,
                   normal_match_price + dis_prices[dis_state][dis] +
                   len_encoder.price( min_match_len, pos_state ) );
      for( int len = min_match_len + 1; len <= newlen; ++len )
        {
        if( dis != match_distances[len] || dis_state < max_dis_states - 1 )
          {
          dis = match_distances[len];
          dis_state = get_dis_state( len );
          dis_price = price_dis( dis, dis_state );
          }
        trials[cur+len].update( dis + num_rep_distances, cur,
                                normal_match_price + dis_price +
                                len_encoder.price( len, pos_state ) );
        }
      }
    }
  }


     // End Of Stream mark => (dis == 0xFFFFFFFFU, len == min_match_len)
void LZ_encoder::full_flush( const State & state )
  {
  const int pos_state = matchfinder.data_position() & pos_state_mask;
  range_encoder.encode_bit( bm_match[state()][pos_state], 1 );
  range_encoder.encode_bit( bm_rep[state()], 0 );
  encode_pair( 0xFFFFFFFFU, min_match_len, pos_state );
  range_encoder.flush();
  File_trailer trailer;
  trailer.data_crc( crc() );
  trailer.data_size( matchfinder.data_position() );
  trailer.member_size( range_encoder.member_position() + File_trailer::size() );
  for( int i = 0; i < File_trailer::size(); ++i )
    range_encoder.put_byte( trailer.data[i] );
  range_encoder.flush_data();
  }


LZ_encoder::LZ_encoder( Matchfinder & mf, const File_header & header,
                        const int outfd )
  :
  longest_match_found( 0 ),
  crc_( 0xFFFFFFFFU ),
  matchfinder( mf ),
  range_encoder( outfd ),
  len_encoder( matchfinder.match_len_limit() ),
  rep_match_len_encoder( matchfinder.match_len_limit() ),
  num_dis_slots( 2 * real_bits( matchfinder.dictionary_size() - 1 ) )
  {
  fill_align_prices();

  for( int i = 0; i < File_header::size; ++i )
    range_encoder.put_byte( header.data[i] );
  }


bool LZ_encoder::encode_member( const long long member_size )
  {
  const long long member_size_limit =
    member_size - File_trailer::size() - max_marker_size;
  const int fill_count = ( matchfinder.match_len_limit() > 12 ) ? 512 : 2048;
  int fill_counter = 0;
  int rep_distances[num_rep_distances];
  State state;
  for( int i = 0; i < num_rep_distances; ++i ) rep_distances[i] = 0;

  if( matchfinder.data_position() != 0 ||
      range_encoder.member_position() != File_header::size )
    return false;			// can be called only once

  if( !matchfinder.finished() )		// encode first byte
    {
    const uint8_t prev_byte = 0;
    const uint8_t cur_byte = matchfinder[0];
    range_encoder.encode_bit( bm_match[state()][0], 0 );
    literal_encoder.encode( range_encoder, prev_byte, cur_byte );
    crc32.update( crc_, cur_byte );
    move_pos( 1 );
    }

  while( true )
    {
    if( matchfinder.finished() ) { full_flush( state ); return true; }
    if( fill_counter <= 0 )
      { fill_distance_prices(); fill_counter = fill_count; }

    int ahead = sequence_optimizer( rep_distances, state );
    if( ahead <= 0 ) return false;
    fill_counter -= ahead;

    for( int i = 0; ; )
      {
      const int pos_state =
        ( matchfinder.data_position() - ahead ) & pos_state_mask;
      const int dis = trials[i].dis;
      const int len = trials[i].price;

      bool bit = ( dis < 0 && len == 1 );
      range_encoder.encode_bit( bm_match[state()][pos_state], !bit );
      if( bit )				// literal byte
        {
        const uint8_t prev_byte = matchfinder[-ahead-1];
        const uint8_t cur_byte = matchfinder[-ahead];
        crc32.update( crc_, cur_byte );
        if( state.is_char() )
          literal_encoder.encode( range_encoder, prev_byte, cur_byte );
        else
          {
          const uint8_t match_byte = matchfinder[-ahead-rep_distances[0]-1];
          literal_encoder.encode_matched( range_encoder,
                                          prev_byte, cur_byte, match_byte );
          }
        state.set_char();
        }
      else				// match or repeated match
        {
        crc32.update( crc_, matchfinder.ptr_to_current_pos() - ahead, len );
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
      ahead -= len; i += len;
      if( range_encoder.member_position() >= member_size_limit )
        {
        if( !matchfinder.dec_pos( ahead ) ) return false;
        full_flush( state );
        return true;
        }
      if( ahead <= 0 ) break;
      }
    }
  }

#endif

