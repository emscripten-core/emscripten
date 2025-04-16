#include <unistd.h>
#include <string.h>

int main()
{
	char msg[] = "Hello!\n";
	write(STDOUT_FILENO, msg, strlen(msg));
}
