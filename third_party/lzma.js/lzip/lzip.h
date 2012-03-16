#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

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

class State
  {
  unsigned char st;

public:
  enum { states = 12 };
  State() : st( 0 ) {}
  unsigned char operator()() const { return st; }
  bool is_char() const { return st < 7; }

  void set_char()
    {
    static const unsigned char next[states] =
      { 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 4, 5 };
    st = next[st];
    }

  void set_match()
    {
    static const unsigned char next[states] =
      { 7, 7, 7, 7, 7, 7, 7, 10, 10, 10, 10, 10 };
    st = next[st];
    }

  void set_rep()
    {
    static const unsigned char next[states] =
      { 8, 8, 8, 8, 8, 8, 8, 11, 11, 11, 11, 11 };
    st = next[st];
    }

  void set_short_rep()
    {
    static const unsigned char next[states] =
      { 9, 9, 9, 9, 9, 9, 9, 11, 11, 11, 11, 11 };
    st = next[st];
    }
  };


enum {
  min_dictionary_bits = 12,
  min_dictionary_size = 1 << min_dictionary_bits,
  max_dictionary_bits = 29,
  max_dictionary_size = 1 << max_dictionary_bits,
  literal_context_bits = 3,
  pos_state_bits = 2,
  pos_states = 1 << pos_state_bits,
  pos_state_mask = pos_states - 1,

  dis_slot_bits = 6,
  start_dis_model = 4,
  end_dis_model = 14,
  modeled_distances = 1 << (end_dis_model / 2),
  dis_align_bits = 4,
  dis_align_size = 1 << dis_align_bits,

  len_low_bits = 3,
  len_mid_bits = 3,
  len_high_bits = 8,
  len_low_symbols = 1 << len_low_bits,
  len_mid_symbols = 1 << len_mid_bits,
  len_high_symbols = 1 << len_high_bits,
  max_len_symbols = len_low_symbols + len_mid_symbols + len_high_symbols,

  min_match_len = 2,					// must be 2
  max_match_len = min_match_len + max_len_symbols - 1,	// 273
  min_match_len_limit = 5,

  max_dis_states = 4 };

inline int get_dis_state( int len )
  {
  len -= min_match_len;
  if( len >= max_dis_states ) len = max_dis_states - 1;
  return len;
  }


enum { bit_model_move_bits = 5,
       bit_model_total_bits = 11,
       bit_model_total = 1 << bit_model_total_bits };

struct Bit_model
  {
  unsigned int probability;
  Bit_model() : probability( bit_model_total / 2 ) {}
  };


class CRC32
  {
  uint32_t data[256];		// Table of CRCs of all 8-bit messages.

public:
  CRC32()
    {
    for( unsigned int n = 0; n < 256; ++n )
      {
      unsigned int c = n;
      for( int k = 0; k < 8; ++k )
        { if( c & 1 ) c = 0xEDB88320U ^ ( c >> 1 ); else c >>= 1; }
      data[n] = c;
      }
    }

  uint32_t operator[]( const uint8_t byte ) const { return data[byte]; }
  void update( uint32_t & crc, const uint8_t byte ) const
    { crc = data[(crc^byte)&0xFF] ^ ( crc >> 8 ); }
  void update( uint32_t & crc, const uint8_t * const buffer, const int size ) const
    {
    for( int i = 0; i < size; ++i )
      crc = data[(crc^buffer[i])&0xFF] ^ ( crc >> 8 );
    }
  };

extern const CRC32 crc32;


inline int real_bits( const int value )
  {
  int bits = 0;
  for( int i = 1, mask = 1; mask > 0; ++i, mask <<= 1 )
    if( value & mask ) bits = i;
  return bits;
  }

const uint8_t magic_string[4] = { 'L', 'Z', 'I', 'P' };

struct File_header
  {
  uint8_t data[6];			// 0-3 magic bytes
					//   4 version
					//   5 coded_dict_size
  enum { size = 6 };

  void set_magic()
    { memcpy( data, magic_string, 4 ); data[4] = 1; }

  bool verify_magic() const
    { return ( memcmp( data, magic_string, 4 ) == 0 ); }

  uint8_t version() const { return data[4]; }
  bool verify_version() const { return ( data[4] <= 1 ); }

  int dictionary_size() const
    {
    int sz = ( 1 << ( data[5] & 0x1F ) );
    if( sz > min_dictionary_size && sz <= max_dictionary_size )
      sz -= ( sz / 16 ) * ( ( data[5] >> 5 ) & 0x07 );
    return sz;
    }

  bool dictionary_size( const int sz )
    {
    if( sz >= min_dictionary_size && sz <= max_dictionary_size )
      {
      data[5] = real_bits( sz - 1 );
      if( sz > min_dictionary_size )
        {
        const int base_size = 1 << data[5];
        const int wedge = base_size / 16;
        for( int i = 7; i >= 1; --i )
          if( base_size - ( i * wedge ) >= sz )
            { data[5] |= ( i << 5 ); break; }
        }
      return true;
      }
    return false;
    }
  };


struct File_trailer
  {
  uint8_t data[20];	//  0-3  CRC32 of the uncompressed data
			//  4-11 size of the uncompressed data
			// 12-19 member size including header and trailer

  static int size( const int version = 1 )
    { return ( ( version >= 1 ) ? 20 : 12 ); }

  uint32_t data_crc() const
    {
    uint32_t tmp = 0;
    for( int i = 3; i >= 0; --i ) { tmp <<= 8; tmp += data[i]; }
    return tmp;
    }

  void data_crc( uint32_t crc )
    { for( int i = 0; i <= 3; ++i ) { data[i] = (uint8_t)crc; crc >>= 8; } }

  long long data_size() const
    {
    long long tmp = 0;
    for( int i = 11; i >= 4; --i ) { tmp <<= 8; tmp += data[i]; }
    return tmp;
    }

  void data_size( long long sz )
    {
    for( int i = 4; i <= 11; ++i ) { data[i] = (uint8_t)sz; sz >>= 8; }
    }

  long long member_size() const
    {
    long long tmp = 0;
    for( int i = 19; i >= 12; --i ) { tmp <<= 8; tmp += data[i]; }
    return tmp;
    }

  void member_size( long long sz )
    {
    for( int i = 12; i <= 19; ++i ) { data[i] = (uint8_t)sz; sz >>= 8; }
    }
  };


struct Error
  {
  const char * const msg;
  Error( const char * const s ) : msg( s ) {}
  };


// defined in main.cc lziprecover.cc
void show_error( const char * const msg, const int errcode = 0,
                 const bool help = false );
void internal_error( const char * const msg );

// defined in decoder.cc
int readblock( const int fd, uint8_t * const buf, const int size );
int writeblock( const int fd, const uint8_t * const buf, const int size );

// XXX
extern void pp(const char *p=NULL);

