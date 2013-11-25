//========================================================================
//
// Form.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2006-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright 2007-2012 Albert Astals Cid <aacid@kde.org>
// Copyright 2007-2008, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2007, 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2007 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright 2008, 2011 Pino Toscano <pino@kde.org>
// Copyright 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright 2009 Matthias Drochner <M.Drochner@fz-juelich.de>
// Copyright 2009 KDAB via Guillermo Amaral <gamaral@amaral.com.mx>
// Copyright 2010, 2012 Mark Riedesel <mark@klowner.com>
// Copyright 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// 
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <set>
#include <stddef.h>
#include <string.h>
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "Error.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Gfx.h"
#include "Form.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "PDFDocEncoding.h"
#include "Annot.h"
#include "Link.h"

//return a newly allocated char* containing an UTF16BE string of size length
char* pdfDocEncodingToUTF16 (GooString* orig, int* length)
{
  //double size, a unicode char takes 2 char, add 2 for the unicode marker
  *length = 2+2*orig->getLength();
  char *result = new char[(*length)];
  char *cstring = orig->getCString();
  //unicode marker
  result[0] = 0xfe;
  result[1] = 0xff;
  //convert to utf16
  for(int i=2,j=0; i<(*length); i+=2,j++) {
    Unicode u = pdfDocEncoding[(unsigned int)((unsigned char)cstring[j])]&0xffff;
    result[i] = (u >> 8) & 0xff;
    result[i+1] = u & 0xff;
  }
  return result;
}

static GooString *convertToUtf16(GooString *pdfDocEncodingString)
{
  int tmp_length;
  char* tmp_str = pdfDocEncodingToUTF16(pdfDocEncodingString, &tmp_length);
  delete pdfDocEncodingString;
  pdfDocEncodingString = new GooString(tmp_str, tmp_length);
  delete [] tmp_str;
  return pdfDocEncodingString;
}


FormWidget::FormWidget(PDFDoc *docA, Object *aobj, unsigned num, Ref aref, FormField *fieldA)
{
  ref = aref;
  ID = 0;
  childNum = num;
  doc = docA;
  xref = doc->getXRef();
  aobj->copy(&obj);
  type = formUndef;
  field = fieldA;
  widget = NULL;
}

FormWidget::~FormWidget()
{
  if (widget)
    widget->decRefCnt();
  obj.free ();
}

#ifdef DEBUG_FORMS
void FormWidget::print(int indent) {
  printf ("%*s+ (%d %d): [widget]\n", indent, "", ref.num, ref.gen);
}
#endif

void FormWidget::createWidgetAnnotation() {
  if (widget)
    return;

  Object obj1;
  obj1.initRef(ref.num, ref.gen);
  widget = new AnnotWidget(doc, obj.getDict(), &obj1, field);
  obj1.free();
}

GBool FormWidget::inRect(double x, double y) const {
  return widget ? widget->inRect(x, y) : gFalse;
}

void FormWidget::getRect(double *x1, double *y1, double *x2, double *y2) const {
  if (widget)
    widget->getRect(x1, y1, x2, y2);
}

double FormWidget::getFontSize() const {
  return widget ? widget->getFontSize() : 0.;
}

bool FormWidget::isReadOnly() const
{
  return field->isReadOnly();
}

int FormWidget::encodeID (unsigned pageNum, unsigned fieldNum)
{
  return (pageNum << 4*sizeof(unsigned)) + fieldNum;
}

void FormWidget::decodeID (unsigned id, unsigned* pageNum, unsigned* fieldNum)
{
  *pageNum = id >> 4*sizeof(unsigned);
  *fieldNum = (id << 4*sizeof(unsigned)) >> 4*sizeof(unsigned);
}

GooString *FormWidget::getPartialName() const {
  return field->getPartialName();
}

GooString *FormWidget::getAlternateUiName() const {
  return field->getAlternateUiName();
}

GooString *FormWidget::getMappingName() const {
  return field->getMappingName();
}

GooString *FormWidget::getFullyQualifiedName() {
  return field->getFullyQualifiedName();
}

LinkAction *FormWidget::getActivationAction() {
  return widget ? widget->getAction() : NULL;
}

LinkAction *FormWidget::getAdditionalAction(Annot::FormAdditionalActionsType type) {
  return widget ? widget->getFormAdditionalAction(type) : NULL;
}

FormWidgetButton::FormWidgetButton (PDFDoc *docA, Object *aobj, unsigned num, Ref ref, FormField *p) :
	FormWidget(docA, aobj, num, ref, p)
{
  type = formButton;
  parent = static_cast<FormFieldButton*>(field);
  onStr = NULL;

  Object obj1, obj2;

  // Find the name of the ON state in the AP dictionnary
  // The reference say the Off state, if it existe, _must_ be stored in the AP dict under the name /Off
  // The "on" state may be stored under any other name
  if (obj.dictLookup("AP", &obj1)->isDict()) {
    if (obj1.dictLookup("N", &obj2)->isDict()) {
      for (int i = 0; i < obj2.dictGetLength(); i++) {
        char *key = obj2.dictGetKey(i);
        if (strcmp (key, "Off") != 0) {
          onStr = new GooString (key);
          break;
        }
      }
    }
    obj2.free();
  }
  obj1.free();
}

char *FormWidgetButton::getOnStr() {
  if (onStr)
    return onStr->getCString();

  // 12.7.4.2.3 Check Boxes
  //  Yes should be used as the name for the on state
  return parent->getButtonType() == formButtonCheck ? (char *)"Yes" : NULL;
}

FormWidgetButton::~FormWidgetButton ()
{
  delete onStr;
}

FormButtonType FormWidgetButton::getButtonType () const
{
  return parent->getButtonType ();
}

