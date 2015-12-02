#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <emscripten.h>

int main()
{
 sigset_t old_set,new_set;
 
 sigemptyset(&old_set);
 sigemptyset(&new_set);

 if(sigaddset(&old_set,SIGSEGV)==0)
 {
   printf("sigaddset successfully added for SIGSEGV\n");
 }
 sigprocmask(SIG_BLOCK,&old_set,NULL); // SIGSEGV signal is masked
 //kill(0,SIGSEGV);


 //*****************************************************************

 if(sigaddset(&new_set,SIGRTMIN)==0)
 {
  printf("sigaddset successfully added for SIGRTMIN\n");
 }
  sigprocmask(SIG_BLOCK,&new_set,&old_set); // SIGRTMIN signal is masked
 //kill(0,SIGSEGV);

 //****************** Unblock one signal at a time ******************

 sigprocmask(SIG_UNBLOCK,&new_set,&old_set); // SIGRTMIN signal is unmasked
 sigprocmask(SIG_UNBLOCK,&new_set,&old_set); // SIGSEGV signal is unmasked
}

 
