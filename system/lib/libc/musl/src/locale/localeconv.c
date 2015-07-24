#include <locale.h>

static const struct lconv posix_lconv = {
	.decimal_point = ".",
	.thousands_sep = "",
	.grouping = "",
	.int_curr_symbol = "",
	.currency_symbol = "",
	.mon_decimal_point = "",
	.mon_thousands_sep = "",
	.mon_grouping = "",
	.positive_sign = "",
	.negative_sign = "",
	.int_frac_digits = -1,
	.frac_digits = -1,
	.p_cs_precedes = -1,
	.p_sep_by_space = -1,
	.n_cs_precedes = -1,
	.n_sep_by_space = -1,
	.p_sign_posn = -1,
	.n_sign_posn = -1,
	.int_p_cs_precedes = -1,
	.int_p_sep_by_space = -1,
	.int_n_cs_precedes = -1,
	.int_n_sep_by_space = -1,
	.int_p_sign_posn = -1,
	.int_n_sign_posn = -1,
};

struct lconv *localeconv(void)
{
	return (void *)&posix_lconv;
}