void FormWidgetButton::setAppearanceState(const char *state) {
  if (!widget)
    return;
  widget->setAppearanceState(state);
}

void FormWidgetButton::updateWidgetAppearance()
{
  // The appearance stream must NOT be regenerated for this widget type
}

void FormWidgetButton::setState (GBool astate)
{
  //pushButtons don't have state
  if (parent->getButtonType() == formButtonPush)
    return;

  // Silently return if can't set ON state
  if (astate && !onStr)
    return;

  parent->setState(astate ? onStr->getCString() : (char *)"Off");
  // Parent will call setAppearanceState()
}

GBool FormWidgetButton::getState ()
{
  return onStr ? parent->getState(onStr->getCString()) : gFalse;
}


FormWidgetText::FormWidgetText (PDFDoc *docA, Object *aobj, unsigned num, Ref ref, FormField *p) :
	FormWidget(docA, aobj, num, ref, p)
{
  type = formText;
  parent = static_cast<FormFieldText*>(field);
}

GooString* FormWidgetText::getContent ()
{
  return parent->getContent(); 
}

GooString* FormWidgetText::getContentCopy ()
{
  return parent->getContentCopy();
}

void FormWidgetText::updateWidgetAppearance()
{
  if (widget)
    widget->updateAppearanceStream();
}

bool FormWidgetText::isMultiline () const 
{ 
  return parent->isMultiline(); 
}

bool FormWidgetText::isPassword () const 
{ 
  return parent->isPassword(); 
}

bool FormWidgetText::isFileSelect () const 
{ 
  return parent->isFileSelect(); 
}

bool FormWidgetText::noSpellCheck () const 
{ 
  return parent->noSpellCheck(); 
}

bool FormWidgetText::noScroll () const 
{ 
  return parent->noScroll(); 
}

bool FormWidgetText::isComb () const 
{ 
  return parent->isComb(); 
}

bool FormWidgetText::isRichText () const 
{ 
  return parent->isRichText(); 
}

int FormWidgetText::getMaxLen () const
{
  return parent->getMaxLen ();
}

void FormWidgetText::setContent(GooString* new_content)
{
  if (isReadOnly()) {
    error(errInternal, -1, "FormWidgetText::setContentCopy called on a read only field\n");
    return;
  }

  parent->setContentCopy(new_content);
}

FormWidgetChoice::FormWidgetChoice(PDFDoc *docA, Object *aobj, unsigned num, Ref ref, FormField *p) :
	FormWidget(docA, aobj, num, ref, p)
{
  type = formChoice;
  parent = static_cast<FormFieldChoice*>(field);
}

FormWidgetChoice::~FormWidgetChoice()
{
}

bool FormWidgetChoice::_checkRange (int i)
{
  if (i < 0 || i >= parent->getNumChoices()) {
    error(errInternal, -1, "FormWidgetChoice::_checkRange i out of range : {0:d}", i);
    return false;
  } 
  return true;
}

void FormWidgetChoice::select (int i)
{
  if (isReadOnly()) {
    error(errInternal, -1, "FormWidgetChoice::select called on a read only field\n");
    return;
  }
  if (!_checkRange(i)) return;
  parent->select(i);
}

void FormWidgetChoice::toggle (int i)
{
  if (isReadOnly()) {
    error(errInternal, -1, "FormWidgetChoice::toggle called on a read only field\n");
    return;
  }
  if (!_checkRange(i)) return;
  parent->toggle(i);
}

void FormWidgetChoice::deselectAll ()
{
  if (isReadOnly()) {
    error(errInternal, -1, "FormWidgetChoice::deselectAll called on a read only field\n");
    return;
  }
  parent->deselectAll();
}

GooString* FormWidgetChoice::getEditChoice ()
{
  if (!hasEdit()) {
    error(errInternal, -1, "FormFieldChoice::getEditChoice called on a non-editable choice\n");
    return NULL;
  }
  return parent->getEditChoice();
}

void FormWidgetChoice::updateWidgetAppearance()
{
  if (widget)
    widget->updateAppearanceStream();
}

bool FormWidgetChoice::isSelected (int i)
{
  if (!_checkRange(i)) return false;
  return parent->isSelected(i);
}

void FormWidgetChoice::setEditChoice (GooString* new_content)
{
  if (isReadOnly()) {
    error(errInternal, -1, "FormWidgetText::setEditChoice called on a read only field\n");
    return;
  }
  if (!hasEdit()) {
    error(errInternal, -1, "FormFieldChoice::setEditChoice : trying to edit an non-editable choice\n");
    return;
  }

  parent->setEditChoice(new_content);
}

int FormWidgetChoice::getNumChoices() 
{ 
  return parent->getNumChoices(); 
}

GooString* FormWidgetChoice::getChoice(int i) 
{ 
  return parent->getChoice(i); 
}

bool FormWidgetChoice::isCombo () const 
{ 
  return parent->isCombo(); 
}

bool FormWidgetChoice::hasEdit () const 
{ 
  return parent->hasEdit(); 
}

bool FormWidgetChoice::isMultiSelect () const 
{ 
  return parent->isMultiSelect(); 
}

bool FormWidgetChoice::noSpellCheck () const 
{ 
  return parent->noSpellCheck(); 
}

bool FormWidgetChoice::commitOnSelChange () const 
{ 
  return parent->commitOnSelChange(); 
}

bool FormWidgetChoice::isListBox () const
{
  return parent->isListBox();
}

FormWidgetSignature::FormWidgetSignature(PDFDoc *docA, Object *aobj, unsigned num, Ref ref, FormField *p) :
	FormWidget(docA, aobj, num, ref, p)
{
  type = formSignature;
  parent = static_cast<FormFieldSignature*>(field);
}

void FormWidgetSignature::updateWidgetAppearance()
{
  // Unimplemented
}


//========================================================================
// FormField
//========================================================================

