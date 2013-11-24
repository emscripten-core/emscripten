//========================================================================
//
// pdf-fullrewrite.cc
//
// Copyright 2007 Julien Rebetez
// Copyright 2012 Fabio D'Urso
//
//========================================================================

#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "goo/GooString.h"
#include "utils/parseargs.h"

static GBool compareDocuments(PDFDoc *origDoc, PDFDoc *newDoc);
static GBool compareObjects(Object *objA, Object *objB);

static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static GBool forceIncremental = gFalse;
static GBool checkOutput = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-opw",    argString,   ownerPassword,    sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,     sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-i",      argFlag,     &forceIncremental,0,
   "incremental update mode"},
  {"-check",  argFlag,     &checkOutput,     0,
   "verify the generated document"},
  {"-h",      argFlag,     &printHelp,       0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,       0,
   "print usage information"},
  {"--help",  argFlag,     &printHelp,       0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,       0,
   "print usage information"},
  {NULL}
};

int main (int argc, char *argv[])
{
  PDFDoc *doc = NULL;
  PDFDoc *docOut = NULL;
  GooString *inputName = NULL;
  GooString *outputName = NULL;
  GooString *ownerPW = NULL;
  GooString *userPW = NULL;
  int res = 0;

  // parse args
  GBool ok = parseArgs(argDesc, &argc, argv);
  if (!ok || (argc < 3) || printHelp) {
    printUsage(argv[0], "INPUT-FILE OUTPUT-FILE", argDesc);
    if (!printHelp) {
      res = 1;
    }
    goto done;
  }

  inputName = new GooString(argv[1]);
  outputName = new GooString(argv[2]);

  if (ownerPassword[0] != '\001') {
    ownerPW = new GooString(ownerPassword);
  }
  if (userPassword[0] != '\001') {
    userPW = new GooString(userPassword);
  }

  // load input document
  globalParams = new GlobalParams();
  doc = new PDFDoc(inputName, ownerPW, userPW);
  if (!doc->isOk()) {
    fprintf(stderr, "Error loading input document\n");
    res = 1;
    goto done;
  }

  // save it back (in rewrite or incremental update mode)
  if (doc->saveAs(outputName, forceIncremental ? writeForceIncremental : writeForceRewrite) != 0) {
    fprintf(stderr, "Error saving document\n");
    res = 1;
    goto done;
  }

  if (checkOutput) {
    // open the generated document to verify it
    docOut = new PDFDoc(outputName, ownerPW, userPW);
    if (!docOut->isOk()) {
      fprintf(stderr, "Error loading generated document\n");
      res = 1;
    } else if (!compareDocuments(doc, docOut)) {
      fprintf(stderr, "Verification failed\n");
      res = 1;
    }
  } else {
    delete outputName;
  }

done:
  delete docOut;
  delete doc;
  delete globalParams;
  delete userPW;
  delete ownerPW;
  return res;
}

static GBool compareDictionaries(Dict *dictA, Dict *dictB)
{
  const int length = dictA->getLength();
  if (dictB->getLength() != length)
    return gFalse;

  /* Check that every key in dictA is contained in dictB.
   * Since keys are unique and we've already checked that dictA and dictB
   * contain the same number of entries, we don't need to check that every key
   * in dictB is also contained in dictA */
  for (int i = 0; i < length; ++i) {
    Object valA, valB;
    const char *key = dictA->getKey(i);
    dictA->getValNF(i, &valA);
    dictB->lookupNF(key, &valB);
    if (!compareObjects(&valA, &valB))
      return gFalse;
    valA.free();
    valB.free();
  }

  return gTrue;
}

static GBool compareObjects(Object *objA, Object *objB)
{
  switch (objA->getType()) {
    case objBool:
    {
      if (objB->getType() != objBool) {
        return gFalse;
      } else {
        return (objA->getBool() == objB->getBool());
      }
    }
    case objInt:
    case objInt64:
    case objReal:
    {
      if (!objB->isNum()) {
        return gFalse;
      } else {
        // Fuzzy comparison
        const double diff = objA->getNum() - objB->getNum();
        return (-0.01 < diff) && (diff < 0.01);
      }
    }
    case objString:
    {
      if (objB->getType() != objString) {
        return gFalse;
      } else {
        GooString *strA = objA->getString();
        GooString *strB = objB->getString();
        return (strA->cmp(strB) == 0);
      }
    }
    case objName:
    {
      if (objB->getType() != objName) {
        return gFalse;
      } else {
        GooString nameA(objA->getName());
        GooString nameB(objB->getName());
        return (nameA.cmp(&nameB) == 0);
      }
    }
    case objNull:
    {
      if (objB->getType() != objNull) {
        return gFalse;
      } else {
        return gTrue;
      }
    }
    case objArray:
    {
      if (objB->getType() != objArray) {
        return gFalse;
      } else {
        Array *arrayA = objA->getArray();
        Array *arrayB = objB->getArray();
        const int length = arrayA->getLength();
        if (arrayB->getLength() != length) {
          return gFalse;
        } else {
          for (int i = 0; i < length; ++i) {
            Object elemA, elemB;
            arrayA->getNF(i, &elemA);
            arrayB->getNF(i, &elemB);
            if (!compareObjects(&elemA, &elemB)) {
              return gFalse;
            }
            elemA.free();
            elemB.free();
          }
          return gTrue;
        }
      }
    }
    case objDict:
    {
      if (objB->getType() != objDict) {
        return gFalse;
      } else {
        Dict *dictA = objA->getDict();
        Dict *dictB = objB->getDict();
        return compareDictionaries(dictA, dictB);
      }
    }
    case objStream:
    {
      if (objB->getType() != objStream) {
        return gFalse;
      } else {
        Stream *streamA = objA->getStream();
        Stream *streamB = objB->getStream();
        if (!compareDictionaries(streamA->getDict(), streamB->getDict())) {
          return gFalse;
        } else {
          int c;
          streamA->reset();
          streamB->reset();
          do
          {
            c = streamA->getChar();
            if (c != streamB->getChar()) {
              return gFalse;
            }
          } while (c != EOF);
          return gTrue;
        }
      }
      return gTrue;
    }
    case objRef:
    {
      if (objB->getType() != objRef) {
        return gFalse;
      } else {
        Ref refA = objA->getRef();
        Ref refB = objB->getRef();
        return (refA.num == refB.num) && (refA.gen == refB.gen);
      }
    }
    default:
    {
      fprintf(stderr, "compareObjects failed: unexpected object type %u\n", objA->getType());
      return gFalse;
    }
  }
}

