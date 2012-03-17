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

class Fmatchfinder
  {
  enum { // bytes to keep in buffer before dictionary
         before_size = max_match_len + 1,
         // bytes to keep in buffer after pos
         after_size = max_match_len,
         num_prev_positions = 1 << 16 };

  long long partial_data_pos;
  uint8_t * buffer;		// input buffer
  int32_t * const prev_positions;	// last seen position of key
  int32_t * prev_pos_chain;
  int dictionary_size_;		// bytes to keep in buffer before pos
  int buffer_size;
  int pos;			// current pos in buffer
  int cyclic_pos;		// current pos in dictionary
  int key4;			// key made from latest 4 bytes
  int stream_pos;		// first byte not yet read from file
  int pos_limit;		// when reached, a new block must be read
  const int match_len_limit_;
  const int infd;		// input file descriptor
  bool at_stream_end;		// stream_pos shows real end of file

  bool read_block();

public:
  Fmatchfinder( const int ifd );

  ~Fmatchfinder()
    { delete[] prev_pos_chain; delete[] prev_positions; free( buffer ); }

  uint8_t operator[]( const int i ) const { return buffer[pos+i]; }
  int available_bytes() const { return stream_pos - pos; }
  long long data_position() const { return partial_data_pos + pos; }
  int dictionary_size() const { return dictionary_size_; }
  bool finished() const { return at_stream_end && pos >= stream_pos; }
  int match_len_limit() const { return match_len_limit_; }
  const uint8_t * ptr_to_current_pos() const { return buffer + pos; }

  int true_match_len( const int index, const int distance, int len_limit ) const
    {
    if( index + len_limit > available_bytes() )
      len_limit = available_bytes() - index;
    const uint8_t * const data = buffer + pos + index - distance;
    int i = 0;
    while( i < len_limit && data[i] == data[i+distance] ) ++i;
    return i;
    }

  void reset();
  void move_pos();
  int longest_match_len( int * const distance );
  void longest_match_len();
  };


class FLZ_encoder
  {
  enum { max_marker_size = 16,
         num_rep_distances = 4 };	// must be 4

  uint32_t crc_;

  Bit_model bm_match[State::states][pos_states];
  Bit_model bm_rep[State::states];
  Bit_model bm_rep0[State::states];
  Bit_model bm_rep1[State::states];
  Bit_model bm_rep2[State::states];
  Bit_model bm_len[State::states][pos_states];
  Bit_model bm_dis_slot[max_dis_states][1<<dis_slot_bits];
  Bit_model bm_dis[modeled_distances-end_dis_model+1];
  Bit_model bm_align[dis_align_size];

  Fmatchfinder & fmatchfinder;
  Range_encoder range_encoder;
  Len_encoder len_encoder;
  Len_encoder rep_match_len_encoder;
  Literal_encoder literal_encoder;

  const int num_dis_slots;
  int match_distance;

  uint32_t crc() const { return crc_ ^ 0xFFFFFFFFU; }

       // move-to-front dis in/into reps
  void mtf_reps( const int dis, int reps[num_rep_distances] )
    {
    if( dis >= num_rep_distances )
      {
      for( int i = num_rep_distances - 1; i > 0; --i ) reps[i] = reps[i-1];
      reps[0] = dis - num_rep_distances;
      }
    else if( dis > 0 )
      {
      const int distance = reps[dis];
      for( int i = dis; i > 0; --i ) reps[i] = reps[i-1];
      reps[0] = distance;
      }
    }

  void encode_pair( const uint32_t dis, const int len, const int pos_state )
    {
    len_encoder.encode( range_encoder, len, pos_state );
    const int dis_slot = dis_slots[dis];
    range_encoder.encode_tree( bm_dis_slot[get_dis_state(len)], dis_slot, dis_slot_bits );

    if( dis_slot >= start_dis_model )
      {
      const int direct_bits = ( dis_slot >> 1 ) - 1;
      const uint32_t base = ( 2 | ( dis_slot & 1 ) ) << direct_bits;
      const uint32_t direct_dis = dis - base;

      if( dis_slot < end_dis_model )
        range_encoder.encode_tree_reversed( bm_dis + base - dis_slot,
                                            direct_dis, direct_bits );
      else
        {
        range_encoder.encode( direct_dis >> dis_align_bits, direct_bits - dis_align_bits );
        range_encoder.encode_tree_reversed( bm_align, direct_dis, dis_align_bits );
        }
      }
    }

  int read_match_distances()
    {
    int len = fmatchfinder.longest_match_len( &match_distance );
    if( len == fmatchfinder.match_len_limit() )
      len += fmatchfinder.true_match_len( len, match_distance + 1, max_match_len - len );
    return len;
    }

  void move_pos( int n, bool skip = false )
    {
    while( --n >= 0 )
      {
      if( skip ) skip = false;
      else fmatchfinder.longest_match_len();
      fmatchfinder.move_pos();
      }
    }

  int sequence_optimizer( const int reps[num_rep_distances],
                          int * const disp, const State & state );

  void full_flush( const State & state );

public:
  FLZ_encoder( Fmatchfinder & mf, const File_header & header, const int outfd );

  bool encode_member( const long long member_size );

  long long member_position() const
    { return range_encoder.member_position(); }
  };
