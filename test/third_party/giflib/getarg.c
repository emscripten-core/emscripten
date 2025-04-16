/***************************************************************************

getarg.c - routines to grab the parameters from the command line:

Names of all the routines except the main one start with GA (Get
Arguments) to prevent conflicts.

The following routines are available in this module:

1. int GAGetArgs(argc, argv, CtrlStr, Variables...)
where argc, argv are received on entry.
CtrlStr is the contrl string (see below)
Variables are all the variables to be set according to CtrlStr.
Note that all the variables MUST be transfered by address.
Return 0 on correct parsing, otherwise error number (see GetArg.h).

2. GAPrintHowTo(CtrlStr)
Print the control string to stderr, in the correct format.
This feature is very useful in case of an error during GetArgs parsing.
Chars equal to SPACE_CHAR are not printed (regular spaces are NOT
allowed, and so using SPACE_CHAR you can create space in PrintHowTo).

3. GAPrintErrMsg(Error)
Describe the error to stderr, according to Error (usually returned by
GAGetArgs).

CtrlStr format:

The control string passed to GetArgs controls the way argv (argc) are
parsed. Each entry in this string must not have any spaces in it. The
First Entry is the name of the program, which is usually ignored except
when GAPrintHowTo is called. All the other entries (except the last one
which we will come back to later) must have the following format:

1. One letter which sets the option letter.
2. '!' or '%' to determines if this option is really optional ('%') or
   required ('!')...
3. '-' must always be given.
4. Alphanumeric string, usually ignored, but used by GAPrintHowTo to
   print the meaning of this option.
5. Sequences starts with '!' or '%'. Again if '!' then this sequence
   must exist (only if its option flag is given of course), and if '%'
   it is optional. Each sequence will continue with one or two
   characters which defines the kind of the input:
a: d, x, o, u - integer is expected (decimal, hex, octal base or unsigned).
b: D, X, O, U - long integer is expected (same as above).
c: f - float number is expected.
d: F - double number is expected.
e: s - string is expected.
f: *? - any number of '?' kind (d, x, o, u, D, X, O, U, f, F, s)
   will match this one. If '?' is numeric, it scans until
   non-numeric input is given. If '?' is 's' then it scans
   up to the next option or end of argv.

If the last parameter given in the CtrlStr, is not an option (i.e. the
second char is not in ['!', '%'] and the third one is not '-'), all what
remained from argv is linked to it.

The variables passed to GAGetArgs (starting from 4th parameter) MUST
match the order of the CtrlStr:

For each option, one integer address must be passed. This integer must
be initialized with 0. If that option is given in the command line, it will
be set.

In addition, the sequences that might follow an option require the
following parameters to pass:

1. d, x, o, u - pointer to integer (int *).
2. D, X, O, U - pointer to long (long *).
3. f - pointer to float (float *).
4. F - pointer to double (double *).
5. s - pointer to char (char *). NO allocation is needed!
6. *? - TWO variables are passed for each wild request. the first
   one is (address of) integer, and it will return number of
   parameters actually matched this sequence, and the second
   one is a pointer to pointer to ? (? **), and will return an
   address to a block of pointers to ? kind, terminated with
   NULL pointer. NO pre-allocation is required. The caller is
   responsible for freeing this memory, including the pointed to
   memory.

Note that these two variables are pretty like the argv/argc pair...

Examples:

"Example1 i%-OneInteger!d s%-Strings!*s j%- k!-Float!f Files"

Will match: Example1 -i 77 -s String1 String2 String3 -k 88.2 File1 File2
or: Example1 -s String1 -k 88.3 -i 999 -j
but not: Example1 -i 77 78 (option i expects one integer, k must be).

Note the option k must exist, and that the order of the options is not
important. In the first examples File1 & File2 will match the Files
in the command line.

A call to GAPrintHowTo with this CtrlStr will print to stderr:
Example1 [-i OneIngeter] [-s Strings...] [-j] -k Float Files...

Notes:

1. This module assumes that all the pointers to all kind of data types
have the same length and format, i.e. sizeof(int *) == sizeof(char *).

SPDX-License-Identifier: MIT

**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
 
#include "getarg.h"

#define MAX_PARAM           100    /* maximum number of parameters allowed. */
#define CTRL_STR_MAX_LEN    1024

