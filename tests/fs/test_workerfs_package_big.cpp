#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <emscripten.h>

double before_it_all;

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish() {
  // load some file data, SYNCHRONOUSLY :)
  char buffer[100];
  int num;

  printf("load files\n");
  FILE *f1 = fopen("files/file1.txt", "r");
  assert(f1);
  FILE *f2 = fopen("files/file2.txt", "r");
  assert(f2);
  FILE *f3 = fopen("files/file3.txt", "r");
  assert(f3);
  FILE *files[] = { f1, f2, f3 };
  double before = emscripten_get_now();
  int counter = 0;
  for (int i = 0; i < /* 10* */ 2*8*1024*1024 - 5*1024*1024; i += 100*1024) {
    i += random() % 10;
    int which = i % 3;
    FILE *f = files[which];
    //printf("%d read %d: %d (%d)\n", counter, which, i, i % 10);
    int off = i % 10;
    int ret = fseek(f, i, SEEK_SET);
    assert(ret == 0);
    num = fread(buffer, 1, 5, f);
    if (num != 5) {
      printf("%d read %d: %d failed num\n", counter, which, i);
      abort();
    }
    buffer[5] = 0;
    char correct[] = "01234567890123456789";
    if (strncmp(buffer, correct + off, 5) != 0) {
      printf("%d read %d: %d (%d) failed data\n", counter, which, i, i % 10);
      abort();
    }
    counter++;
  }
  double after = emscripten_get_now();
  fclose(f1);
  fclose(f2);
  fclose(f3);
  printf("read IO time: %f\n", after - before);

  printf("total time: %f\n", after - before_it_all);

  // all done
  printf("success\n");
  int result = 1;
  REPORT_RESULT();
}

}

int main() {
  before_it_all = emscripten_get_now();

  // Load the metadata and data of our file package. When they arrive, load the contents of the package into our filesystem.
  // The data arrives as a Blob, which could in other cases arrive from any other way a Blob can arrive:
  //   * Local file the user selected
  //   * Data loaded from IndexedDB
  // In all cases, including the one here of a network request, Blobs allow the browser to optimize them so that
  // a large file is not necessarily all in memory at once.
  EM_ASM({
    var meta, blob;
    function maybeReady() {
      if (!(meta && blob)) return;

      meta = JSON.parse(meta);

      Module.print('loading into filesystem');
      FS.mkdir('/files');
      FS.mount(WORKERFS, {
        packages: [{ metadata: meta, blob: blob }]
      }, '/files');

      Module.ccall('finish');
    }

    function saveLocally(arrayBuffer) {
      var dataToSave = new Blob([arrayBuffer]);
      arrayBuffer = null;

      function finishSave(data) {
        blob = data;
        Module.print('save finished: ' + [blob, blob.size]);
        maybeReady();
      }

      Module.print('saving data locally');

      var indexedDB = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
      var IDB_RO = "readonly";
      var IDB_RW = "readwrite";
      var DB_NAME = 'EM_WORKERFS_CACHE';
      var DB_VERSION = 1;
      var PACKAGE_STORE_NAME = 'PACKAGES';
      var PACKAGE_NAME = 'package';

      function openDatabase(callback) {
        Module.print('open IDB');
        var openRequest = indexedDB.open(DB_NAME, DB_VERSION);
        openRequest.onupgradeneeded = function(event) {
          var db = event.target.result;
          if (db.objectStoreNames.contains(PACKAGE_STORE_NAME)) {
            db.deleteObjectStore(PACKAGE_STORE_NAME);
          }
          var packages = db.createObjectStore(PACKAGE_STORE_NAME);
        };
        openRequest.onsuccess = function(event) {
          Module.print('open IDB succeeded');
          var db = event.target.result;
          callback(db);
        };
      };

      function fetchPackage(db, packageName, callback) {
        Module.print('fetch data');
        var transaction = db.transaction([PACKAGE_STORE_NAME], IDB_RO);
        var packages = transaction.objectStore(PACKAGE_STORE_NAME);
        var getRequest = packages.get(packageName);
        getRequest.onsuccess = function(event) {
          var result = event.target.result;
          callback(result);
        };
      };

      function cachePackage(db, packageName, packageData, callback) {
        Module.print('cache data ' + [packageData, packageData.size]);
        var transaction = db.transaction([PACKAGE_STORE_NAME], IDB_RW);
        var packages = transaction.objectStore(PACKAGE_STORE_NAME);
        var putPackageRequest = packages.put(packageData, packageName);
        putPackageRequest.onsuccess = function(event) {
          Module.print('cache data succeeded');
          callback();
        };
        putPackageRequest.onerror = function(event) {
          Module.print('cache data FAILED');
        };
      };

      // main

      openDatabase(function(db) {
        cachePackage(db, PACKAGE_NAME, dataToSave, function() {
          fetchPackage(db, PACKAGE_NAME, finishSave);
        });
      });
    }

    var meta_xhr = new XMLHttpRequest();
    meta_xhr.open("GET", "files.js.metadata", true);
    meta_xhr.responseType = "text";
    meta_xhr.onload = function() {
      Module.print('got metadata');
      meta = meta_xhr.response;
      maybeReady();
    };
    meta_xhr.send();

    var data_xhr = new XMLHttpRequest();
    data_xhr.open("GET", "files.data", true);
    data_xhr.responseType = "blob";
    data_xhr.onload = function() {
      Module.print('got data');
      saveLocally(data_xhr.response);
      maybeReady();
    };
    data_xhr.send();
  });

  emscripten_exit_with_live_runtime();

  return 1;
}