FormField::FormField(PDFDoc *docA, Object *aobj, const Ref& aref, FormField *parentA, std::set<int> *usedParents, FormFieldType ty)
{
  doc = docA;
  xref = doc->getXRef();
  aobj->copy(&obj);
  Dict* dict = obj.getDict();
  ref.num = ref.gen = 0;
  type = ty;
  parent = parentA;
  numChildren = 0;
  children = NULL;
  terminal = false;
  widgets = NULL;
  readOnly = false;
  defaultAppearance = NULL;
  fullyQualifiedName = NULL;
  quadding = quaddingLeftJustified;
  hasQuadding = gFalse;

  ref = aref;

  Object obj1;
  //childs
  if (dict->lookup("Kids", &obj1)->isArray()) {
    // Load children
    for (int i = 0 ; i < obj1.arrayGetLength(); i++) {
      Object childRef, childObj;

      if (!obj1.arrayGetNF(i, &childRef)->isRef()) {
        error (errSyntaxError, -1, "Invalid form field renference");
        childRef.free();
        continue;
      }
      if (!obj1.arrayGet(i, &childObj)->isDict()) {
        error (errSyntaxError, -1, "Form field child is not a dictionary");
        childObj.free();
        childRef.free();
        continue;
      }

      const Ref ref = childRef.getRef();
      if (usedParents->find(ref.num) == usedParents->end()) {
        Object obj2, obj3;
        // Field child: it could be a form field or a widget or composed dict
        if (childObj.dictLookupNF("Parent", &obj2)->isRef() || childObj.dictLookup("Parent", &obj3)->isDict()) {
          // Child is a form field or composed dict
          // We create the field, if it's composed
          // it will create the widget as a child
          std::set<int> usedParentsAux = *usedParents;
          usedParentsAux.insert(ref.num);
          obj2.free();
          obj3.free();

          if (terminal) {
            error(errSyntaxWarning, -1, "Field can't have both Widget AND Field as kids\n");
            continue;
          }

          numChildren++;
          children = (FormField**)greallocn(children, numChildren, sizeof(FormField*));
          children[numChildren - 1] = Form::createFieldFromDict(&childObj, doc, ref, this, &usedParentsAux);
        } else if (childObj.dictLookup("Subtype", &obj2)->isName("Widget")) {
          // Child is a widget annotation
          if (!terminal && numChildren > 0) {
            error(errSyntaxWarning, -1, "Field can't have both Widget AND Field as kids\n");
            obj2.free();
            obj3.free();
            continue;
          }
          _createWidget(&childObj, ref);
        }
        obj2.free();
        obj3.free();
      }
      childObj.free();
      childRef.free();
    }
  } else {
    // No children, if it's a composed dict, create the child widget
    obj1.free();
    if (dict->lookup("Subtype", &obj1)->isName("Widget"))
      _createWidget(&obj, ref);
  }
  obj1.free();

  //flags
  if (Form::fieldLookup(dict, "Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    if (flags & 0x1) { // 1 -> ReadOnly
      readOnly = true;
    }
    if (flags & 0x2) { // 2 -> Required
      //TODO
    }
    if (flags & 0x4) { // 3 -> NoExport
      //TODO
    }
  }
  obj1.free();

  // Variable Text
  if (Form::fieldLookup(dict, "DA", &obj1)->isString())
    defaultAppearance = obj1.getString()->copy();
  obj1.free();

  if (Form::fieldLookup(dict, "Q", &obj1)->isInt()) {
    quadding = static_cast<VariableTextQuadding>(obj1.getInt());
    hasQuadding = gTrue;
  }
  obj1.free();

  if (dict->lookup("T", &obj1)->isString()) {
    partialName = obj1.getString()->copy();
  } else {
    partialName = NULL;
  }
  obj1.free();

  if (dict->lookup("TU", &obj1)->isString()) {
    alternateUiName = obj1.getString()->copy();
  } else {
    alternateUiName = NULL;
  }
  obj1.free();

  if(dict->lookup("TM", &obj1)->isString()) {
    mappingName = obj1.getString()->copy();
  } else {
    mappingName = NULL;
  }
  obj1.free();
}

FormField::~FormField()
{
  if (!terminal) {
    if(children) {
      for (int i=0; i<numChildren; i++)
        delete children[i];
      gfree(children);
    }
  } else {
    for (int i = 0; i < numChildren; ++i)
      delete widgets[i];
    gfree (widgets);
  }
  obj.free();

  delete defaultAppearance;
  delete partialName;
  delete alternateUiName;
  delete mappingName;
  delete fullyQualifiedName;
}

#ifdef DEBUG_FORMS
void FormField::print(int indent)
{
  printf ("%*s- (%d %d): [container] terminal: %s children: %d\n", indent, "", ref.num, ref.gen,
          terminal ? "Yes" : "No", numChildren);
}

void FormField::printTree(int indent)
{
  print(indent);
  if (terminal) {
    for (int i = 0; i < numChildren; i++)
      widgets[i]->print(indent + 4);
  } else {
    for (int i = 0; i < numChildren; i++)
      children[i]->printTree(indent + 4);
  }
}
#endif

void FormField::fillChildrenSiblingsID()
{
  if (terminal)
    return;
  for (int i = 0; i < numChildren; i++) {
    children[i]->fillChildrenSiblingsID();
  }
}

void FormField::createWidgetAnnotations() {
  if (terminal) {
    for (int i = 0; i < numChildren; i++)
      widgets[i]->createWidgetAnnotation();
  } else {
    for (int i = 0; i < numChildren; i++)
      children[i]->createWidgetAnnotations();
  }
}