#define SPACE_CHAR '|'  /* The character not to print using HowTo. */

#define ARG_OK false

#define ISSPACE(x) ((x) <= ' ') /* Not conventional - but works fine! */

/* The two characters '%' and '!' are used in the control string: */
#define ISCTRLCHAR(x) (((x) == '%') || ((x) == '!'))

static char *GAErrorToken;  /* On error, ErrorToken is set to point to it. */
static int GATestAllSatis(char *CtrlStrCopy, char *CtrlStr, char **argv_end,
                          char ***argv, void *Parameters[MAX_PARAM],
                          int *ParamCount);
static bool GAUpdateParameters(void *Parameters[], int *ParamCount,
                              char *Option, char *CtrlStrCopy, char *CtrlStr,
                              char **argv_end, char ***argv);
static int GAGetParmeters(void *Parameters[], int *ParamCount,
                          char *CtrlStrCopy, char *Option, char **argv_end,
                          char ***argv);
static int GAGetMultiParmeters(void *Parameters[], int *ParamCount,
                               char *CtrlStrCopy, char **argv_end, char ***argv);
static void GASetParamCount(char *CtrlStr, int Max, int *ParamCount);
static bool GAOptionExists(char **argv_end, char **argv);

/***************************************************************************
 Allocate or die
***************************************************************************/
static void *
xmalloc(unsigned size) {

    void *p;

    if ((p = malloc(size)) != NULL)
        return p;

    fprintf(stderr, "Not enough memory, exit.\n");
    exit(2);

    return NULL;    /* Makes warning silent. */
}
/***************************************************************************
 Routine to access the command line argument and interpret them:       
 Return ARG_OK (0) is case of successful parsing, error code else...       
***************************************************************************/
bool
GAGetArgs(int argc,
        char **argv,
        char *CtrlStr, ...) {

    int i, ParamCount = 0;
    void *Parameters[MAX_PARAM];     /* Save here parameter addresses. */
    char CtrlStrCopy[CTRL_STR_MAX_LEN];
    char **argv_end = argv + argc;
    va_list ap;

    strncpy(CtrlStrCopy, CtrlStr, sizeof(CtrlStrCopy)-1);
    GASetParamCount(CtrlStr, strlen(CtrlStr), &ParamCount);
    va_start(ap, CtrlStr);
    for (i = 1; i <= ParamCount; i++)
        Parameters[i - 1] = va_arg(ap, void *);
    va_end(ap);

    argv++;    /* Skip the program name (first in argv/c list). */
    while (argv < argv_end) {
	bool Error = false;
        if (!GAOptionExists(argv_end, argv))
            break;    /* The loop. */
        char *Option = *argv++;
        if ((Error = GAUpdateParameters(Parameters, &ParamCount, Option,
                                        CtrlStrCopy, CtrlStr, argv_end,
                                        &argv)) != false)
            return Error;
    }
    /* Check for results and update trail of command line: */
    return GATestAllSatis(CtrlStrCopy, CtrlStr, argv_end, &argv, Parameters,
                          &ParamCount) != ARG_OK;
}

/***************************************************************************
 Routine to search for unsatisfied flags - simply scan the list for !- 
 sequence. Before this scan, this routine updates the rest of the command
 line into the last two parameters if it is requested by the CtrlStr 
 (last item in CtrlStr is NOT an option). 
 Return ARG_OK if all satisfied, CMD_ERR_AllSatis error else. 
***************************************************************************/
static int
GATestAllSatis(char *CtrlStrCopy,
               char *CtrlStr,
               char **argv_end,
               char ***argv,
               void *Parameters[MAX_PARAM],
               int *ParamCount) {

    int i;
    static char *LocalToken = NULL;

    /* If LocalToken is not initialized - do it now. Note that this string
     * should be writable as well so we can not assign it directly.
     */
    if (LocalToken == NULL) {
        LocalToken = (char *)malloc(3);
        strcpy(LocalToken, "-?");
    }

    /* Check if last item is an option. If not then copy rest of command
     * line into it as 1. NumOfprm, 2. pointer to block of pointers.
     */
    for (i = strlen(CtrlStr) - 1; i > 0 && !ISSPACE(CtrlStr[i]); i--) ;
    if (!ISCTRLCHAR(CtrlStr[i + 2])) {
        GASetParamCount(CtrlStr, i, ParamCount); /* Point in correct param. */
        *(int *)Parameters[(*ParamCount)++] = argv_end - *argv;
	*(char ***)Parameters[(*ParamCount)++] = *argv;
    }

    i = 0;
    while (++i < (int)strlen(CtrlStrCopy))
        if ((CtrlStrCopy[i] == '-') && (CtrlStrCopy[i - 1] == '!')) {
            GAErrorToken = LocalToken;
            LocalToken[1] = CtrlStrCopy[i - 2];    /* Set the correct flag. */
            return CMD_ERR_AllSatis;
        }

    return ARG_OK;
}

