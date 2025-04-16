/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/*
** A program for performance testing.
**
** The available command-line options are described below:
*/
static const char zHelp[] =
  "Usage: %s [--options] DATABASE\n"
  "Options:\n"
  "  --autovacuum        Enable AUTOVACUUM mode\n"
  "  --cachesize N       Set the cache size to N\n" 
  "  --exclusive         Enable locking_mode=EXCLUSIVE\n"
  "  --explain           Like --sqlonly but with added EXPLAIN keywords\n"
  "  --heap SZ MIN       Memory allocator uses SZ bytes & min allocation MIN\n"
  "  --incrvacuum        Enable incremenatal vacuum mode\n"
  "  --journalmode M     Set the journal_mode to MODE\n"
  "  --key KEY           Set the encryption key to KEY\n"
  "  --lookaside N SZ    Configure lookaside for N slots of SZ bytes each\n"
  "  --nosync            Set PRAGMA synchronous=OFF\n"
  "  --notnull           Add NOT NULL constraints to table columns\n"
  "  --pagesize N        Set the page size to N\n"
  "  --pcache N SZ       Configure N pages of pagecache each of size SZ bytes\n"
  "  --primarykey        Use PRIMARY KEY instead of UNIQUE where appropriate\n"
  "  --reprepare         Reprepare each statement upon every invocation\n"
  "  --scratch N SZ      Configure scratch memory for N slots of SZ bytes each\n"
  "  --sqlonly           No-op.  Only show the SQL that would have been run.\n"
  "  --size N            Relative test size.  Default=100\n"
  "  --stats             Show statistics at the end\n"
  "  --testset T         Run test-set T\n"
  "  --trace             Turn on SQL tracing\n"
  "  --utf16be           Set text encoding to UTF-16BE\n"
  "  --utf16le           Set text encoding to UTF-16LE\n"
  "  --verify            Run additional verification steps.\n"
  "  --without-rowid     Use WITHOUT ROWID where appropriate\n"
;


#include "sqlite3.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* All global state is held in this structure */
static struct Global {
  sqlite3 *db;               /* The open database connection */
  sqlite3_stmt *pStmt;       /* Current SQL statement */
  sqlite3_int64 iStart;      /* Start-time for the current test */
  sqlite3_int64 iTotal;      /* Total time */
  int bWithoutRowid;         /* True for --without-rowid */
  int bReprepare;            /* True to reprepare the SQL on each rerun */
  int bSqlOnly;              /* True to print the SQL once only */
  int bExplain;              /* Print SQL with EXPLAIN prefix */
  int bVerify;               /* Try to verify that results are correct */
  int szTest;                /* Scale factor for test iterations */
  const char *zWR;           /* Might be WITHOUT ROWID */
  const char *zNN;           /* Might be NOT NULL */
  const char *zPK;           /* Might be UNIQUE or PRIMARY KEY */
  unsigned int x, y;         /* Pseudo-random number generator state */
  int nResult;               /* Size of the current result */
  char zResult[3000];        /* Text of the current result */
} g;


/* Print an error message and exit */
static void fatal_error(const char *zMsg, ...){
  va_list ap;
  va_start(ap, zMsg);
  vfprintf(stderr, zMsg, ap);
  va_end(ap);
  exit(1);
}

/*
** Return the value of a hexadecimal digit.  Return -1 if the input
** is not a hex digit.
*/
static int hexDigitValue(char c){
  if( c>='0' && c<='9' ) return c - '0';
  if( c>='a' && c<='f' ) return c - 'a' + 10;
  if( c>='A' && c<='F' ) return c - 'A' + 10;
  return -1;
}

/* Provide an alternative to sqlite3_stricmp() in older versions of
** SQLite */
#if SQLITE_VERSION_NUMBER<3007011
# define sqlite3_stricmp strcmp
#endif

/*
** Interpret zArg as an integer value, possibly with suffixes.
*/
static int integerValue(const char *zArg){
  sqlite3_int64 v = 0;
  static const struct { char *zSuffix; int iMult; } aMult[] = {
    { "KiB", 1024 },
    { "MiB", 1024*1024 },
    { "GiB", 1024*1024*1024 },
    { "KB",  1000 },
    { "MB",  1000000 },
    { "GB",  1000000000 },
    { "K",   1000 },
    { "M",   1000000 },
    { "G",   1000000000 },
  };
  int i;
  int isNeg = 0;
  if( zArg[0]=='-' ){
    isNeg = 1;
    zArg++;
  }else if( zArg[0]=='+' ){
    zArg++;
  }
  if( zArg[0]=='0' && zArg[1]=='x' ){
    int x;
    zArg += 2;
    while( (x = hexDigitValue(zArg[0]))>=0 ){
      v = (v<<4) + x;
      zArg++;
    }
  }else{
    while( isdigit(zArg[0]) ){
      v = v*10 + zArg[0] - '0';
      zArg++;
    }
  }
  for(i=0; i<sizeof(aMult)/sizeof(aMult[0]); i++){
    if( sqlite3_stricmp(aMult[i].zSuffix, zArg)==0 ){
      v *= aMult[i].iMult;
      break;
    }
  }
  if( v>0x7fffffff ) fatal_error("parameter too large - max 2147483648");
  return (int)(isNeg? -v : v);
}

/* Return the current wall-clock time, in milliseconds */
sqlite3_int64 speedtest1_timestamp(void){
  static sqlite3_vfs *clockVfs = 0;
  sqlite3_int64 t;
  if( clockVfs==0 ) clockVfs = sqlite3_vfs_find(0);
#if SQLITE_VERSION_NUMBER>=3007000
  if( clockVfs->iVersion>=2 && clockVfs->xCurrentTimeInt64!=0 ){
    clockVfs->xCurrentTimeInt64(clockVfs, &t);
  }else
#endif
  {
    double r;
    clockVfs->xCurrentTime(clockVfs, &r);
    t = (sqlite3_int64)(r*86400000.0);
  }
  return t;
}

/* Return a pseudo-random unsigned integer */
unsigned int speedtest1_random(void){
  g.x = (g.x>>1) ^ ((1+~(g.x&1)) & 0xd0000001);
  g.y = g.y*1103515245 + 12345;
  return g.x ^ g.y;
}

/* Map the value in within the range of 1...limit into another
** number in a way that is chatic and invertable.
*/
unsigned swizzle(unsigned in, unsigned limit){
  unsigned out = 0;
  while( limit ){
    out = (out<<1) | (in&1);
    in >>= 1;
    limit >>= 1;
  }
  return out;
}

/* Round up a number so that it is a power of two minus one
*/
unsigned roundup_allones(unsigned limit){
  unsigned m = 1;
  while( m<limit ) m = (m<<1)+1;
  return m;
}

