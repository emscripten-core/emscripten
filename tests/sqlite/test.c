/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int main(){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  int i;
  const char *commands[] = {
    "CREATE TABLE t1(a INTEGER, b INTEGER, c VARCHAR(100));",
    "INSERT INTO t1 VALUES(1,13153,'thirteen thousand one hundred fifty three');",
    "INSERT INTO t1 VALUES(1,987,'some other number');",
    "SELECT count(*) FROM t1;",
    "SELECT a, b, c FROM t1;",
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

