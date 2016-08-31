// This is from// http://pic.dhe.ibm.com/infocenter/iseries/v7r1m0/index.jsp?topic=%2Frtref%2Fregexec.htm
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  regex_t preg;
  const char *string = "a very simple simple simple string";
  const char *pattern = "\\(sim[a-z]le\\) \\1";
  int rc;
  size_t nmatch = 2;
  regmatch_t pmatch[2];

  if (0 != (rc = regcomp(&preg, pattern, 0))) {
    printf("regcomp() failed, returning nonzero (%d)\n", rc);
    exit(EXIT_FAILURE);
  }

  if (0 != (rc = regexec(&preg, string, nmatch, pmatch, 0))) {
    printf("Failed to match '%s' with '%s',returning %d.\n", string, pattern,
           rc);
  } else {
    printf(
        "With the whole expression, "
        "a matched substring \"%.*s\" is found at position %d to %d.\n",
        pmatch[0].rm_eo - pmatch[0].rm_so, &string[pmatch[0].rm_so],
        pmatch[0].rm_so, pmatch[0].rm_eo - 1);
    printf(
        "With the sub-expression, "
        "a matched substring \"%.*s\" is found at position %d to %d.\n",
        pmatch[1].rm_eo - pmatch[1].rm_so, &string[pmatch[1].rm_so],
        pmatch[1].rm_so, pmatch[1].rm_eo - 1);
  }
  regfree(&preg);
  return 0;
}