/* The speedtest1_numbername procedure below converts its argment (an integer)
** into a string which is the English-language name for that number.
** The returned string should be freed with sqlite3_free().
**
** Example:
**
**     speedtest1_numbername(123)   ->  "one hundred twenty three"
*/
int speedtest1_numbername(unsigned int n, char *zOut, int nOut){
  static const char *ones[] = {  "zero", "one", "two", "three", "four", "five", 
                  "six", "seven", "eight", "nine", "ten", "eleven", "twelve", 
                  "thirteen", "fourteen", "fifteen", "sixteen", "seventeen",
                  "eighteen", "nineteen" };
  static const char *tens[] = { "", "ten", "twenty", "thirty", "forty",
                 "fifty", "sixty", "seventy", "eighty", "ninety" };
  int i = 0;

  if( n>=1000000000 ){
    i += speedtest1_numbername(n/1000000000, zOut+i, nOut-i);
    sqlite3_snprintf(nOut-i, zOut+i, " billion");
    i += (int)strlen(zOut+i);
    n = n % 1000000000;
  }
  if( n>=1000000 ){
    if( i && i<nOut-1 ) zOut[i++] = ' ';
    i += speedtest1_numbername(n/1000000, zOut+i, nOut-i);
    sqlite3_snprintf(nOut-i, zOut+i, " million");
    i += (int)strlen(zOut+i);
    n = n % 1000000;
  }
  if( n>=1000 ){
    if( i && i<nOut-1 ) zOut[i++] = ' ';
    i += speedtest1_numbername(n/1000, zOut+i, nOut-i);
    sqlite3_snprintf(nOut-i, zOut+i, " thousand");
    i += (int)strlen(zOut+i);
    n = n % 1000;
  }
  if( n>=100 ){
    if( i && i<nOut-1 ) zOut[i++] = ' ';
    sqlite3_snprintf(nOut-i, zOut+i, "%s hundred", ones[n/100]);
    i += (int)strlen(zOut+i);
    n = n % 100;
  }
  if( n>=20 ){
    if( i && i<nOut-1 ) zOut[i++] = ' ';
    sqlite3_snprintf(nOut-i, zOut+i, "%s", tens[n/10]);
    i += (int)strlen(zOut+i);
    n = n % 10;
  }
  if( n>0 ){
    if( i && i<nOut-1 ) zOut[i++] = ' ';
    sqlite3_snprintf(nOut-i, zOut+i, "%s", ones[n]);
    i += (int)strlen(zOut+i);
  }
  if( i==0 ){
    sqlite3_snprintf(nOut-i, zOut+i, "zero");
    i += (int)strlen(zOut+i);
  }
  return i;
}


/* Start a new test case */
#define NAMEWIDTH 60
static const char zDots[] =
  ".......................................................................";
void speedtest1_begin_test(int iTestNum, const char *zTestName, ...){
  int n = (int)strlen(zTestName);
  char *zName;
  va_list ap;
  va_start(ap, zTestName);
  zName = sqlite3_vmprintf(zTestName, ap);
  va_end(ap);
  n = (int)strlen(zName);
  if( n>NAMEWIDTH ){
    zName[NAMEWIDTH] = 0;
    n = NAMEWIDTH;
  }
  if( g.bSqlOnly ){
    printf("/* %4d - %s%.*s */\n", iTestNum, zName, NAMEWIDTH-n, zDots);
  }else{
    printf("%4d - %s%.*s ", iTestNum, zName, NAMEWIDTH-n, zDots);
    fflush(stdout);
  }
  sqlite3_free(zName);
  g.nResult = 0;
  g.iStart = speedtest1_timestamp();
  g.x = 0xad131d0b;
  g.y = 0x44f9eac8;
}

/* Complete a test case */
void speedtest1_end_test(void){
  sqlite3_int64 iElapseTime = speedtest1_timestamp() - g.iStart;
  if( !g.bSqlOnly ){
    g.iTotal += iElapseTime;
    printf("%4d.%03ds\n", (int)(iElapseTime/1000), (int)(iElapseTime%1000));
  }
  if( g.pStmt ){
    sqlite3_finalize(g.pStmt);
    g.pStmt = 0;
  }
}

/* Report end of testing */
void speedtest1_final(void){
  if( !g.bSqlOnly ){
    printf("       TOTAL%.*s %4d.%03ds\n", NAMEWIDTH-5, zDots,
           (int)(g.iTotal/1000), (int)(g.iTotal%1000));
  }
}

/* Print an SQL statement to standard output */
static void printSql(const char *zSql){
  int n = (int)strlen(zSql);
  while( n>0 && (zSql[n-1]==';' || isspace(zSql[n-1])) ){ n--; }
  if( g.bExplain ) printf("EXPLAIN ");
  printf("%.*s;\n", n, zSql);
  if( g.bExplain
#if SQLITE_VERSION_NUMBER>=3007010 
   && ( sqlite3_strglob("CREATE *", zSql)==0
     || sqlite3_strglob("DROP *", zSql)==0
     || sqlite3_strglob("ALTER *", zSql)==0
      )
#endif
  ){
    printf("%.*s;\n", n, zSql);
  }
}

/* Run SQL */
void speedtest1_exec(const char *zFormat, ...){
  va_list ap;
  char *zSql;
  va_start(ap, zFormat);
  zSql = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  if( g.bSqlOnly ){
    printSql(zSql);
  }else{
    char *zErrMsg = 0;
    int rc = sqlite3_exec(g.db, zSql, 0, 0, &zErrMsg);
    if( zErrMsg ) fatal_error("SQL error: %s\n%s\n", zErrMsg, zSql);
    if( rc!=SQLITE_OK ) fatal_error("exec error: %s\n", sqlite3_errmsg(g.db));
  }
  sqlite3_free(zSql);
}

/* Prepare an SQL statement */
void speedtest1_prepare(const char *zFormat, ...){
  va_list ap;
  char *zSql;
  va_start(ap, zFormat);
  zSql = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  if( g.bSqlOnly ){
    printSql(zSql);
  }else{
    int rc;
    if( g.pStmt ) sqlite3_finalize(g.pStmt);
    rc = sqlite3_prepare_v2(g.db, zSql, -1, &g.pStmt, 0);
    if( rc ){
      fatal_error("SQL error: %s\n", sqlite3_errmsg(g.db));
    }
  }
  sqlite3_free(zSql);
}

/* Run an SQL statement previously prepared */
void speedtest1_execute(void){
  int i, n, len;
  if( g.bSqlOnly ) return;
  assert( g.pStmt );
  g.nResult = 0;
  while( sqlite3_step(g.pStmt)==SQLITE_ROW ){
    n = sqlite3_column_count(g.pStmt);
    for(i=0; i<n; i++){
      const char *z = (const char*)sqlite3_column_text(g.pStmt, i);
      if( z==0 ) z = "nil";
      len = (int)strlen(z);
      if( g.nResult+len<sizeof(g.zResult)-2 ){
        if( g.nResult>0 ) g.zResult[g.nResult++] = ' ';
        memcpy(g.zResult + g.nResult, z, len+1);
        g.nResult += len;
      }
    }
  }
  if( g.bReprepare ){
    sqlite3_stmt *pNew;
    sqlite3_prepare_v2(g.db, sqlite3_sql(g.pStmt), -1, &pNew, 0);
    sqlite3_finalize(g.pStmt);
    g.pStmt = pNew;
  }else{
    sqlite3_reset(g.pStmt);
  }
}

