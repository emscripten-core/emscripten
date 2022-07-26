/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <locale.h>
#include <stdio.h>

int main()
{
    // Test basic functions from classic locale.
    struct lconv* locale = localeconv();

    printf("Testing locale information.\n");
    printf("Decimal point: %s\n", locale->decimal_point);
    printf("Thousands separator: %s\n", locale->thousands_sep);
    printf("Grouping: %s\n", locale->grouping);
    printf("International currency symbol: %s\n", locale->int_curr_symbol);
    printf("Currency symbol: %s\n", locale->currency_symbol);
    printf("Money decimal point: %s\n", locale->mon_decimal_point);
    printf("Money thousands separator: %s\n", locale->mon_thousands_sep);
    printf("Money Grouping: %s\n", locale->mon_grouping);
    printf("Positive sign: %s\n", locale->positive_sign);
    printf("Negative sign: %s\n", locale->negative_sign);
    
    // If no runtime errors, assume the test passed.
    printf("Locale tests passed.\n");
    return 0;
}