void FormField::_createWidget (Object *obj, Ref aref)
{
  terminal = true;
  numChildren++;
  widgets = (FormWidget**)greallocn(widgets, numChildren, sizeof(FormWidget*));
  //ID = index in "widgets" table
  switch (type) {
  case formButton:
    widgets[numChildren-1] = new FormWidgetButton(doc, obj, numChildren-1, aref, this);
    break;
  case formText:
    widgets[numChildren-1] = new FormWidgetText(doc, obj, numChildren-1, aref, this);
    break;
  case formChoice:
    widgets[numChildren-1] = new FormWidgetChoice(doc, obj, numChildren-1, aref, this);
    break;
  case formSignature:
    widgets[numChildren-1] = new FormWidgetSignature(doc, obj, numChildren-1, aref, this);
    break;
  default:
    error(errSyntaxWarning, -1, "SubType on non-terminal field, invalid document?");
    numChildren--;
    terminal = false;
  }
}

FormWidget* FormField::findWidgetByRef (Ref aref)
{
  if (terminal) {
    for(int i=0; i<numChildren; i++) {
      if (widgets[i]->getRef().num == aref.num 
          && widgets[i]->getRef().gen == aref.gen)
        return widgets[i];
    }
  } else {
    for(int i=0; i<numChildren; i++) {
      FormWidget* result = children[i]->findWidgetByRef(aref);
      if(result) return result;
    }
  }
  return NULL;
}

GooString* FormField::getFullyQualifiedName() {
  Object obj1, obj2;
  Object parent;
  GooString *parent_name;
  GooString *full_name;
  GBool unicode_encoded = gFalse;

  if (fullyQualifiedName)
    return fullyQualifiedName;

  full_name = new GooString();

  obj.copy(&obj1);
  while (obj1.dictLookup("Parent", &parent)->isDict()) {
    if (parent.dictLookup("T", &obj2)->isString()) {
      parent_name = obj2.getString();

      if (unicode_encoded) {
        full_name->insert(0, "\0.", 2); // 2-byte unicode period
        if (parent_name->hasUnicodeMarker()) {
          full_name->insert(0, parent_name->getCString() + 2, parent_name->getLength() - 2); // Remove the unicode BOM
        } else {
          int tmp_length;
          char* tmp_str = pdfDocEncodingToUTF16(parent_name, &tmp_length);
          full_name->insert(0, tmp_str + 2, tmp_length - 2); // Remove the unicode BOM
          delete [] tmp_str;
        }
      } else {
        full_name->insert(0, '.'); // 1-byte ascii period
        if (parent_name->hasUnicodeMarker()) {          
          unicode_encoded = gTrue;
          full_name = convertToUtf16(full_name);
          full_name->insert(0, parent_name->getCString() + 2, parent_name->getLength() - 2); // Remove the unicode BOM
        } else {
          full_name->insert(0, parent_name);
        }
      }
      obj2.free();
    }
    obj1.free();
    parent.copy(&obj1);
    parent.free();
  }
  obj1.free();
  parent.free();

  if (partialName) {
    if (unicode_encoded) {
      if (partialName->hasUnicodeMarker()) {
        full_name->append(partialName->getCString() + 2, partialName->getLength() - 2); // Remove the unicode BOM
      } else {
        int tmp_length;
        char* tmp_str = pdfDocEncodingToUTF16(partialName, &tmp_length);
        full_name->append(tmp_str + 2, tmp_length - 2); // Remove the unicode BOM
        delete [] tmp_str;
      }
    } else {
      if (partialName->hasUnicodeMarker()) {
          unicode_encoded = gTrue;        
          full_name = convertToUtf16(full_name);
          full_name->append(partialName->getCString() + 2, partialName->getLength() - 2); // Remove the unicode BOM
      } else {
        full_name->append(partialName);
      }
    }
  } else {
    int len = full_name->getLength();
    // Remove the last period
    if (unicode_encoded) {
      if (len > 1) {
        full_name->del(len - 2, 2);
      }
    } else {
      if (len > 0) {
        full_name->del(len - 1, 1);
      }
    }
  }
  
  if (unicode_encoded) {
    full_name->insert(0, 0xff);
    full_name->insert(0, 0xfe);
  }

  fullyQualifiedName = full_name;
  return fullyQualifiedName;
}

void FormField::updateChildrenAppearance()
{
  // Recursively update each child's appearance
  if (terminal) {
    for (int i = 0; i < numChildren; i++)
      widgets[i]->updateWidgetAppearance();
  } else {
    for (int i = 0; i < numChildren; i++)
      children[i]->updateChildrenAppearance();
  }
}