/* The sqlite3_trace() callback function */
static void traceCallback(void *NotUsed, const char *zSql){
  int n = (int)strlen(zSql);
  while( n>0 && (zSql[n-1]==';' || isspace(zSql[n-1])) ) n--;
  fprintf(stderr,"%.*s;\n", n, zSql);
}

/* Substitute random() function that gives the same random
** sequence on each run, for repeatability. */
static void randomFunc1(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **NotUsed2
){
  sqlite3_result_int64(context, (sqlite3_int64)speedtest1_random());
}

/* Estimate the square root of an integer */
static int est_square_root(int x){
  int y0 = x/2;
  int y1;
  int n;
  for(n=0; y0>0 && n<10; n++){
    y1 = (y0 + x/y0)/2;
    if( y1==y0 ) break;
    y0 = y1;
  }
  return y0;
}

/*
** The main and default testset
*/
void testset_main(void){
  int i;                        /* Loop counter */
  int n;                        /* iteration count */
  int sz;                       /* Size of the tables */
  int maxb;                     /* Maximum swizzled value */
  unsigned x1, x2;              /* Parameters */
  int len;                      /* Length of the zNum[] string */
  char zNum[2000];              /* A number name */

  sz = n = g.szTest*500;
  maxb = roundup_allones(sz);
  speedtest1_begin_test(100, "%d INSERTs into table with no index", n);
  speedtest1_exec("BEGIN");
  speedtest1_exec("CREATE TABLE t1(a INTEGER %s, b INTEGER %s, c TEXT %s);",
                  g.zNN, g.zNN, g.zNN);
  speedtest1_prepare("INSERT INTO t1 VALUES(?1,?2,?3); --  %d times", n);
  for(i=1; i<=n; i++){
    x1 = swizzle(i,maxb);
    speedtest1_numbername(x1, zNum, sizeof(zNum));
    sqlite3_bind_int64(g.pStmt, 1, (sqlite3_int64)x1);
    sqlite3_bind_int(g.pStmt, 2, i);
    sqlite3_bind_text(g.pStmt, 3, zNum, -1, SQLITE_STATIC);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = sz;
  speedtest1_begin_test(110, "%d ordered INSERTS with one index/PK", n);
  speedtest1_exec("BEGIN");
  speedtest1_exec("CREATE TABLE t2(a INTEGER %s %s, b INTEGER %s, c TEXT %s) %s",
                   g.zNN, g.zPK, g.zNN, g.zNN, g.zWR);
  speedtest1_prepare("INSERT INTO t2 VALUES(?1,?2,?3); -- %d times", n);
  for(i=1; i<=n; i++){
    x1 = swizzle(i,maxb);
    speedtest1_numbername(x1, zNum, sizeof(zNum));
    sqlite3_bind_int(g.pStmt, 1, i);
    sqlite3_bind_int64(g.pStmt, 2, (sqlite3_int64)x1);
    sqlite3_bind_text(g.pStmt, 3, zNum, -1, SQLITE_STATIC);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = sz;
  speedtest1_begin_test(120, "%d unordered INSERTS with one index/PK", n);
  speedtest1_exec("BEGIN");
  speedtest1_exec("CREATE TABLE t3(a INTEGER %s %s, b INTEGER %s, c TEXT %s) %s",
                   g.zNN, g.zPK, g.zNN, g.zNN, g.zWR);
  speedtest1_prepare("INSERT INTO t3 VALUES(?1,?2,?3); -- %d times", n);
  for(i=1; i<=n; i++){
    x1 = swizzle(i,maxb);
    speedtest1_numbername(x1, zNum, sizeof(zNum));
    sqlite3_bind_int(g.pStmt, 2, i);
    sqlite3_bind_int64(g.pStmt, 1, (sqlite3_int64)x1);
    sqlite3_bind_text(g.pStmt, 3, zNum, -1, SQLITE_STATIC);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = 25;
  speedtest1_begin_test(130, "%d SELECTS, numeric BETWEEN, unindexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT count(*), avg(b), sum(length(c)) FROM t1\n"
    " WHERE b BETWEEN ?1 AND ?2; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb;
    x2 = speedtest1_random()%10 + sz/5000 + x1;
    sqlite3_bind_int(g.pStmt, 1, x1);
    sqlite3_bind_int(g.pStmt, 2, x2);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = 10;
  speedtest1_begin_test(140, "%d SELECTS, LIKE, unindexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT count(*), avg(b), sum(length(c)) FROM t1\n"
    " WHERE c LIKE ?1; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb;
    zNum[0] = '%';
    len = speedtest1_numbername(i, zNum+1, sizeof(zNum)-2);
    zNum[len] = '%';
    zNum[len+1] = 0;
    sqlite3_bind_text(g.pStmt, 1, zNum, len, SQLITE_STATIC);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = 10;
  speedtest1_begin_test(142, "%d SELECTS w/ORDER BY, unindexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT a, b, c FROM t1 WHERE c LIKE ?1\n"
    " ORDER BY a; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb;
    zNum[0] = '%';
    len = speedtest1_numbername(i, zNum+1, sizeof(zNum)-2);
    zNum[len] = '%';
    zNum[len+1] = 0;
    sqlite3_bind_text(g.pStmt, 1, zNum, len, SQLITE_STATIC);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();

  n = 10; //g.szTest/5;
  speedtest1_begin_test(145, "%d SELECTS w/ORDER BY and LIMIT, unindexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT a, b, c FROM t1 WHERE c LIKE ?1\n"
    " ORDER BY a LIMIT 10; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb;
    zNum[0] = '%';
    len = speedtest1_numbername(i, zNum+1, sizeof(zNum)-2);
    zNum[len] = '%';
    zNum[len+1] = 0;
    sqlite3_bind_text(g.pStmt, 1, zNum, len, SQLITE_STATIC);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  speedtest1_begin_test(150, "CREATE INDEX five times");
  speedtest1_exec("BEGIN;");
  speedtest1_exec("CREATE UNIQUE INDEX t1b ON t1(b);");
  speedtest1_exec("CREATE INDEX t1c ON t1(c);");
  speedtest1_exec("CREATE UNIQUE INDEX t2b ON t2(b);");
  speedtest1_exec("CREATE INDEX t2c ON t2(c DESC);");
  speedtest1_exec("CREATE INDEX t3bc ON t3(b,c);");
  speedtest1_exec("COMMIT;");
  speedtest1_end_test();


  n = sz/5;
  speedtest1_begin_test(160, "%d SELECTS, numeric BETWEEN, indexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT count(*), avg(b), sum(length(c)) FROM t1\n"
    " WHERE b BETWEEN ?1 AND ?2; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb;
    x2 = speedtest1_random()%10 + sz/5000 + x1;
    sqlite3_bind_int(g.pStmt, 1, x1);
    sqlite3_bind_int(g.pStmt, 2, x2);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = sz/5;
  speedtest1_begin_test(161, "%d SELECTS, numeric BETWEEN, PK", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT count(*), avg(b), sum(length(c)) FROM t2\n"
    " WHERE a BETWEEN ?1 AND ?2; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb;
    x2 = speedtest1_random()%10 + sz/5000 + x1;
    sqlite3_bind_int(g.pStmt, 1, x1);
    sqlite3_bind_int(g.pStmt, 2, x2);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = sz/5;
  speedtest1_begin_test(170, "%d SELECTS, text BETWEEN, indexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT count(*), avg(b), sum(length(c)) FROM t1\n"
    " WHERE c BETWEEN ?1 AND (?1||'~'); -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = swizzle(i, maxb);
    len = speedtest1_numbername(x1, zNum, sizeof(zNum)-1);
    sqlite3_bind_text(g.pStmt, 1, zNum, len, SQLITE_STATIC);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();

  n = sz;
  speedtest1_begin_test(180, "%d INSERTS with three indexes", n);
  speedtest1_exec("BEGIN");
  speedtest1_exec(
    "CREATE TABLE t4(\n"
    "  a INTEGER %s %s,\n"
    "  b INTEGER %s,\n"
    "  c TEXT %s\n"
    ") %s",
    g.zNN, g.zPK, g.zNN, g.zNN, g.zWR);
  speedtest1_exec("CREATE INDEX t4b ON t4(b)");
  speedtest1_exec("CREATE INDEX t4c ON t4(c)");
  speedtest1_exec("INSERT INTO t4 SELECT * FROM t1");
  speedtest1_exec("COMMIT");
  speedtest1_end_test();

  n = sz;
  speedtest1_begin_test(190, "DELETE and REFILL one table", n);
  speedtest1_exec("DELETE FROM t2;");
  speedtest1_exec("INSERT INTO t2 SELECT * FROM t1;");
  speedtest1_end_test();

//  #include <emscripten.h>
  speedtest1_begin_test(200, "VACUUM");
//  EM_ASM( alert('pre') );
  speedtest1_exec("VACUUM");
//  EM_ASM( alert('post') );
  speedtest1_end_test();


  speedtest1_begin_test(210, "ALTER TABLE ADD COLUMN, and query");
  speedtest1_exec("ALTER TABLE t2 ADD COLUMN d DEFAULT 123");
  speedtest1_exec("SELECT sum(d) FROM t2");
  speedtest1_end_test();


  n = sz/5;
  speedtest1_begin_test(230, "%d UPDATES, numeric BETWEEN, indexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "UPDATE t2 SET d=b*2 WHERE b BETWEEN ?1 AND ?2; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb;
    x2 = speedtest1_random()%10 + sz/5000 + x1;
    sqlite3_bind_int(g.pStmt, 1, x1);
    sqlite3_bind_int(g.pStmt, 2, x2);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = sz;
  speedtest1_begin_test(240, "%d UPDATES of individual rows", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "UPDATE t2 SET d=b*3 WHERE a=?1; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%sz + 1;
    sqlite3_bind_int(g.pStmt, 1, x1);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();

  speedtest1_begin_test(250, "One big UPDATE of the whole %d-row table", sz);
  speedtest1_exec("UPDATE t2 SET d=b*4");
  speedtest1_end_test();


  speedtest1_begin_test(260, "Query added column after filling");
  speedtest1_exec("SELECT sum(d) FROM t2");
  speedtest1_end_test();



  n = sz/5;
  speedtest1_begin_test(270, "%d DELETEs, numeric BETWEEN, indexed", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "DELETE FROM t2 WHERE b BETWEEN ?1 AND ?2; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%maxb + 1;
    x2 = speedtest1_random()%10 + sz/5000 + x1;
    sqlite3_bind_int(g.pStmt, 1, x1);
    sqlite3_bind_int(g.pStmt, 2, x2);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  n = sz;
  speedtest1_begin_test(280, "%d DELETEs of individual rows", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "DELETE FROM t3 WHERE a=?1; -- %d times", n
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%sz + 1;
    sqlite3_bind_int(g.pStmt, 1, x1);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();


  speedtest1_begin_test(290, "Refill two %d-row tables using REPLACE", sz);
  speedtest1_exec("REPLACE INTO t2(a,b,c) SELECT a,b,c FROM t1");
  speedtest1_exec("REPLACE INTO t3(a,b,c) SELECT a,b,c FROM t1");
  speedtest1_end_test();

  speedtest1_begin_test(300, "Refill a %d-row table using (b&1)==(a&1)", sz);
  speedtest1_exec("DELETE FROM t2;");
  speedtest1_exec("INSERT INTO t2(a,b,c)\n"
                  " SELECT a,b,c FROM t1  WHERE (b&1)==(a&1);");
  speedtest1_exec("INSERT INTO t2(a,b,c)\n"
                  " SELECT a,b,c FROM t1  WHERE (b&1)<>(a&1);");
  speedtest1_end_test();


  n = sz/5;
  speedtest1_begin_test(310, "%d four-ways joins", n);
  speedtest1_exec("BEGIN");
  speedtest1_prepare(
    "SELECT t1.c FROM t1, t2, t3, t4\n"
    " WHERE t4.a BETWEEN ?1 AND ?2\n"
    "   AND t3.a=t4.b\n"
    "   AND t2.a=t3.b\n"
    "   AND t1.c=t2.c"
  );
  for(i=1; i<=n; i++){
    x1 = speedtest1_random()%sz + 1;
    x2 = speedtest1_random()%10 + x1 + 4;
    sqlite3_bind_int(g.pStmt, 1, x1);
    sqlite3_bind_int(g.pStmt, 2, x2);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();

  speedtest1_begin_test(320, "subquery in result set", n);
  speedtest1_prepare(
    "SELECT sum(a), max(c),\n"
    "       avg((SELECT a FROM t2 WHERE 5+t2.b=t1.b) AND rowid<?1), max(c)\n"
    " FROM t1 WHERE rowid<?1;"
  );
  sqlite3_bind_int(g.pStmt, 1, est_square_root(g.szTest)*50);
  speedtest1_execute();
  speedtest1_end_test();

  speedtest1_begin_test(980, "PRAGMA integrity_check");
  speedtest1_exec("PRAGMA integrity_check");
  speedtest1_end_test();


  speedtest1_begin_test(990, "ANALYZE");
  speedtest1_exec("ANALYZE");
  speedtest1_end_test();
}

/*
** A testset for common table expressions.  This exercises code
** for views, subqueries, co-routines, etc.
*/
void testset_cte(void){
  static const char *azPuzzle[] = {
    /* Easy */
    "534...9.."
    "67.195..."
    ".98....6."
    "8...6...3"
    "4..8.3..1"
    "....2...6"
    ".6....28."
    "...419..5"
    "...28..79",

    /* Medium */
    "53....9.."
    "6..195..."
    ".98....6."
    "8...6...3"
    "4..8.3..1"
    "....2...6"
    ".6....28."
    "...419..5"
    "....8..79",

    /* Hard */
    "53......."
    "6..195..."
    ".98....6."
    "8...6...3"
    "4..8.3..1"
    "....2...6"
    ".6....28."
    "...419..5"
    "....8..79",
  };
  const char *zPuz;
  double rSpacing;
  int nElem;

  if( g.szTest<25 ){
    zPuz = azPuzzle[0];
  }else if( g.szTest<70 ){
    zPuz = azPuzzle[1];
  }else{
    zPuz = azPuzzle[2];
  }
  speedtest1_begin_test(100, "Sudoku with recursive 'digits'");
  speedtest1_prepare(
    "WITH RECURSIVE\n"
    "  input(sud) AS (VALUES(?1)),\n"
    "  digits(z,lp) AS (\n"
    "    VALUES('1', 1)\n"
    "    UNION ALL\n"
    "    SELECT CAST(lp+1 AS TEXT), lp+1 FROM digits WHERE lp<9\n"
    "  ),\n"
    "  x(s, ind) AS (\n"
    "    SELECT sud, instr(sud, '.') FROM input\n"
    "    UNION ALL\n"
    "    SELECT\n"
    "      substr(s, 1, ind-1) || z || substr(s, ind+1),\n"
    "      instr( substr(s, 1, ind-1) || z || substr(s, ind+1), '.' )\n"
    "     FROM x, digits AS z\n"
    "    WHERE ind>0\n"
    "      AND NOT EXISTS (\n"
    "            SELECT 1\n"
    "              FROM digits AS lp\n"
    "             WHERE z.z = substr(s, ((ind-1)/9)*9 + lp, 1)\n"
    "                OR z.z = substr(s, ((ind-1)%%9) + (lp-1)*9 + 1, 1)\n"
    "                OR z.z = substr(s, (((ind-1)/3) %% 3) * 3\n"
    "                        + ((ind-1)/27) * 27 + lp\n"
    "                        + ((lp-1) / 3) * 6, 1)\n"
    "         )\n"
    "  )\n"
    "SELECT s FROM x WHERE ind=0;"
  );
  sqlite3_bind_text(g.pStmt, 1, zPuz, -1, SQLITE_STATIC);
  speedtest1_execute();
  speedtest1_end_test();

  speedtest1_begin_test(200, "Sudoku with VALUES 'digits'");
  speedtest1_prepare(
    "WITH RECURSIVE\n"
    "  input(sud) AS (VALUES(?1)),\n"
    "  digits(z,lp) AS (VALUES('1',1),('2',2),('3',3),('4',4),('5',5),\n"
    "                         ('6',6),('7',7),('8',8),('9',9)),\n"
    "  x(s, ind) AS (\n"
    "    SELECT sud, instr(sud, '.') FROM input\n"
    "    UNION ALL\n"
    "    SELECT\n"
    "      substr(s, 1, ind-1) || z || substr(s, ind+1),\n"
    "      instr( substr(s, 1, ind-1) || z || substr(s, ind+1), '.' )\n"
    "     FROM x, digits AS z\n"
    "    WHERE ind>0\n"
    "      AND NOT EXISTS (\n"
    "            SELECT 1\n"
    "              FROM digits AS lp\n"
    "             WHERE z.z = substr(s, ((ind-1)/9)*9 + lp, 1)\n"
    "                OR z.z = substr(s, ((ind-1)%%9) + (lp-1)*9 + 1, 1)\n"
    "                OR z.z = substr(s, (((ind-1)/3) %% 3) * 3\n"
    "                        + ((ind-1)/27) * 27 + lp\n"
    "                        + ((lp-1) / 3) * 6, 1)\n"
    "         )\n"
    "  )\n"
    "SELECT s FROM x WHERE ind=0;"
  );
  sqlite3_bind_text(g.pStmt, 1, zPuz, -1, SQLITE_STATIC);
  speedtest1_execute();
  speedtest1_end_test();

  rSpacing = 5.0/g.szTest;
  speedtest1_begin_test(300, "Mandelbrot Set with spacing=%f", rSpacing);
  speedtest1_prepare(
   "WITH RECURSIVE \n"
   "  xaxis(x) AS (VALUES(-2.0) UNION ALL SELECT x+?1 FROM xaxis WHERE x<1.2),\n"
   "  yaxis(y) AS (VALUES(-1.0) UNION ALL SELECT y+?2 FROM yaxis WHERE y<1.0),\n"
   "  m(iter, cx, cy, x, y) AS (\n"
   "    SELECT 0, x, y, 0.0, 0.0 FROM xaxis, yaxis\n"
   "    UNION ALL\n"
   "    SELECT iter+1, cx, cy, x*x-y*y + cx, 2.0*x*y + cy FROM m \n"
   "     WHERE (x*x + y*y) < 4.0 AND iter<28\n"
   "  ),\n"
   "  m2(iter, cx, cy) AS (\n"
   "    SELECT max(iter), cx, cy FROM m GROUP BY cx, cy\n"
   "  ),\n"
   "  a(t) AS (\n"
   "    SELECT group_concat( substr(' .+*#', 1+min(iter/7,4), 1), '') \n"
   "    FROM m2 GROUP BY cy\n"
   "  )\n"
   "SELECT group_concat(rtrim(t),x'0a') FROM a;"
  );
  sqlite3_bind_double(g.pStmt, 1, rSpacing*.05);
  sqlite3_bind_double(g.pStmt, 2, rSpacing);
  speedtest1_execute();
  speedtest1_end_test();

  nElem = 10000*g.szTest;
  speedtest1_begin_test(400, "EXCEPT operator on %d-element tables", nElem);
  speedtest1_prepare(
    "WITH RECURSIVE \n"
    "  t1(x) AS (VALUES(2) UNION ALL SELECT x+2 FROM t1 WHERE x<%d),\n"
    "  t2(y) AS (VALUES(3) UNION ALL SELECT y+3 FROM t2 WHERE y<%d)\n"
    "SELECT count(x), avg(x) FROM (\n"
    "  SELECT x FROM t1 EXCEPT SELECT y FROM t2 ORDER BY 1\n"
    ");",
    nElem, nElem
  );
  speedtest1_execute();
  speedtest1_end_test();

}

/* Generate two numbers between 1 and mx.  The first number is less than
** the second.  Usually the numbers are near each other but can sometimes
** be far apart.
*/
static void twoCoords(
  int p1, int p2,                   /* Parameters adjusting sizes */
  unsigned mx,                      /* Range of 1..mx */
  unsigned *pX0, unsigned *pX1      /* OUT: write results here */
){
  unsigned d, x0, x1, span;

  span = mx/100 + 1;
  if( speedtest1_random()%3==0 ) span *= p1;
  if( speedtest1_random()%p2==0 ) span = mx/2;
  d = speedtest1_random()%span + 1;
  x0 = speedtest1_random()%(mx-d) + 1;
  x1 = x0 + d;
  *pX0 = x0;
  *pX1 = x1;
}

/* The following routine is an R-Tree geometry callback.  It returns
** true if the object overlaps a slice on the Y coordinate between the
** two values given as arguments.  In other words
**
**     SELECT count(*) FROM rt1 WHERE id MATCH xslice(10,20);
**
** Is the same as saying:
**
**     SELECT count(*) FROM rt1 WHERE y1>=10 AND y0<=20;
*/
static int xsliceGeometryCallback(
  sqlite3_rtree_geometry *p,
  int nCoord,
  double *aCoord,
  int *pRes
){
  *pRes = aCoord[3]>=p->aParam[0] && aCoord[2]<=p->aParam[1];
  return SQLITE_OK;
}

/*
** A testset for the R-Tree virtual table
*/
void testset_rtree(int p1, int p2){
  unsigned i, n;
  unsigned mxCoord;
  unsigned x0, x1, y0, y1, z0, z1;
  unsigned iStep;
  int *aCheck = sqlite3_malloc( sizeof(int)*g.szTest*100 );

  mxCoord = 15000;
  n = g.szTest*100;
  speedtest1_begin_test(100, "%d INSERTs into an r-tree", n);
  speedtest1_exec("BEGIN");
  speedtest1_exec("CREATE VIRTUAL TABLE rt1 USING rtree(id,x0,x1,y0,y1,z0,z1)");
  speedtest1_prepare("INSERT INTO rt1(id,x0,x1,y0,y1,z0,z1)"
                     "VALUES(?1,?2,?3,?4,?5,?6,?7)");
  for(i=1; i<=n; i++){
    twoCoords(p1, p2, mxCoord, &x0, &x1);
    twoCoords(p1, p2, mxCoord, &y0, &y1);
    twoCoords(p1, p2, mxCoord, &z0, &z1);
    sqlite3_bind_int(g.pStmt, 1, i);
    sqlite3_bind_int(g.pStmt, 2, x0);
    sqlite3_bind_int(g.pStmt, 3, x1);
    sqlite3_bind_int(g.pStmt, 4, y0);
    sqlite3_bind_int(g.pStmt, 5, y1);
    sqlite3_bind_int(g.pStmt, 6, z0);
    sqlite3_bind_int(g.pStmt, 7, z1);
    speedtest1_execute();
  }
  speedtest1_exec("COMMIT");
  speedtest1_end_test();

  speedtest1_begin_test(101, "Copy from rtree to a regular table");
  speedtest1_exec("CREATE TABLE t1(id INTEGER PRIMARY KEY,x0,x1,y0,y1,z0,z1)");
  speedtest1_exec("INSERT INTO t1 SELECT * FROM rt1");
  speedtest1_end_test();

  n = g.szTest*20;
  speedtest1_begin_test(110, "%d one-dimensional intersect slice queries", n);
  speedtest1_prepare("SELECT count(*) FROM rt1 WHERE x0>=?1 AND x1<=?2");
  iStep = mxCoord/n;
  for(i=0; i<n; i++){
    sqlite3_bind_int(g.pStmt, 1, i*iStep);
    sqlite3_bind_int(g.pStmt, 2, (i+1)*iStep);
    speedtest1_execute();
    aCheck[i] = atoi(g.zResult);
  }
  speedtest1_end_test();

  if( g.bVerify ){
    n = g.szTest*20;
    speedtest1_begin_test(111, "Verify result from 1-D intersect slice queries");
    speedtest1_prepare("SELECT count(*) FROM t1 WHERE x0>=?1 AND x1<=?2");
    iStep = mxCoord/n;
    for(i=0; i<n; i++){
      sqlite3_bind_int(g.pStmt, 1, i*iStep);
      sqlite3_bind_int(g.pStmt, 2, (i+1)*iStep);
      speedtest1_execute();
      if( aCheck[i]!=atoi(g.zResult) ){
        fatal_error("Count disagree step %d: %d..%d.  %d vs %d",
                    i, i*iStep, (i+1)*iStep, aCheck[i], atoi(g.zResult));
      }
    }
    speedtest1_end_test();
  }
  
  n = g.szTest*20;
  speedtest1_begin_test(120, "%d one-dimensional overlap slice queries", n);
  speedtest1_prepare("SELECT count(*) FROM rt1 WHERE y1>=?1 AND y0<=?2");
  iStep = mxCoord/n;
  for(i=0; i<n; i++){
    sqlite3_bind_int(g.pStmt, 1, i*iStep);
    sqlite3_bind_int(g.pStmt, 2, (i+1)*iStep);
    speedtest1_execute();
    aCheck[i] = atoi(g.zResult);
  }
  speedtest1_end_test();

  if( g.bVerify ){
    n = g.szTest*20;
    speedtest1_begin_test(121, "Verify result from 1-D overlap slice queries");
    speedtest1_prepare("SELECT count(*) FROM t1 WHERE y1>=?1 AND y0<=?2");
    iStep = mxCoord/n;
    for(i=0; i<n; i++){
      sqlite3_bind_int(g.pStmt, 1, i*iStep);
      sqlite3_bind_int(g.pStmt, 2, (i+1)*iStep);
      speedtest1_execute();
      if( aCheck[i]!=atoi(g.zResult) ){
        fatal_error("Count disagree step %d: %d..%d.  %d vs %d",
                    i, i*iStep, (i+1)*iStep, aCheck[i], atoi(g.zResult));
      }
    }
    speedtest1_end_test();
  }
  

  n = g.szTest*20;
  speedtest1_begin_test(125, "%d custom geometry callback queries", n);
  //sqlite3_rtree_geometry_callback(g.db, "xslice", xsliceGeometryCallback, 0);
  speedtest1_prepare("SELECT count(*) FROM rt1 WHERE id MATCH xslice(?1,?2)");
  iStep = mxCoord/n;
  for(i=0; i<n; i++){
    sqlite3_bind_int(g.pStmt, 1, i*iStep);
    sqlite3_bind_int(g.pStmt, 2, (i+1)*iStep);
    speedtest1_execute();
    if( aCheck[i]!=atoi(g.zResult) ){
      fatal_error("Count disagree step %d: %d..%d.  %d vs %d",
                  i, i*iStep, (i+1)*iStep, aCheck[i], atoi(g.zResult));
    }
  }
  speedtest1_end_test();

  n = g.szTest*80;
  speedtest1_begin_test(130, "%d three-dimensional intersect box queries", n);
  speedtest1_prepare("SELECT count(*) FROM rt1 WHERE x1>=?1 AND x0<=?2"
                     " AND y1>=?1 AND y0<=?2 AND z1>=?1 AND z0<=?2");
  iStep = mxCoord/n;
  for(i=0; i<n; i++){
    sqlite3_bind_int(g.pStmt, 1, i*iStep);
    sqlite3_bind_int(g.pStmt, 2, (i+1)*iStep);
    speedtest1_execute();
    aCheck[i] = atoi(g.zResult);
  }
  speedtest1_end_test();

  n = g.szTest*100;
  speedtest1_begin_test(140, "%d rowid queries", n);
  speedtest1_prepare("SELECT * FROM rt1 WHERE id=?1");
  for(i=1; i<=n; i++){
    sqlite3_bind_int(g.pStmt, 1, i);
    speedtest1_execute();
  }
  speedtest1_end_test();
}

/*
** A testset used for debugging speedtest1 itself.
*/
void testset_debug1(void){
  unsigned i, n;
  unsigned x1, x2;
  char zNum[2000];              /* A number name */

  n = g.szTest;
  for(i=1; i<=n; i++){
    x1 = swizzle(i, n);
    x2 = swizzle(x1, n);
    speedtest1_numbername(x1, zNum, sizeof(zNum));
    printf("%5d %5d %5d %s\n", i, x1, x2, zNum);
  }
}

int main(int argc, char **argv){
  int doAutovac = 0;            /* True for --autovacuum */
  int cacheSize = 0;            /* Desired cache size.  0 means default */
  int doExclusive = 0;          /* True for --exclusive */
  int nHeap = 0, mnHeap = 0;    /* Heap size from --heap */
  int doIncrvac = 0;            /* True for --incrvacuum */
  const char *zJMode = 0;       /* Journal mode */
  const char *zKey = 0;         /* Encryption key */
  int nLook = 0, szLook = 0;    /* --lookaside configuration */
  int noSync = 0;               /* True for --nosync */
  int pageSize = 0;             /* Desired page size.  0 means default */
  int nPCache = 0, szPCache = 0;/* --pcache configuration */
  int nScratch = 0, szScratch=0;/* --scratch configuration */
  int showStats = 0;            /* True for --stats */
  const char *zTSet = "main";   /* Which --testset torun */
  int doTrace = 0;              /* True for --trace */
  const char *zEncoding = 0;    /* --utf16be or --utf16le */
  const char *zDbName = 0;      /* Name of the test database */

  void *pHeap = 0;              /* Allocated heap space */
  void *pLook = 0;              /* Allocated lookaside space */
  void *pPCache = 0;            /* Allocated storage for pcache */
  void *pScratch = 0;           /* Allocated storage for scratch */
  int iCur, iHi;                /* Stats values, current and "highwater" */
  int i;                        /* Loop counter */
  int rc;                       /* API return code */

  /* Process command-line arguments */
  g.zWR = "";
  g.zNN = "";
  g.zPK = "UNIQUE";
  g.szTest = 100;

  /* Emscripten commandline argument processing - first arg is ours */
  int arg = argc > 1 ? argv[1][0] - '0' : 3;
  switch(arg) {
    case 0: return 0; break;
    case 1: arg = 5; break;
    case 2: arg = 15; break;
    case 3: arg = 40; break;
    case 4: arg = 80; break;
    case 5: arg = 160; break;
    default:
      printf("error: %d\\n", arg);
      return -1;
  }
  g.szTest = arg;

  /* Back to sqlite - after the first is theirs*/
  for(i=2; i<argc; i++){
    const char *z = argv[i];
    if( z[0]=='-' ){
      do{ z++; }while( z[0]=='-' );
      if( strcmp(z,"autovacuum")==0 ){
        doAutovac = 1;
      }else if( strcmp(z,"cachesize")==0 ){
        if( i>=argc-1 ) fatal_error("missing argument on %s\n", argv[i]);
        i++;
        cacheSize = integerValue(argv[i]);
      }else if( strcmp(z,"exclusive")==0 ){
        doExclusive = 1;
      }else if( strcmp(z,"explain")==0 ){
        g.bSqlOnly = 1;
        g.bExplain = 1;
      }else if( strcmp(z,"heap")==0 ){
        if( i>=argc-2 ) fatal_error("missing arguments on %s\n", argv[i]);
        nHeap = integerValue(argv[i+1]);
        mnHeap = integerValue(argv[i+2]);
        i += 2;
      }else if( strcmp(z,"incrvacuum")==0 ){
        doIncrvac = 1;
      }else if( strcmp(z,"journal")==0 ){
        if( i>=argc-1 ) fatal_error("missing argument on %s\n", argv[i]);
        zJMode = argv[++i];
      }else if( strcmp(z,"key")==0 ){
        if( i>=argc-1 ) fatal_error("missing argument on %s\n", argv[i]);
        zKey = argv[++i];
      }else if( strcmp(z,"lookaside")==0 ){
        if( i>=argc-2 ) fatal_error("missing arguments on %s\n", argv[i]);
        nLook = integerValue(argv[i+1]);
        szLook = integerValue(argv[i+2]);
        i += 2;
      }else if( strcmp(z,"nosync")==0 ){
        noSync = 1;
      }else if( strcmp(z,"notnull")==0 ){
        g.zNN = "NOT NULL";
      }else if( strcmp(z,"pagesize")==0 ){
        if( i>=argc-1 ) fatal_error("missing argument on %s\n", argv[i]);
        pageSize = integerValue(argv[++i]);
      }else if( strcmp(z,"pcache")==0 ){
        if( i>=argc-2 ) fatal_error("missing arguments on %s\n", argv[i]);
        nPCache = integerValue(argv[i+1]);
        szPCache = integerValue(argv[i+2]);
        i += 2;
      }else if( strcmp(z,"primarykey")==0 ){
        g.zPK = "PRIMARY KEY";
      }else if( strcmp(z,"reprepare")==0 ){
        g.bReprepare = 1;
      }else if( strcmp(z,"scratch")==0 ){
        if( i>=argc-2 ) fatal_error("missing arguments on %s\n", argv[i]);
        nScratch = integerValue(argv[i+1]);
        szScratch = integerValue(argv[i+2]);
        i += 2;
      }else if( strcmp(z,"sqlonly")==0 ){
        g.bSqlOnly = 1;
      }else if( strcmp(z,"size")==0 ){
        if( i>=argc-1 ) fatal_error("missing argument on %s\n", argv[i]);
        g.szTest = integerValue(argv[++i]);
      }else if( strcmp(z,"stats")==0 ){
        showStats = 1;
      }else if( strcmp(z,"testset")==0 ){
        if( i>=argc-1 ) fatal_error("missing argument on %s\n", argv[i]);
        zTSet = argv[++i];
      }else if( strcmp(z,"trace")==0 ){
        doTrace = 1;
      }else if( strcmp(z,"utf16le")==0 ){
        zEncoding = "utf16le";
      }else if( strcmp(z,"utf16be")==0 ){
        zEncoding = "utf16be";
      }else if( strcmp(z,"verify")==0 ){
        g.bVerify = 1;
      }else if( strcmp(z,"without-rowid")==0 ){
        g.zWR = "WITHOUT ROWID";
        g.zPK = "PRIMARY KEY";
      }else if( strcmp(z, "help")==0 || strcmp(z,"?")==0 ){
        printf(zHelp, argv[0]);
        exit(0);
      }else{
        fatal_error("unknown option: %s\nUse \"%s -?\" for help\n",
                    argv[i], argv[0]);
      }
    }else if( zDbName==0 ){
      zDbName = argv[i];
    }else{
      fatal_error("surplus argument: %s\nUse \"%s -?\" for help\n",
                  argv[i], argv[0]);
    }
  }
#if 0
  if( zDbName==0 ){
    fatal_error(zHelp, argv[0]);
  }
#endif
  if( nHeap>0 ){
    pHeap = malloc( nHeap );
    if( pHeap==0 ) fatal_error("cannot allocate %d-byte heap\n", nHeap);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, pHeap, nHeap, mnHeap);
    if( rc ) fatal_error("heap configuration failed: %d\n", rc);
  }
  if( nPCache>0 && szPCache>0 ){
    pPCache = malloc( nPCache*(sqlite3_int64)szPCache );
    if( pPCache==0 ) fatal_error("cannot allocate %lld-byte pcache\n",
                                 nPCache*(sqlite3_int64)szPCache);
    rc = sqlite3_config(SQLITE_CONFIG_PAGECACHE, pPCache, szPCache, nPCache);
    if( rc ) fatal_error("pcache configuration failed: %d\n", rc);
  }
  if( nScratch>0 && szScratch>0 ){
    pScratch = malloc( nScratch*(sqlite3_int64)szScratch );
    if( pScratch==0 ) fatal_error("cannot allocate %lld-byte scratch\n",
                                 nScratch*(sqlite3_int64)szScratch);
    rc = sqlite3_config(SQLITE_CONFIG_SCRATCH, pScratch, szScratch, nScratch);
    if( rc ) fatal_error("scratch configuration failed: %d\n", rc);
  }
  if( nLook>0 ){
    sqlite3_config(SQLITE_CONFIG_LOOKASIDE, 0, 0);
  }
 
  /* Open the database and the input file */
  if( sqlite3_open(":memory:", &g.db) ){
    fatal_error("Cannot open database file: %s\n", zDbName);
  }
  if( nLook>0 && szLook>0 ){
    pLook = malloc( nLook*szLook );
    rc = sqlite3_db_config(g.db, SQLITE_DBCONFIG_LOOKASIDE, pLook, szLook,nLook);
    if( rc ) fatal_error("lookaside configuration failed: %d\n", rc);
  }

  /* Set database connection options */
  sqlite3_create_function(g.db, "random", 0, SQLITE_UTF8, 0, randomFunc1, 0, 0);
  if( doTrace ) sqlite3_trace(g.db, traceCallback, 0);
  if( zKey ){
    speedtest1_exec("PRAGMA key('%s')", zKey);
  }
  if( zEncoding ){
    speedtest1_exec("PRAGMA encoding=%s", zEncoding);
  }
  if( doAutovac ){
    speedtest1_exec("PRAGMA auto_vacuum=FULL");
  }else if( doIncrvac ){
    speedtest1_exec("PRAGMA auto_vacuum=INCREMENTAL");
  }
  if( pageSize ){
    speedtest1_exec("PRAGMA page_size=%d", pageSize);
  }
  if( cacheSize ){
    speedtest1_exec("PRAGMA cache_size=%d", cacheSize);
  }
  if( noSync ) speedtest1_exec("PRAGMA synchronous=OFF");
  if( doExclusive ){
    speedtest1_exec("PRAGMA locking_mode=EXCLUSIVE");
  }
  if( zJMode ){
    speedtest1_exec("PRAGMA journal_mode=%s", zJMode);
  }

  if( g.bExplain ) printf(".explain\n.echo on\n");
  if( strcmp(zTSet,"main")==0 ){
    testset_main();
  }else if( strcmp(zTSet,"debug1")==0 ){
    testset_debug1();
  }else if( strcmp(zTSet,"cte")==0 ){
    testset_cte();
  }else if( strcmp(zTSet,"rtree")==0 ){
    testset_rtree(6, 147);
  }else{
    fatal_error("unknown testset: \"%s\"\nChoices: main debug1 cte rtree\n",
                 zTSet);
  }
  speedtest1_final();

  /* Database connection statistics printed after both prepared statements
  ** have been finalized */
#if SQLITE_VERSION_NUMBER>=3007009
  if( showStats ){
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_USED, &iCur, &iHi, 0);
    printf("-- Lookaside Slots Used:        %d (max %d)\n", iCur,iHi);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_HIT, &iCur, &iHi, 0);
    printf("-- Successful lookasides:       %d\n", iHi);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE, &iCur,&iHi,0);
    printf("-- Lookaside size faults:       %d\n", iHi);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL, &iCur,&iHi,0);
    printf("-- Lookaside OOM faults:        %d\n", iHi);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_CACHE_USED, &iCur, &iHi, 0);
    printf("-- Pager Heap Usage:            %d bytes\n", iCur);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_CACHE_HIT, &iCur, &iHi, 1);
    printf("-- Page cache hits:             %d\n", iCur);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_CACHE_MISS, &iCur, &iHi, 1);
    printf("-- Page cache misses:           %d\n", iCur);
#if SQLITE_VERSION_NUMBER>=3007012
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_CACHE_WRITE, &iCur, &iHi, 1);
    printf("-- Page cache writes:           %d\n", iCur); 
#endif
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_SCHEMA_USED, &iCur, &iHi, 0);
    printf("-- Schema Heap Usage:           %d bytes\n", iCur); 
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_STMT_USED, &iCur, &iHi, 0);
    printf("-- Statement Heap Usage:        %d bytes\n", iCur); 
  }
#endif

  sqlite3_close(g.db);

  /* Global memory usage statistics printed after the database connection
  ** has closed.  Memory usage should be zero at this point. */
  if( showStats ){
    sqlite3_status(SQLITE_STATUS_MEMORY_USED, &iCur, &iHi, 0);
    printf("-- Memory Used (bytes):         %d (max %d)\n", iCur,iHi);
#if SQLITE_VERSION_NUMBER>=3007000
    sqlite3_status(SQLITE_STATUS_MALLOC_COUNT, &iCur, &iHi, 0);
    printf("-- Outstanding Allocations:     %d (max %d)\n", iCur,iHi);
#endif
    sqlite3_status(SQLITE_STATUS_PAGECACHE_OVERFLOW, &iCur, &iHi, 0);
    printf("-- Pcache Overflow Bytes:       %d (max %d)\n", iCur,iHi);
    sqlite3_status(SQLITE_STATUS_SCRATCH_OVERFLOW, &iCur, &iHi, 0);
    printf("-- Scratch Overflow Bytes:      %d (max %d)\n", iCur,iHi);
    sqlite3_status(SQLITE_STATUS_MALLOC_SIZE, &iCur, &iHi, 0);
    printf("-- Largest Allocation:          %d bytes\n",iHi);
    sqlite3_status(SQLITE_STATUS_PAGECACHE_SIZE, &iCur, &iHi, 0);
    printf("-- Largest Pcache Allocation:   %d bytes\n",iHi);
    sqlite3_status(SQLITE_STATUS_SCRATCH_SIZE, &iCur, &iHi, 0);
    printf("-- Largest Scratch Allocation:  %d bytes\n", iHi);
  }

  /* Release memory */
  free( pLook );
  free( pPCache );
  free( pScratch );
  free( pHeap );
  return 0;
}
