/* The Computer Language Benchmarks Game
   http://shootout.alioth.debian.org/
   contributed by Andrew Moon
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// limit output, so we do not benchmark speed of printing
void puts_limited(char *x)
{
  static int left = 550;
  int len = strlen(x);
  if (len <= left) {
    puts(x);
    left -= len;
    return;
  }
  if (left > 0) {
    x[left] = '\0';
    puts(x);
    x[left] = 'z';
    left = 0;
  }
}

struct Random {
   enum { IM = 139968, IA = 3877, IC = 29573 };
   Random() : last(42) {}
   float get( float max = 1.0f ) {
      last = ( last * IA + IC ) % IM;
      return max * last / IM;
   }
protected:
   unsigned int last;
} rng;

struct IUB {
   int c;
   double p;
   unsigned int pi;
};

struct Cumulative {
   enum { slots = 512, };

   Cumulative( IUB *start ) {
      double p = 0;
      for ( IUB *iter = start; iter->c; ++iter ) {
         p += iter->p;
         iter->p = p < 1.0 ? p : 1.0;
         iter->pi = (unsigned int )( iter->p * slots );
      }

      for ( unsigned int i = 0; i <= slots; i++ ) {
         while ( i > start->pi && start->pi != 0) {
            ++start;
          }

         table[i] = start;
      }
   }

   const char operator[] ( float pct ) const {
      IUB *iter = table[(unsigned int )( pct * slots )];
      while ( iter->p < pct )
         ++iter;
      return iter->c;
   }

protected:
   IUB *table[slots + 1];
};

static const size_t lineLength = 60;

struct LineBuffer {
   LineBuffer() : lastN(0) {}
   LineBuffer &genrand( Cumulative &table, size_t N ) {
      //assert(N <= lineLength);
      for ( size_t i = 0; i < N; i++ )
         buffer[i] = table[rng.get()];
      buffer[N] = '\n';
      buffer[N+1] = '\0';
      lastN = N + 1;
      return *this;
   }
   void writeline() { puts_limited(buffer); }
protected:
   char buffer[lineLength + 2];
   size_t lastN;
};

struct RotatingString {
   RotatingString( const char *in ) : pos(0) {
      size = strlen( in );
      buffer = new char[size + lineLength];
      memcpy( buffer, in, size );
      memcpy( buffer + size, in, lineLength );
   }
   ~RotatingString() { delete[] buffer; }
   void write( size_t bytes ) {
      char* temp = new char[bytes+2];
      memcpy(temp, buffer + pos, bytes);
      temp[bytes] = '\n';
      temp[bytes] = '\0';
      puts_limited(temp);
      delete temp;
      pos += bytes;
      if ( pos > size )
         pos -= size;
   }
protected:
   char *buffer;
   size_t size, pos;
};

template< class Output >
void makeFasta( const char *id, const char *desc, size_t N, Output &output ) {
   while ( N ) {
      const size_t bytes = N < lineLength ? N : lineLength;
      output.writeline( bytes );
      N -= bytes;
   }
}

struct Repeater {
   Repeater( const char *alu ) : rot(alu) {}
   void writeline( size_t bytes ) { rot.write( bytes ); }
   void run( const char *id, const char *desc, size_t N ) {
      makeFasta( id, desc, N, *this );
   }
protected:
   RotatingString rot;
};

struct Randomized {
   Randomized( IUB *start ) : table(start) {}
   void writeline( size_t bytes ) { line.genrand(table, bytes).writeline(); }
   void run( const char *id, const char *desc, size_t N ) {
      makeFasta( id, desc, N, *this );
   }
protected:
   Cumulative table;
   LineBuffer line;
};

IUB iub[] = {
   { 'a', 0.27, 0 },
   { 'c', 0.12, 0 },
   { 'g', 0.12, 0 },
   { 't', 0.27, 0 },

   { 'B', 0.02, 0 },
   { 'D', 0.02, 0 },
   { 'H', 0.02, 0 },
   { 'K', 0.02, 0 },
   { 'M', 0.02, 0 },
   { 'N', 0.02, 0 },
   { 'R', 0.02, 0 },
   { 'S', 0.02, 0 },
   { 'V', 0.02, 0 },
   { 'W', 0.02, 0 },
   { 'Y', 0.02, 0 },
   {   0,    0, 0 },
};

IUB homosapiens[] = {
   { 'a', 0.3029549426680, 0 },
   { 'c', 0.1979883004921, 0 },
   { 'g', 0.1975473066391, 0 },
   { 't', 0.3015094502008, 0 },
   {   0,               0, 0 },
};

static const char alu[] =
   "GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTG"
   "GGAGGCCGAGGCGGGCGGATCACCTGAGGTCAGGAGTTCGA"
   "GACCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAA"
   "AATACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAAT"
   "CCCAGCTACTCGGGAGGCTGAGGCAGGAGAATCGCTTGAAC"
   "CCGGGAGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTG"
   "CACTCCAGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAA";

int main( int argc, const char *argv[] ) {
   const size_t n = ( argc > 1 ) ? atoi( argv[1] ) : 512;

   Repeater(alu)
      .run( "ONE", "Homo sapiens alu", n*2 );
   Randomized(iub)
      .run( "TWO", "IUB ambiguity codes", n*3 );
   Randomized(homosapiens)
      .run( "THREE", "Homo sapiens frequency", n*5 );

   return 0;
}