/***************************************************************************
 Routine to update the parameters according to the given Option:
 **************************************************************************/
static bool
GAUpdateParameters(void *Parameters[],
                   int *ParamCount,
                   char *Option,
                   char *CtrlStrCopy,
                   char *CtrlStr,
                   char **argv_end,
                   char ***argv) {

    int i;
    bool BooleanTrue = Option[2] != '-';

    if (Option[0] != '-') {
        GAErrorToken = Option;
        return CMD_ERR_NotAnOpt;
    }
    i = 0;    /* Scan the CtrlStrCopy for that option: */
    while (i + 2 < (int)strlen(CtrlStrCopy)) {
        if ((CtrlStrCopy[i] == Option[1]) && (ISCTRLCHAR(CtrlStrCopy[i + 1]))
            && (CtrlStrCopy[i + 2] == '-')) {
            /* We found that option! */
            break;
        }
        i++;
    }
    if (i + 2 >= (int)strlen(CtrlStrCopy)) {
        GAErrorToken = Option;
        return CMD_ERR_NoSuchOpt;
    }

    /* If we are here, then we found that option in CtrlStr - Strip it off: */
    CtrlStrCopy[i] = CtrlStrCopy[i + 1] = CtrlStrCopy[i + 2] = (char)' ';
    GASetParamCount(CtrlStr, i, ParamCount); /* Set it to point in
                                                correct prm. */
    i += 3;
    /* Set boolean flag for that option. */
    *(bool *)Parameters[(*ParamCount)++] = BooleanTrue;
    if (ISSPACE(CtrlStrCopy[i]))
        return ARG_OK;    /* Only a boolean flag is needed. */

    /* Skip the text between the boolean option and data follows: */
    while (!ISCTRLCHAR(CtrlStrCopy[i]))
        i++;
    /* Get the parameters and return the appropriete return code: */
    return GAGetParmeters(Parameters, ParamCount, &CtrlStrCopy[i],
                          Option, argv_end, argv);
}