//------------------------------------------------------------------------
// FormFieldButton
//------------------------------------------------------------------------
FormFieldButton::FormFieldButton(PDFDoc *docA, Object *aobj, const Ref& ref, FormField *parent, std::set<int> *usedParents)
  : FormField(docA, aobj, ref, parent, usedParents, formButton)
{
  Dict* dict = obj.getDict();
  active_child = -1;
  noAllOff = false;
  siblings = NULL;
  numSiblings = 0;
  appearanceState.initNull();

  Object obj1;
  btype = formButtonCheck; 
  if (Form::fieldLookup(dict, "Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    
    if (flags & 0x10000) { // 17 -> push button
      btype = formButtonPush;
    } else if (flags & 0x8000) { // 16 -> radio button
      btype = formButtonRadio;
      if (flags & 0x4000) { // 15 -> noToggleToOff
        noAllOff = true;
      }
    } 
    if (flags & 0x1000000) { // 26 -> radiosInUnison
      error(errUnimplemented, -1, "FormFieldButton:: radiosInUnison flag unimplemented, please report a bug with a testcase\n");
    } 
  }

  if (btype != formButtonPush) {
    // Even though V is inheritable we are interested in the value of this
    // field, if not present it's probably because it's a button in a set.
    dict->lookup("V", &appearanceState);
  }
}

#ifdef DEBUG_FORMS
static char *_getButtonType(FormButtonType type)
{
  switch (type) {
  case formButtonPush:
    return "push";
  case formButtonCheck:
    return "check";
  case formButtonRadio:
    return "radio";
  default:
    break;
  }
  return "unknown";
}

void FormFieldButton::print(int indent)
{
  printf ("%*s- (%d %d): [%s] terminal: %s children: %d\n", indent, "", ref.num, ref.gen,
          _getButtonType(btype), terminal ? "Yes" : "No", numChildren);
}
#endif

void FormFieldButton::setNumSiblings (int num)
{ 
  numSiblings = num; 
  siblings = (FormFieldButton**)greallocn(siblings, numSiblings, sizeof(FormFieldButton*));
}

void FormFieldButton::fillChildrenSiblingsID()
{
  if (!terminal) {
    for(int i=0; i<numChildren; i++) {
      FormFieldButton *child = dynamic_cast<FormFieldButton*>(children[i]);
      if (child != NULL) {
        // Fill the siblings of this node childs
        child->setNumSiblings(numChildren-1);
        for(int j=0, counter=0; j<numChildren; j++) {
          FormFieldButton *otherChild = dynamic_cast<FormFieldButton*>(children[j]);
          if (i == j) continue;
          if (child == otherChild) continue;
          child->setSibling(counter, otherChild);
          counter++;
        }

        // now call ourselves on the child
        // to fill its children data
        child->fillChildrenSiblingsID();
      }
    }
  }
}

GBool FormFieldButton::setState(char *state)
{
  if (readOnly) {
    error(errInternal, -1, "FormFieldButton::setState called on a readOnly field\n");
    return gFalse;
  }

  // A check button could behave as a radio button
  // when it's in a set of more than 1 buttons
  if (btype != formButtonRadio && btype != formButtonCheck)
    return gFalse;

  if (terminal && parent && parent->getType() == formButton && appearanceState.isNull()) {
    // It's button in a set, set state on parent
    if (static_cast<FormFieldButton*>(parent)->setState(state)) {
      return gTrue;
    }
    return gFalse;
  }

  GBool isOn = strcmp(state, "Off") != 0;

  if (!isOn && noAllOff)
    return gFalse; // Don't allow to set all radio to off

  char *current = getAppearanceState();
  GBool currentFound = gFalse, newFound = gFalse;

  for (int i = 0; i < numChildren; i++) {
    FormWidgetButton *widget;

    // If radio button is a terminal field we want the widget at i, but
    // if it's not terminal, the child widget is a composed dict, so
    // we want the ony child widget of the children at i
    if (terminal)
      widget = static_cast<FormWidgetButton*>(widgets[i]);
    else
      widget = static_cast<FormWidgetButton*>(children[i]->getWidget(0));

    if (!widget->getOnStr())
      continue;

    char *onStr = widget->getOnStr();
    if (current && strcmp(current, onStr) == 0) {
      widget->setAppearanceState("Off");
      if (!isOn)
        break;
      currentFound = gTrue;
    }

    if (isOn && strcmp(state, onStr) == 0) {
      widget->setAppearanceState(state);
      newFound = gTrue;
    }

    if (currentFound && newFound)
      break;
  }

  updateState(state);

  return gTrue;
}

GBool FormFieldButton::getState(char *state) {
  if (appearanceState.isName(state))
    return gTrue;

  return (parent && parent->getType() == formButton) ? static_cast<FormFieldButton*>(parent)->getState(state) : gFalse;
}

void FormFieldButton::updateState(char *state) {
  Object obj1;

  appearanceState.free();
  appearanceState.initName(state);

  appearanceState.copy(&obj1);
  obj.getDict()->set("V", &obj1);
  xref->setModifiedObject(&obj, ref);
}

FormFieldButton::~FormFieldButton()
{
  appearanceState.free();
  if (siblings)
    gfree(siblings);
}

//------------------------------------------------------------------------
// FormFieldText
//------------------------------------------------------------------------
FormFieldText::FormFieldText(PDFDoc *docA, Object *aobj, const Ref& ref, FormField *parent, std::set<int> *usedParents)
  : FormField(docA, aobj, ref, parent, usedParents, formText)
{
  Dict* dict = obj.getDict();
  Object obj1;
  content = NULL;
  multiline = password = fileSelect = doNotSpellCheck = doNotScroll = comb = richText = false;
  maxLen = 0;

  if (Form::fieldLookup(dict, "Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    if (flags & 0x1000) // 13 -> Multiline
      multiline = true;
    if (flags & 0x2000) // 14 -> Password
      password = true;
    if (flags & 0x100000) // 21 -> FileSelect
      fileSelect = true;
    if (flags & 0x400000)// 23 -> DoNotSpellCheck
      doNotSpellCheck = true;
    if (flags & 0x800000) // 24 -> DoNotScroll
      doNotScroll = true;
    if (flags & 0x1000000) // 25 -> Comb
      comb = true;
    if (flags & 0x2000000)// 26 -> RichText
      richText = true;
  }
  obj1.free();

  if (Form::fieldLookup(dict, "MaxLen", &obj1)->isInt()) {
    maxLen = obj1.getInt();
  }
  obj1.free();

  if (Form::fieldLookup(dict, "V", &obj1)->isString()) {
    if (obj1.getString()->hasUnicodeMarker()) {
      if (obj1.getString()->getLength() > 2)
        content = obj1.getString()->copy();
    } else if (obj1.getString()->getLength() > 0) {
      //non-unicode string -- assume pdfDocEncoding and try to convert to UTF16BE
      int tmp_length;
      char* tmp_str = pdfDocEncodingToUTF16(obj1.getString(), &tmp_length);
      content = new GooString(tmp_str, tmp_length);
      delete [] tmp_str;
    }
  }
  obj1.free();
}

#ifdef DEBUG_FORMS
void FormFieldText::print(int indent)
{
  printf ("%*s- (%d %d): [text] terminal: %s children: %d\n", indent, "", ref.num, ref.gen,
          terminal ? "Yes" : "No", numChildren);
}
#endif

GooString* FormFieldText::getContentCopy ()
{
  if (!content) return NULL;
  return new GooString(content);
}

void FormFieldText::setContentCopy (GooString* new_content)
{
  delete content;
  content = NULL;

  if (new_content) {
    content = new_content->copy();

    //append the unicode marker <FE FF> if needed
    if (!content->hasUnicodeMarker()) {
      content->insert(0, 0xff);
      content->insert(0, 0xfe);
    }
  }

  Object obj1;
  obj1.initString(content ? content->copy() : new GooString(""));
  obj.getDict()->set("V", &obj1);
  xref->setModifiedObject(&obj, ref);
  updateChildrenAppearance();
}

FormFieldText::~FormFieldText()
{
  delete content;
}


//------------------------------------------------------------------------
// FormFieldChoice
//------------------------------------------------------------------------
FormFieldChoice::FormFieldChoice(PDFDoc *docA, Object *aobj, const Ref& ref, FormField *parent, std::set<int> *usedParents)
  : FormField(docA, aobj, ref, parent, usedParents, formChoice)
{
  numChoices = 0;
  choices = NULL;
  editedChoice = NULL;
  topIdx = 0;

  Dict* dict = obj.getDict();
  Object obj1;

  combo = edit = multiselect = doNotSpellCheck = doCommitOnSelChange = false;

  if (Form::fieldLookup(dict, "Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    if (flags & 0x20000) // 18 -> Combo
      combo = true; 
    if (flags & 0x40000) // 19 -> Edit
      edit = true;
    if (flags & 0x200000) // 22 -> MultiSelect
      multiselect = true;
    if (flags & 0x400000) // 23 -> DoNotSpellCheck
      doNotSpellCheck = true;
    if (flags & 0x4000000) // 27 -> CommitOnSelChange
      doCommitOnSelChange = true;
  }
  obj1.free();

  if (dict->lookup("TI", &obj1)->isInt())
    topIdx = obj1.getInt();
  obj1.free();

  if (dict->lookup("Opt", &obj1)->isArray()) {
    Object obj2;

    numChoices = obj1.arrayGetLength();
    choices = new ChoiceOpt[numChoices];
    memset(choices, 0, sizeof(ChoiceOpt) * numChoices);

    for (int i = 0; i < numChoices; i++) {
      if (obj1.arrayGet(i, &obj2)->isString()) {
        choices[i].optionName = obj2.getString()->copy();
      } else if (obj2.isArray()) { // [Export_value, Displayed_text]
        Object obj3;

        if (obj2.arrayGetLength() < 2) {
          error(errSyntaxError, -1, "FormWidgetChoice:: invalid Opt entry -- array's length < 2\n");
          continue;
        }
        if (obj2.arrayGet(0, &obj3)->isString())
          choices[i].exportVal = obj3.getString()->copy();
        else
          error(errSyntaxError, -1, "FormWidgetChoice:: invalid Opt entry -- exported value not a string\n");
        obj3.free();

        if (obj2.arrayGet(1, &obj3)->isString())
          choices[i].optionName = obj3.getString()->copy();
        else
          error(errSyntaxError, -1, "FormWidgetChoice:: invalid Opt entry -- choice name not a string\n");
        obj3.free();
      } else {
        error(errSyntaxError, -1, "FormWidgetChoice:: invalid {0:d} Opt entry\n", i);
      }
      obj2.free();
    }
  } else {
    //empty choice
  }
  obj1.free();

  // Find selected items
  // Note: PDF specs say that /V has precedence over /I, but acroread seems to
  // do the opposite. We do the same.
  if (Form::fieldLookup(dict, "I", &obj1)->isArray()) {
    for (int i = 0; i < obj1.arrayGetLength(); i++) {
      Object obj2;
      if (obj1.arrayGet(i, &obj2)->isInt() && obj2.getInt() >= 0 && obj2.getInt() < numChoices) {
        choices[obj2.getInt()].selected = true;
      }
      obj2.free();
    }
  } else {
    obj1.free();
    // Note: According to PDF specs, /V should *never* contain the exportVal.
    // However, if /Opt is an array of (exportVal,optionName) pairs, acroread
    // seems to expect the exportVal instead of the optionName and so we do too.
    if (Form::fieldLookup(dict, "V", &obj1)->isString()) {
      GBool optionFound = gFalse;

      for (int i = 0; i < numChoices; i++) {
        if (choices[i].exportVal) {
          if (choices[i].exportVal->cmp(obj1.getString()) == 0) {
            optionFound = gTrue;
          }
        } else if (choices[i].optionName) {
          if (choices[i].optionName->cmp(obj1.getString()) == 0) {
            optionFound = gTrue;
          }
        }

        if (optionFound) {
          choices[i].selected = true;
          break; // We've determined that this option is selected. No need to keep on scanning
        }
      }

      // Set custom value if /V doesn't refer to any predefined option and the field is user-editable
      if (!optionFound && edit) {
        editedChoice = obj1.getString()->copy();
      }
    } else if (obj1.isArray()) {
      for (int i = 0; i < numChoices; i++) {
        for (int j = 0; j < obj1.arrayGetLength(); j++) {
          Object obj2;
          obj1.arrayGet(j, &obj2);
          GBool matches = gFalse;

          if (choices[i].exportVal) {
            if (choices[i].exportVal->cmp(obj2.getString()) == 0) {
              matches = gTrue;
            }
          } else if (choices[i].optionName) {
            if (choices[i].optionName->cmp(obj2.getString()) == 0) {
              matches = gTrue;
            }
          }

          obj2.free();

          if (matches) {
            choices[i].selected = true;
            break; // We've determined that this option is selected. No need to keep on scanning
          }
        }
      }
    }
  }
  obj1.free();
}

FormFieldChoice::~FormFieldChoice()
{
  for (int i = 0; i < numChoices; i++) {
    delete choices[i].exportVal;
    delete choices[i].optionName;
  }
  delete [] choices;
  delete editedChoice;
}

#ifdef DEBUG_FORMS
void FormFieldChoice::print(int indent)
{
  printf ("%*s- (%d %d): [choice] terminal: %s children: %d\n", indent, "", ref.num, ref.gen,
          terminal ? "Yes" : "No", numChildren);
}
#endif

void FormFieldChoice::updateSelection() {
  Object objV, objI, obj1;
  objI.initNull();

  if (edit && editedChoice) {
    // This is an editable combo-box with user-entered text
    objV.initString(editedChoice->copy());
  } else {
    const int numSelected = getNumSelected();

    // Create /I array only if multiple selection is allowed (as per PDF spec)
    if (multiselect) {
      objI.initArray(xref);
    }

    if (numSelected == 0) {
      // No options are selected
      objV.initString(new GooString(""));
    } else if (numSelected == 1) {
      // Only one option is selected
      for (int i = 0; i < numChoices; i++) {
        if (choices[i].selected) {
          if (multiselect) {
            objI.arrayAdd(obj1.initInt(i));
          }

          if (choices[i].exportVal) {
            objV.initString(choices[i].exportVal->copy());
          } else if (choices[i].optionName) {
            objV.initString(choices[i].optionName->copy());
          }

          break; // We've just written the selected option. No need to keep on scanning
        }
      }
    } else {
      // More than one option is selected
      objV.initArray(xref);
      for (int i = 0; i < numChoices; i++) {
        if (choices[i].selected) {
          if (multiselect) {
            objI.arrayAdd(obj1.initInt(i));
          }

          if (choices[i].exportVal) {
            objV.arrayAdd(obj1.initString(choices[i].exportVal->copy()));
          } else if (choices[i].optionName) {
            objV.arrayAdd(obj1.initString(choices[i].optionName->copy()));
          }
        }
      }
    }
  }

  obj.getDict()->set("V", &objV);
  obj.getDict()->set("I", &objI);
  xref->setModifiedObject(&obj, ref);
  updateChildrenAppearance();
}

void FormFieldChoice::unselectAll ()
{
  for (int i = 0; i < numChoices; i++) {
    choices[i].selected = false;
  }
}

void FormFieldChoice::deselectAll () {
  delete editedChoice;
  editedChoice = NULL;

  unselectAll();
  updateSelection();
}

void FormFieldChoice::toggle (int i)
{
  delete editedChoice;
  editedChoice = NULL;

  choices[i].selected = !choices[i].selected;
  updateSelection();
}

void FormFieldChoice::select (int i)
{
  delete editedChoice;
  editedChoice = NULL;

  if (!multiselect)
    unselectAll();

  choices[i].selected = true;
  updateSelection();
}

void FormFieldChoice::setEditChoice (GooString* new_content)
{
  delete editedChoice;
  editedChoice = NULL;

  unselectAll();

  if (new_content) {
    editedChoice = new_content->copy();

    //append the unicode marker <FE FF> if needed
    if (!editedChoice->hasUnicodeMarker()) {
      editedChoice->insert(0, 0xff);
      editedChoice->insert(0, 0xfe);
    }
  }
  updateSelection();
}

GooString* FormFieldChoice::getEditChoice ()
{
  return editedChoice;
}

int FormFieldChoice::getNumSelected ()
{
  int cnt = 0;
  for(int i=0; i<numChoices; i++) {
    if (choices[i].selected)
      cnt++;
  }
  return cnt;
}

GooString *FormFieldChoice::getSelectedChoice() {
  if (edit && editedChoice)
    return editedChoice;

  for (int i = 0; i < numChoices; i++) {
    if (choices[i].optionName && choices[i].selected)
      return choices[i].optionName;
  }

  return NULL;
}

//------------------------------------------------------------------------
// FormFieldSignature
//------------------------------------------------------------------------
FormFieldSignature::FormFieldSignature(PDFDoc *docA, Object *dict, const Ref& ref, FormField *parent, std::set<int> *usedParents)
  : FormField(docA, dict, ref, parent, usedParents, formSignature)
{
}

FormFieldSignature::~FormFieldSignature()
{

}

#ifdef DEBUG_FORMS
void FormFieldSignature::print(int indent)
{
  printf ("%*s- (%d %d): [signature] terminal: %s children: %d\n", indent, "", ref.num, ref.gen,
          terminal ? "Yes" : "No", numChildren);
}
#endif

//------------------------------------------------------------------------
// Form
//------------------------------------------------------------------------

Form::Form(PDFDoc *docA, Object* acroFormA)
{
  Object obj1;

  doc = docA;
  xref = doc->getXRef();
  acroForm = acroFormA;
  
  size = 0;
  numFields = 0;
  rootFields = NULL;
  quadding = quaddingLeftJustified;
  defaultAppearance = NULL;
  defaultResources = NULL;

  acroForm->dictLookup("NeedAppearances", &obj1);
  needAppearances = (obj1.isBool() && obj1.getBool());
  obj1.free();

  if (acroForm->dictLookup("DA", &obj1)->isString())
    defaultAppearance = obj1.getString()->copy();
  obj1.free();

  if (acroForm->dictLookup("Q", &obj1)->isInt())
    quadding = static_cast<VariableTextQuadding>(obj1.getInt());
  obj1.free();

  acroForm->dictLookup("DR", &resDict);
  if (resDict.isDict()) {
    // At a minimum, this dictionary shall contain a Font entry
    if (resDict.dictLookup("Font", &obj1)->isDict())
      defaultResources = new GfxResources(xref, resDict.getDict(), NULL);
    obj1.free();
  }
  if (!defaultResources) {
    resDict.free();
    resDict.initNull();
  }

  acroForm->dictLookup("Fields", &obj1);
  if (obj1.isArray()) {
    Array *array = obj1.getArray();
    Object obj2;
    
    for(int i=0; i<array->getLength(); i++) {
      Object oref;
      array->get(i, &obj2);
      array->getNF(i, &oref);
      if (!oref.isRef()) {
        error(errSyntaxWarning, -1, "Direct object in rootFields");
	obj2.free();
	oref.free();
        continue;
      }

      if (!obj2.isDict()) {
        error(errSyntaxWarning, -1, "Reference in Fields array to an invalid or non existant object");
	obj2.free();
	oref.free();
	continue;
      }

      if (numFields >= size) {
        size += 16;
        rootFields = (FormField**)greallocn(rootFields,size,sizeof(FormField*));
      }

      std::set<int> usedParents;
      rootFields[numFields++] = createFieldFromDict (&obj2, doc, oref.getRef(), NULL, &usedParents);

      obj2.free();
      oref.free();
    }
  } else {
    error(errSyntaxError, -1, "Can't get Fields array\n");
  }
  obj1.free ();

#ifdef DEBUG_FORMS
  for (int i = 0; i < numFields; i++)
    rootFields[i]->printTree();
#endif
}

Form::~Form() {
  int i;
  for(i = 0; i< numFields; ++i)
    delete rootFields[i];
  gfree (rootFields);
  delete defaultAppearance;
  delete defaultResources;
  resDict.free();
}

// Look up an inheritable field dictionary entry.
static Object *fieldLookup(Dict *field, const char *key, Object *obj, std::set<int> *usedParents) {
  Dict *dict;
  Object parent;

  dict = field;
  if (!dict->lookup(key, obj)->isNull()) {
    return obj;
  }
  obj->free();
  dict->lookupNF("Parent", &parent);
  if (parent.isRef()) {
    const Ref ref = parent.getRef();
    if (usedParents->find(ref.num) == usedParents->end()) {
      usedParents->insert(ref.num);

      Object obj2;
      parent.fetch(dict->getXRef(), &obj2);
      if (obj2.isDict()) {
        fieldLookup(obj2.getDict(), key, obj, usedParents);
      } else {
        obj->initNull();
      }
      obj2.free();
    }
  } else if (parent.isDict()) {
    fieldLookup(parent.getDict(), key, obj, usedParents);
  } else {
    obj->initNull();
  }
  parent.free();
  return obj;
}

Object *Form::fieldLookup(Dict *field, const char *key, Object *obj) {
  std::set<int> usedParents;
  return ::fieldLookup(field, key, obj, &usedParents);
}

FormField *Form::createFieldFromDict (Object* obj, PDFDoc *docA, const Ref& pref, FormField *parent, std::set<int> *usedParents)
{
    Object obj2;
    FormField *field;

    if (Form::fieldLookup(obj->getDict (), "FT", &obj2)->isName("Btn")) {
      field = new FormFieldButton(docA, obj, pref, parent, usedParents);
    } else if (obj2.isName("Tx")) {
      field = new FormFieldText(docA, obj, pref, parent, usedParents);
    } else if (obj2.isName("Ch")) {
      field = new FormFieldChoice(docA, obj, pref, parent, usedParents);
    } else if (obj2.isName("Sig")) {
      field = new FormFieldSignature(docA, obj, pref, parent, usedParents);
    } else { //we don't have an FT entry => non-terminal field
      field = new FormField(docA, obj, pref, parent, usedParents);
    }
    obj2.free();

    return field;
}

void Form::postWidgetsLoad()
{
  // We create the widget annotations associated to
  // every form widget here, because the AnnotWidget constructor
  // needs the form object that gets from the catalog. When constructing
  // a FormWidget the Catalog is still creating the form object
  for (int i = 0; i < numFields; i++) {
    rootFields[i]->fillChildrenSiblingsID();
    rootFields[i]->createWidgetAnnotations();
  }
}

FormWidget* Form::findWidgetByRef (Ref aref)
{
  for(int i=0; i<numFields; i++) {
    FormWidget *result = rootFields[i]->findWidgetByRef(aref);
    if(result) return result;
  }
  return NULL;
}

//------------------------------------------------------------------------
// FormPageWidgets
//------------------------------------------------------------------------

FormPageWidgets::FormPageWidgets (Annots *annots, unsigned int page, Form *form)
{
  numWidgets = 0;
  widgets = NULL;

  if (annots && annots->getNumAnnots() > 0 && form) {
    size = annots->getNumAnnots();
    widgets = (FormWidget**)greallocn(widgets, size, sizeof(FormWidget*));

    /* For each entry in the page 'Annots' dict, try to find
       a matching form field */
    for (int i = 0; i < size; ++i) {
      Annot *annot = annots->getAnnot(i);

      if (annot->getType() != Annot::typeWidget)
        continue;

      if (!annot->getHasRef()) {
        /* Since all entry in a form field's kid dict needs to be
           indirect references, if this annot isn't indirect, it isn't 
           related to a form field */
        continue;
      }

      Ref r = annot->getRef();

      /* Try to find a form field which either has this Annot in its Kids entry
          or  is merged with this Annot */
      FormWidget* tmp = form->findWidgetByRef(r);
      if (tmp) {
        // We've found a corresponding form field, link it
        tmp->setID(FormWidget::encodeID(page, numWidgets));
        widgets[numWidgets++] = tmp;
      }
    }
  }
}

FormPageWidgets::~FormPageWidgets()
{
  gfree (widgets);
}
