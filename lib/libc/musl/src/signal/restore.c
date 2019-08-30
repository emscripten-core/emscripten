/* These functions will not work, but suffice for targets where the
 * kernel sigaction structure does not actually use sa_restorer. */

void __restore()
{
}

void __restore_rt()
{
}
