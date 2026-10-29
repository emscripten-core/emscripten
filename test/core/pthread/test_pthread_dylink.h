extern int foo_main;  // Data defind in main
extern int foo_side;  // Data defind in side

int bar_main();  // Function defined in main
int bar_side();  // Function defined in side

void print_addresses_side();

typedef int (*bar_func)();
bar_func bar_main_addr_side();  // &bar_main, as seen by side
