#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include <emscripten.h>

int print = 1;

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  if (!print) return 0;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int test(){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  int i;
  const char *commands[] = {
    "CREATE TABLE t2(a INTEGER, b INTEGER, c VARCHAR(100));",
    "INSERT INTO t2 VALUES(1,13153,'thirteen thousand one hundred fifty three');",
    "INSERT INTO t2 VALUES(1,987,'some other number');",
    "SELECT count(*) FROM t2;",
    "SELECT datetime('2012-04-16 12:35:57', '+1 days');",
    "SELECT a, b, c FROM t2;",
    NULL
  };

  rc = sqlite3_open(":memory:", &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }
  for (i = 0; commands[i]; i++) {
    rc = sqlite3_exec(db, commands[i], callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error on %d: %s\n", i, zErrMsg);
      sqlite3_free(zErrMsg);
      exit(1);
    }
  }
  sqlite3_close(db);
  return 0;
}

int main(int argc, char **argv){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc, i;
  double t;
  int n, m;

  n = argc > 1 ? atoi(argv[1]) : 5000;
  m = argc > 2 ? atoi(argv[2]) : 1;

  rc = sqlite3_open(":memory:", &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  #define RUN(cmd) \
    { \
      rc = sqlite3_exec(db, cmd, callback, 0, &zErrMsg); \
      if( rc!=SQLITE_OK ){ \
        fprintf(stderr, "SQL error on %d: %s\n", i, zErrMsg); \
        sqlite3_free(zErrMsg); \
        exit(1); \
      } \
    }

  #define TIME(msg) \
    { \
      double now = emscripten_get_now(); \
      printf(msg " : took %f ms\n", now - t); \
      t = now; \
    }

  t = emscripten_get_now();
  TIME("'startup' - IGNORE THIS VALUE, it is an artifact");

  RUN("CREATE TABLE t1(a INTEGER, b INTEGER, c VARCHAR(100));");
  TIME("create table");

  RUN("BEGIN;");

  // n*5 INSERTs in a transaction
  for (i = 0; i < n; i++) {
    RUN("INSERT INTO t1 VALUES(1,12345,'one 1 one 1 one 1');");
    RUN("INSERT INTO t1 VALUES(2,23422,'two two two two');");
    RUN("INSERT INTO t1 VALUES(3,31233,'three three 33333333333 three');");
    RUN("INSERT INTO t1 VALUES(4,41414,'FOUR four 4 phor FOUR 44444');");
    RUN("INSERT INTO t1 VALUES(5,52555,'five 5 FIVE Five phayve 55 5 5 5 5 55  5');");
  }
  TIME("25,000 inserts");

  RUN("COMMIT;");
  TIME("commit");

  // Counts
  for (i = 0; i < m; i++) {
    print = i == 0;
    RUN("SELECT count(*) FROM t1;");
    RUN("SELECT count(*) FROM t1 WHERE a == 4");
    RUN("SELECT count(*) FROM t1 WHERE b > 20000 AND b < 50000;");
    RUN("SELECT count(*) FROM t1 WHERE c like '%three%';");
  }
  TIME("selects");

  // Index
  RUN("CREATE INDEX iiaa ON t1(a);");
  RUN("CREATE INDEX iibb ON t1(b);");
  TIME("create indexes");

  for (i = 0; i < m; i++) {
    print = i == 0;
    RUN("SELECT count(*) FROM t1 WHERE a == 4");
    RUN("SELECT count(*) FROM t1 WHERE b > 20000 AND b < 50000;");
  }
  TIME("selects with indexes");

  sqlite3_close(db);

  return test();
}

