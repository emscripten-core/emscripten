#include <stdio.h>

typedef void *Class, *id, *SEL;
typedef id (*IMP)(id, SEL, ...);

static id
hello(id self, SEL _cmd)
{
	puts("hello, world!");
	return NULL;
}

/* fake lookup function */
IMP
objc_msg_lookup(id self, SEL _cmd)
{
	return (IMP)hello;
}

/* fake method to register a module */
void
__objc_exec_class(void *module)
{
}

/* create a selector */
@protocol Hello
- (void)hello;
@end

int
main()
{
	struct {
		Class isa;
	} s;
	id obj = (id)&s;

	[obj hello];

	return 0;
}
