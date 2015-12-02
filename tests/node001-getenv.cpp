#include <stdio.h>       
#include <emscripten.h>

int main ()
{
 char *s = getenv ( process.env.APPDATA );
 printf ( "\n NODE.JS <%s> " ,s  ) ; 
 printf ( "\n NODE.JS <%s> " , getenv ( process.env.HOMEDRIVE ) ) ;
 printf ( "\n NODE.JS <%s> " , getenv ( process.env ) ) ; 

  return 0;
}

/* OUTPUT

 NODE.JS <"C:\\Users\\claudio\\AppData\\Roaming"> 
 NODE.JS <"C:"> 
 ...

*/
