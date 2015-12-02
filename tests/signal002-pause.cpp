#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <emscripten.h>

// compile with :
//
// em++ signal002-pause.cpp -o main.js -s ASYNCIFY=1
//


void mystop (int param)
{
  puts ( "stop!");
}
void mycont (int param)
{
  puts ( "cont!");
}
int main (int argc, char *argv[])
{
     // ........................ signal handler
	signal (SIGSTOP, mystop );
	signal (SIGCONT, mycont );
	
    pause ( 1000 ) ;
	
	return 0;
}