/***************************************************************************
 Routine to get parameters according to the CtrlStr given from argv/argc
***************************************************************************/
static int
GAGetParmeters(void *Parameters[],
               int *ParamCount,
               char *CtrlStrCopy,
               char *Option,
               char **argv_end,
               char ***argv) {

    int i = 0, ScanRes;

    while (!(ISSPACE(CtrlStrCopy[i]))) {
        switch (CtrlStrCopy[i + 1]) {
          case 'd':    /* Get signed integers. */
              ScanRes = sscanf(*((*argv)++), "%d",
                               (int *)Parameters[(*ParamCount)++]);
              break;
          case 'u':    /* Get unsigned integers. */
              ScanRes = sscanf(*((*argv)++), "%u",
                               (unsigned *)Parameters[(*ParamCount)++]);
              break;
          case 'x':    /* Get hex integers. */
              ScanRes = sscanf(*((*argv)++), "%x",
                               (unsigned int *)Parameters[(*ParamCount)++]);
              break;
          case 'o':    /* Get octal integers. */
              ScanRes = sscanf(*((*argv)++), "%o",
                               (unsigned int *)Parameters[(*ParamCount)++]);
              break;
          case 'D':    /* Get signed long integers. */
              ScanRes = sscanf(*((*argv)++), "%ld",
                               (long *)Parameters[(*ParamCount)++]);
              break;
          case 'U':    /* Get unsigned long integers. */
              ScanRes = sscanf(*((*argv)++), "%lu",
                               (unsigned long *)Parameters[(*ParamCount)++]);
              break;
          case 'X':    /* Get hex long integers. */
              ScanRes = sscanf(*((*argv)++), "%lx",
                               (unsigned long *)Parameters[(*ParamCount)++]);
              break;
          case 'O':    /* Get octal long integers. */
              ScanRes = sscanf(*((*argv)++), "%lo",
                               (unsigned long *)Parameters[(*ParamCount)++]);
              break;
          case 'f':    /* Get float number. */
              ScanRes = sscanf(*((*argv)++), "%f",
                               (float *)Parameters[(*ParamCount)++]);
	      break;
          case 'F':    /* Get double float number. */
              ScanRes = sscanf(*((*argv)++), "%lf",
                               (double *)Parameters[(*ParamCount)++]);
              break;
          case 's':    /* It as a string. */
              ScanRes = 1;    /* Allways O.K. */
	      *(char **)Parameters[(*ParamCount)++] = *((*argv)++);
              break;
          case '*':    /* Get few parameters into one: */
              ScanRes = GAGetMultiParmeters(Parameters, ParamCount,
                                            &CtrlStrCopy[i], argv_end, argv);
              if ((ScanRes == 0) && (CtrlStrCopy[i] == '!')) {
                  GAErrorToken = Option;
                  return CMD_ERR_WildEmpty;
              }
              break;
          default:
              ScanRes = 0;    /* Make optimizer warning silent. */
        }
        /* If reading fails and this number is a must (!) then error: */
        if ((ScanRes == 0) && (CtrlStrCopy[i] == '!')) {
            GAErrorToken = Option;
            return CMD_ERR_NumRead;
        }
        if (CtrlStrCopy[i + 1] != '*') {
            i += 2;    /* Skip to next parameter (if any). */
        } else
            i += 3;    /* Skip the '*' also! */
    }

    return ARG_OK;
}