static GBool compareDocuments(PDFDoc *origDoc, PDFDoc *newDoc)
{
  GBool result = gTrue;
  XRef *origXRef = origDoc->getXRef();
  XRef *newXRef = newDoc->getXRef();

  // Make sure that special flags are set in both documents
  origXRef->scanSpecialFlags();
  newXRef->scanSpecialFlags();

  // Compare XRef tables' size
  const int origNumObjects = origXRef->getNumObjects();
  const int newNumObjects = newXRef->getNumObjects();
  if (forceIncremental && origXRef->isXRefStream()) {
    // In case of incremental update, expect a new entry to be appended to store the new XRef stream
    if (origNumObjects+1 != newNumObjects) {
      fprintf(stderr, "XRef table: Unexpected number of entries (%d+1 != %d)\n", origNumObjects, newNumObjects);
      result = gFalse;
    }
  } else {
    // In all other cases the number of entries must be the same
    if (origNumObjects != newNumObjects) {
      fprintf(stderr, "XRef table: Different number of entries (%d != %d)\n", origNumObjects, newNumObjects);
      result = gFalse;
    }
  }

  // Compare each XRef entry
  const int numObjects = (origNumObjects < newNumObjects) ? origNumObjects : newNumObjects;
  for (int i = 0; i < numObjects; ++i) {
    XRefEntryType origType = origXRef->getEntry(i)->type;
    XRefEntryType newType = newXRef->getEntry(i)->type;
    const int origGenNum = (origType != xrefEntryCompressed) ? origXRef->getEntry(i)->gen : 0;
    const int newGenNum = (newType != xrefEntryCompressed) ? newXRef->getEntry(i)->gen : 0;

    // Check that DontRewrite entries are freed in full rewrite mode
    if (!forceIncremental && origXRef->getEntry(i)->getFlag(XRefEntry::DontRewrite)) {
      if (newType != xrefEntryFree || origGenNum+1 != newGenNum) {
        fprintf(stderr, "XRef entry %u: DontRewrite entry was not freed correctly\n", i);
        result = gFalse;
      }
      continue; // There's nothing left to check for this entry
    }

    // Compare generation numbers
    // Object num 0 should always have gen 65535 according to specs, but some
    // documents have it set to 0. We always write 65535 in output
    if (i != 0) {
      if (origGenNum != newGenNum) {
        fprintf(stderr, "XRef entry %u: generation numbers differ (%d != %d)\n", i, origGenNum, newGenNum);
        result = gFalse;
        continue;
      }
    } else {
      if (newGenNum != 65535) {
        fprintf(stderr, "XRef entry %u: generation number was expected to be 65535 (%d != 65535)\n", i, newGenNum);
        result = gFalse;
        continue;
      }
    }

    // Compare object flags. A failure shows that there's some error in XRef::scanSpecialFlags()
    if (origXRef->getEntry(i)->flags != newXRef->getEntry(i)->flags) {
      fprintf(stderr, "XRef entry %u: flags detected by scanSpecialFlags differ (%d != %d)\n", i, origXRef->getEntry(i)->flags, newXRef->getEntry(i)->flags);
      result = gFalse;
    }

    // Check that either both are free or both are in use
    if ((origType == xrefEntryFree) != (newType == xrefEntryFree)) {
      const char *origStatus = (origType == xrefEntryFree) ? "free" : "in use";
      const char *newStatus = (newType == xrefEntryFree) ? "free" : "in use";
      fprintf(stderr, "XRef entry %u: usage status differs (%s != %s)\n", i, origStatus, newStatus);
      result = gFalse;
      continue;
    }

    // Skip free entries
    if (origType == xrefEntryFree) {
      continue;
    }

    // Compare contents
    Object origObj, newObj;
    origXRef->fetch(i, origGenNum, &origObj);
    newXRef->fetch(i, newGenNum, &newObj);
    if (!compareObjects(&origObj, &newObj)) {
      fprintf(stderr, "XRef entry %u: contents differ\n", i);
      result = gFalse;
    }
    origObj.free();
    newObj.free();
  }

  return result;
}
