#include <stdio.h>   
#include <stdlib.h>    
#include <emscripten.h>

int main ()
{
	system ( "dir" ) ; 
	system ( "node -v" ) ;
	system ( "dir c:\\windows" );
	system ( "em++ node001-getenv.cpp -o main.js" );

  return 0;
}

/* OUTPUT
 
v4.1.1

 Il volume nell'unit� C � ROOT
 Numero di serie del volume: 06E6-D93D

 Directory di c:\windows

24/10/2015  13.08    <DIR>          .
24/10/2015  13.08    <DIR>          ..
10/07/2015  12.04    <DIR>          addins
30/10/2015  10.59    <DIR>          appcompat
24/11/2015  03.15    <DIR>          AppPatch
02/12/2015  15.29    <DIR>          AppReadiness
...
10/07/2015  12.00            11.264 write.exe
              23 File    721.754.485 byte
              70 Directory  170.484.563.968 byte disponibili

 Il volume nell'unit� C � ROOT
 Numero di serie del volume: 06E6-D93D

 Directory di C:\wamp\www\em\bug

02/12/2015  15.44    <DIR>          .
02/12/2015  15.44    <DIR>          ..
18/11/2015  21.59               137 index.html
02/12/2015  15.44           370.407 main.js
...
02/12/2015  15.22             2.006 signal003-sigprocmask.cpp
02/12/2015  15.44                 0 txt.txt
23/11/2015  14.58               946 type.signal.CPP
              10 File        376.049 byte
               2 Directory  170.484.563.968 byte disponibili



*/


