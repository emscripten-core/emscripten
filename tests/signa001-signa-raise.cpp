#include <stdio.h>       
#include <signal.h>      

void mySTUB1 (int param)
{
  puts ( "ONE!");
}
void mySTUB2 (int param)
{
  puts ( "TWO!");
}

int main ()
{
    // ........................ signal handler
	signal (SIGINT  , NULL );
	signal (SIGABRT, mySTUB2 );
    // ........................ raise risgnal
	raise(SIGINT);
	raise(SIGABRT);
    // ........................ raise undef handler
	printf ( "%d" ,raise(SIGSTOP) ) ;
    // ........................ raise undef signal
	printf ( "%d" ,raise(-2) ) ;
	

  return 0;
}
