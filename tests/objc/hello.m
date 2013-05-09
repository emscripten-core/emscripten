#include <stdio.h>
@interface X
@end
@implementation X
+(void)print
{
	printf("Hello\n");
}
@end

int main()
{
	[X print];
	return 0;
}