/***************************************************************************
 Routine to get a few parameters into one pointer such that the returned
 pointer actually points on a block of pointers to the parameters...
 For example *F means a pointer to pointers on floats.
 Returns number of parameters actually read.
 This routine assumes that all pointers (on any kind of scalar) has the
 same size (and the union below is totally ovelapped bteween dif. arrays)
***************************************************************************/
static int
GAGetMultiParmeters(void *Parameters[],
                    int *ParamCount,
                    char *CtrlStrCopy,
                    char **argv_end,
                    char ***argv) {

    int i = 0, ScanRes, NumOfPrm = 0;
    void **Pmain, **Ptemp;
    union TmpArray {    /* Save here the temporary data before copying it to */
        void *VoidArray[MAX_PARAM];    /* the returned pointer block. */
        int *IntArray[MAX_PARAM];
        long *LngArray[MAX_PARAM];
        float *FltArray[MAX_PARAM];
        double *DblArray[MAX_PARAM];
        char *ChrArray[MAX_PARAM];
    } TmpArray;

    do {
        switch (CtrlStrCopy[2]) { /* CtrlStr == '!*?' or '%*?' where ? is. */
          case 'd':    /* Format to read the parameters: */
              TmpArray.IntArray[NumOfPrm] = xmalloc(sizeof(int));
              ScanRes = sscanf(*((*argv)++), "%d",
                               (int *)TmpArray.IntArray[NumOfPrm++]);
              break;
          case 'u':
              TmpArray.IntArray[NumOfPrm] = xmalloc(sizeof(int));
              ScanRes = sscanf(*((*argv)++), "%u",
                               (unsigned int *)TmpArray.IntArray[NumOfPrm++]);
              break;
          case 'o':
              TmpArray.IntArray[NumOfPrm] = xmalloc(sizeof(int));
              ScanRes = sscanf(*((*argv)++), "%o",
                               (unsigned int *)TmpArray.IntArray[NumOfPrm++]);
              break;
          case 'x':
              TmpArray.IntArray[NumOfPrm] = xmalloc(sizeof(int));
              ScanRes = sscanf(*((*argv)++), "%x",
                               (unsigned int *)TmpArray.IntArray[NumOfPrm++]);
              break;
          case 'D':
              TmpArray.LngArray[NumOfPrm] = xmalloc(sizeof(long));
              ScanRes = sscanf(*((*argv)++), "%ld",
                               (long *)TmpArray.IntArray[NumOfPrm++]);
              break;
          case 'U':
              TmpArray.LngArray[NumOfPrm] = xmalloc(sizeof(long));
              ScanRes = sscanf(*((*argv)++), "%lu",
                               (unsigned long *)TmpArray.
                               IntArray[NumOfPrm++]);
              break;
          case 'O':
              TmpArray.LngArray[NumOfPrm] = xmalloc(sizeof(long));
              ScanRes = sscanf(*((*argv)++), "%lo",
                               (unsigned long *)TmpArray.
                               IntArray[NumOfPrm++]);
              break;
          case 'X':
              TmpArray.LngArray[NumOfPrm] = xmalloc(sizeof(long));
              ScanRes = sscanf(*((*argv)++), "%lx",
                               (unsigned long *)TmpArray.
                               IntArray[NumOfPrm++]);
              break;
          case 'f':
              TmpArray.FltArray[NumOfPrm] = xmalloc(sizeof(float));
              ScanRes = sscanf(*((*argv)++), "%f",
			       // cppcheck-suppress invalidPointerCast
                               (float *)TmpArray.LngArray[NumOfPrm++]);
              break;
          case 'F':
              TmpArray.DblArray[NumOfPrm] = xmalloc(sizeof(double));
              ScanRes = sscanf(*((*argv)++), "%lf",
			       // cppcheck-suppress invalidPointerCast
                               (double *)TmpArray.LngArray[NumOfPrm++]);
              break;
          case 's':
              while ((*argv < argv_end) && ((**argv)[0] != '-')) {
                  TmpArray.ChrArray[NumOfPrm++] = *((*argv)++);
              }
              ScanRes = 0;    /* Force quit from do - loop. */
              NumOfPrm++;    /* Updated again immediately after loop! */
              (*argv)++;    /* "" */
              break;
          default:
              ScanRes = 0;    /* Make optimizer warning silent. */
        }
    }
    while (ScanRes == 1);    /* Exactly one parameter was read. */
    (*argv)--;
    NumOfPrm--;

    /* Now allocate the block with the exact size, and set it: */
    Ptemp = Pmain = xmalloc((unsigned)(NumOfPrm + 1) * sizeof(void *));
    /* And here we use the assumption that all pointers are the same: */
    for (i = 0; i < NumOfPrm; i++)
        *Ptemp++ = TmpArray.VoidArray[i];
    *Ptemp = NULL;    /* Close the block with NULL pointer. */

    /* That it save the number of parameters read as first parameter to
     * return and the pointer to the block as second, and return: */
    *(int *)Parameters[(*ParamCount)++] = NumOfPrm;
    *(void ***)Parameters[(*ParamCount)++] = Pmain;
    /* free(Pmain); -- can not free here as caller needs to access memory */
    return NumOfPrm;
}

/***************************************************************************
 Routine to scan the CtrlStr, up to Max and count the number of parameters
 to that point:
 1. Each option is counted as one parameter - boolean variable (int)
 2. Within an option, each %? or !? is counted once - pointer to something
 3. Within an option, %*? or !*? is counted twice - one for item count
 and one for pointer to block pointers.
 Note ALL variables are passed by address and so of fixed size (address).
***************************************************************************/
static void
GASetParamCount(char *CtrlStr,
                int Max,
                int *ParamCount) {
    int i;

    *ParamCount = 0;
    for (i = 0; i < Max; i++)
        if (ISCTRLCHAR(CtrlStr[i])) {
            if (CtrlStr[i + 1] == '*')
                *ParamCount += 2;
            else
                (*ParamCount)++;
        }
}

/***************************************************************************
 Routine to check if more option (i.e. first char == '-') exists in the
 given list argc, argv:
***************************************************************************/
static bool
GAOptionExists(char **argv_end,
               char **argv) {

    while (argv < argv_end)
        if ((*argv++)[0] == '-')
            return true;
    return false;
}

