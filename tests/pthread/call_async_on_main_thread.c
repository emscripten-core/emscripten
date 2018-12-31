#include <stdio.h>
#include <assert.h>
#include <string.h>

extern void report_result(int param1, int param2, int param3);

int main()
{
	report_result(1, 2, 3);
}
