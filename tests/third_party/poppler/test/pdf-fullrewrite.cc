//========================================================================
//
// pdf-fullrewrite.cc
//
// Copyright 2007 Julien Rebetez
//
//========================================================================
#include "config.h"
#include <poppler-config.h>
#include "GlobalParams.h"
#include "Error.h"
#include "PDFDoc.h"
#include "goo/GooString.h"

int main (int argc, char *argv[])
{
  PDFDoc *doc;
  GooString *inputName, *outputName;

  // parse args
  if (argc < 3) {
    fprintf(stderr, "usage: %s INPUT-FILE OUTPUT-FILE\n", argv[0]);
    return 1;
  }

  inputName = new GooString(argv[1]);
  outputName = new GooString(argv[2]);

  globalParams = new GlobalParams();

  doc = new PDFDoc(inputName);

  if (!doc->isOk()) {
    delete doc;
    fprintf(stderr, "Error loading document !\n");
    return 1;
  }


  int res = doc->saveAs(outputName, writeForceRewrite);

  delete doc;
  delete globalParams;
  delete outputName;
  return res;
}