/***************************************************************************
 Routine to print some error messages, for this module:
***************************************************************************/
void
GAPrintErrMsg(int Error) {

    fprintf(stderr, "Error in command line parsing - ");
    switch (Error) {
      case 0:;
          fprintf(stderr, "Undefined error");
          break;
      case CMD_ERR_NotAnOpt:
          fprintf(stderr, "None option Found");
          break;
      case CMD_ERR_NoSuchOpt:
          fprintf(stderr, "Undefined option Found");
          break;
      case CMD_ERR_WildEmpty:
          fprintf(stderr, "Empty input for '!*?' seq.");
          break;
      case CMD_ERR_NumRead:
          fprintf(stderr, "Failed on reading number");
          break;
      case CMD_ERR_AllSatis:
          fprintf(stderr, "Fail to satisfy");
          break;
    }
    fprintf(stderr, " - '%s'.\n", GAErrorToken);
}

/***************************************************************************
 Routine to print correct format of command line allowed:
***************************************************************************/
void
GAPrintHowTo(char *CtrlStr) {

    int i = 0;
    bool SpaceFlag;

    fprintf(stderr, "Usage: ");
    /* Print program name - first word in ctrl. str. (optional): */
    while (!(ISSPACE(CtrlStr[i])) && (!ISCTRLCHAR(CtrlStr[i + 1])))
        fprintf(stderr, "%c", CtrlStr[i++]);

    while (i < (int)strlen(CtrlStr)) {
	// cppcheck-suppress arrayIndexThenCheck
        while ((ISSPACE(CtrlStr[i])) && (i < (int)strlen(CtrlStr)))
            i++;
        switch (CtrlStr[i + 1]) {
          case '%':
              fprintf(stderr, " [-%c", CtrlStr[i++]);
              i += 2;    /* Skip the '%-' or '!- after the char! */
              SpaceFlag = true;
              while (!ISCTRLCHAR(CtrlStr[i]) && (i < (int)strlen(CtrlStr)) &&
                     (!ISSPACE(CtrlStr[i])))
                  if (SpaceFlag) {
                      if (CtrlStr[i++] == SPACE_CHAR)
                          fprintf(stderr, " ");
                      else
                          fprintf(stderr, " %c", CtrlStr[i - 1]);
                      SpaceFlag = false;
                  } else if (CtrlStr[i++] == SPACE_CHAR)
                      fprintf(stderr, " ");
                  else
                      fprintf(stderr, "%c", CtrlStr[i - 1]);
              while (!ISSPACE(CtrlStr[i]) && (i < (int)strlen(CtrlStr))) {
                  if (CtrlStr[i] == '*')
                      fprintf(stderr, "...");
                  i++;    /* Skip the rest of it. */
              }
              fprintf(stderr, "]");
              break;
          case '!':
              fprintf(stderr, " -%c", CtrlStr[i++]);
              i += 2;    /* Skip the '%-' or '!- after the char! */
              SpaceFlag = true;
              while (!ISCTRLCHAR(CtrlStr[i]) && (i < (int)strlen(CtrlStr)) &&
                     (!ISSPACE(CtrlStr[i])))
                  if (SpaceFlag) {
                      if (CtrlStr[i++] == SPACE_CHAR)
                          fprintf(stderr, " ");
                      else
                          fprintf(stderr, " %c", CtrlStr[i - 1]);
                      SpaceFlag = false;
                  } else if (CtrlStr[i++] == SPACE_CHAR)
                      fprintf(stderr, " ");
                  else
                      fprintf(stderr, "%c", CtrlStr[i - 1]);
              while (!ISSPACE(CtrlStr[i]) && (i < (int)strlen(CtrlStr))) {
                  if (CtrlStr[i] == '*')
                      fprintf(stderr, "...");
                  i++;    /* Skip the rest of it. */
              }
              break;
          default:    /* Not checked, but must be last one! */
              fprintf(stderr, " ");
              while (!ISSPACE(CtrlStr[i]) && (i < (int)strlen(CtrlStr)) &&
                     !ISCTRLCHAR(CtrlStr[i]))
                  fprintf(stderr, "%c", CtrlStr[i++]);
              fprintf(stderr, "\n");
              return;
        }
    }
    fprintf(stderr, "\n");
}

/* end */
