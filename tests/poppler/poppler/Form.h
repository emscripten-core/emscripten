//========================================================================
//
// Form.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2006 Julien Rebetez <julienr@svn.gnome.org>
// Copyright 2007, 2008 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2007-2010 Albert Astals Cid <aacid@kde.org>
// Copyright 2010 Mark Riedesel <mark@klowner.com>
//
//========================================================================

#ifndef FORM_H
#define FORM_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"

#include <set>

class GooString;
class Array;
class Dict;
class Annot;
class Catalog;

enum FormFieldType {
  formButton,
  formText,
  formChoice,
  formSignature,
  formUndef
};

enum FormButtonType {
  formButtonCheck,
  formButtonPush,
  formButtonRadio
};

class Form;
class FormField;
class FormFieldButton;
class FormFieldText;
class FormFieldSignature;
class FormFieldChoice;

//------------------------------------------------------------------------
// FormWidget
// A FormWidget represents the graphical part of a field and is "attached"
// to a page.
//------------------------------------------------------------------------

class FormWidget {
public:
  virtual ~FormWidget();

  // see the description of FormField::LoadChildrenDefaults
  virtual void loadDefaults () {}
  
  // Check if point is inside the field bounding rect
  GBool inRect(double x, double y)
    { return x1 <= x && x <= x2 && y1 <= y && y <= y2; }

  // Get the field bounding rect
  void getRect(double *xa1, double *ya1, double *xa2, double *ya2)
    { *xa1 = x1; *ya1 = y1; *xa2 = x2; *ya2 = y2; }

  unsigned getID () { return ID; }
  void setID (unsigned int i) { ID=i; }

  FormField *getField () { return field; }
  FormFieldType getType() { return type; }

  Object* getObj() { return &obj; }
  Ref getRef() { return ref; }

  void setChildNum (unsigned i) { childNum = i; }
  unsigned getChildNum () { return childNum; }

  void setFontSize(double f) { fontSize = f; }
  double getFontSize () { return fontSize; }

  GooString *getPartialName() const { return partialName; }
  GooString *getMappingName() const { return mappingName; }
  GooString *getFullyQualifiedName();

  GBool isModified () { return modified; }

  bool isReadOnly() const;

  // return the unique ID corresponding to pageNum/fieldNum
  static int encodeID (unsigned pageNum, unsigned fieldNum);
  // decode id and retrieve pageNum and fieldNum
  static void decodeID (unsigned id, unsigned* pageNum, unsigned* fieldNum);

protected:
  FormWidget(XRef *xrefA, Object *aobj, unsigned num, Ref aref, FormField *fieldA);

  void updateField (const char *key, Object *value);

  FormField* field;
  FormFieldType type;
  Object obj;
  Ref ref;
  XRef *xref;
  GBool defaultsLoaded;
  GBool modified;
  GooString *partialName; // T field
  GooString *mappingName; // TM field
  GooString *fullyQualifiedName;

  //index of this field in the parent's child list
  unsigned childNum;

  /*
  Field ID is an (unsigned) integer, calculated as follow :
  the first sizeof/2 bits are the field number, relative to the page
  the last sizeof/2 bits are the page number
  [page number | field number]
  (encoding) id = (pageNum << 4*sizeof(unsigned)) + fieldNum;
  (decoding) pageNum = id >> 4*sizeof(unsigned); fieldNum = (id << 4*sizeof(unsigned)) >> 4*sizeof(unsigned);
  */
  unsigned ID; 

  double x1, y1;                // lower left corner
  double x2, y2;                // upper right corner
  double fontSize; //font size if this widget has text

};

//------------------------------------------------------------------------
// FormWidgetButton
//------------------------------------------------------------------------

class FormWidgetButton: public FormWidget {
public:
  FormWidgetButton(XRef *xrefA, Object *dict, unsigned num, Ref ref, FormField *p);
  ~FormWidgetButton ();

  FormButtonType getButtonType() const;
  
  void setState (GBool state, GBool calledByParent=gFalse);
  GBool getState ();

  char* getOnStr () { return onStr->getCString(); }

  void loadDefaults();

  void setNumSiblingsID (int i);
  void setSiblingsID (int i, unsigned id) { siblingsID[i] = id; }

  //For radio buttons, return the IDs of the other radio buttons in the same group
  unsigned* getSiblingsID () const { return siblingsID; }
  int getNumSiblingsID () const { return numSiblingsID; }

protected:
  unsigned* siblingsID; // IDs of dependent buttons (each button of a radio field has all the others buttons
                        // of the same field in this array)
  int numSiblingsID;
  GooString *onStr;
  FormFieldButton *parent;
  GBool state;
};

//------------------------------------------------------------------------
// FormWidgetText
//------------------------------------------------------------------------

class FormWidgetText: public FormWidget {
public:
  FormWidgetText(XRef *xrefA, Object *dict, unsigned num, Ref ref, FormField *p);
  //return the field's content (UTF16BE)
  GooString* getContent() ;
  //return a copy of the field's content (UTF16BE)
  GooString* getContentCopy();

  //except a UTF16BE string
  void setContent(GooString* new_content);

  void loadDefaults ();

  bool isMultiline () const; 
  bool isPassword () const; 
  bool isFileSelect () const; 
  bool noSpellCheck () const; 
  bool noScroll () const; 
  bool isComb () const; 
  bool isRichText () const;
  int getMaxLen () const;
protected:
  FormFieldText *parent;
};

//------------------------------------------------------------------------
// FormWidgetChoice
//------------------------------------------------------------------------

class FormWidgetChoice: public FormWidget {
public:
  FormWidgetChoice(XRef *xrefA, Object *dict, unsigned num, Ref ref, FormField *p);
  ~FormWidgetChoice();

  void loadDefaults ();
  int getNumChoices();
  //return the display name of the i-th choice (UTF16BE)
  GooString* getChoice(int i);
  //select the i-th choice
  void select (int i); 

  //toggle selection of the i-th choice
  void toggle (int i);

  //deselect everything
  void deselectAll ();

  //except a UTF16BE string
  //only work for editable combo box, set the user-entered text as the current choice
  void setEditChoice(GooString* new_content);

  GooString* getEditChoice ();

  bool isSelected (int i);

  bool isCombo () const; 
  bool hasEdit () const; 
  bool isMultiSelect () const; 
  bool noSpellCheck () const; 
  bool commitOnSelChange () const; 
  bool isListBox () const;
protected:
  void _updateV ();
  bool _checkRange (int i);
  FormFieldChoice *parent;
};

//------------------------------------------------------------------------
// FormWidgetSignature
//------------------------------------------------------------------------

class FormWidgetSignature: public FormWidget {
public:
  FormWidgetSignature(XRef *xrefA, Object *dict, unsigned num, Ref ref, FormField *p);
protected:
  FormFieldSignature *parent;
};

//------------------------------------------------------------------------
// FormField
// A FormField implements the logical side of a field and is "attached" to
// the Catalog. This is an internal class and client applications should
// only interact with FormWidgets.
//------------------------------------------------------------------------

class FormField {
public:
  FormField(XRef* xrefa, Object *aobj, const Ref& aref, std::set<int> *usedParents, FormFieldType t=formUndef);

  virtual ~FormField();

  // Accessors.
  FormFieldType getType() { return type; }
  Object* getObj() { return &obj; }
  Ref getRef() { return ref; }

  void setReadOnly (bool b) { readOnly = b; }
  bool isReadOnly () const { return readOnly; }

  FormWidget* findWidgetByRef (Ref aref);
  // Since while loading their defaults, children may call parents methods, it's better
  // to do that when parents are completly constructed
  void loadChildrenDefaults();

  // only implemented in FormFieldButton
  virtual void fillChildrenSiblingsID ();


 protected:
  void _createWidget (Object *obj, Ref aref);

  FormFieldType type;           // field type
  Ref ref;
  bool terminal;
  Object obj;
  XRef *xref;
  FormField **children;
  int numChildren;
  FormWidget **widgets;
  bool readOnly;

private:
  FormField() {}
};


//------------------------------------------------------------------------
// FormFieldButton
//------------------------------------------------------------------------

class FormFieldButton: public FormField {
public:
  FormFieldButton(XRef *xrefA, Object *dict, const Ref& ref, std::set<int> *usedParents);

  FormButtonType getButtonType () { return btype; }

  bool noToggleToOff () const { return noAllOff; }

  // returns gTrue if the state modification is accepted
  GBool setState (int num, GBool s);
  
  void fillChildrenSiblingsID ();

  virtual ~FormFieldButton();
protected:
  FormButtonType btype;
  int size;
  int active_child; //only used for combo box
  bool noAllOff;
};

//------------------------------------------------------------------------
// FormFieldText
//------------------------------------------------------------------------

class FormFieldText: public FormField {
public:
  FormFieldText(XRef *xrefA, Object *dict, const Ref& ref, std::set<int> *usedParents);
  
  GooString* getContent () { return content; }
  GooString* getContentCopy ();
  void setContentCopy (GooString* new_content);
  virtual ~FormFieldText();

  bool isMultiline () const { return multiline; }
  bool isPassword () const { return password; }
  bool isFileSelect () const { return fileSelect; }
  bool noSpellCheck () const { return doNotSpellCheck; }
  bool noScroll () const { return doNotScroll; }
  bool isComb () const { return comb; }
  bool isRichText () const { return richText; }

  int getMaxLen () const { return maxLen; }
protected:
  GooString* content;
  bool multiline;
  bool password;
  bool fileSelect;
  bool doNotSpellCheck;
  bool doNotScroll;
  bool comb;
  bool richText;
  int maxLen;
};

//------------------------------------------------------------------------
// FormFieldChoice
//------------------------------------------------------------------------

class FormFieldChoice: public FormField {
public:
  FormFieldChoice(XRef *xrefA, Object *aobj, const Ref& ref, std::set<int> *usedParents);

  virtual ~FormFieldChoice();

  int getNumChoices() { return numChoices; }
  GooString* getChoice(int i) { return choices[i].optionName; }
  GooString* getExportVal (int i) { return choices[i].exportVal; }

  //select the i-th choice
  void select (int i); 

  //toggle selection of the i-th choice
  void toggle (int i);

  //deselect everything
  void deselectAll ();

  //only work for editable combo box, set the user-entered text as the current choice
  void setEditChoice(GooString* new_content);

  GooString* getEditChoice ();

  bool isSelected (int i) { return choices[i].selected; }

  int getNumSelected ();

  bool isCombo () const { return combo; }
  bool hasEdit () const { return edit; }
  bool isMultiSelect () const { return multiselect; }
  bool noSpellCheck () const { return doNotSpellCheck; }
  bool commitOnSelChange () const { return doCommitOnSelChange; }
  bool isListBox () const { return !combo; }

  /* these functions _must_ only be used by FormWidgetChoice */
  void _setNumChoices (int i) { numChoices = i; }
  void _createChoicesTab ();
  void _setChoiceExportVal (int i, GooString* str) { choices[i].exportVal = str; }
  void _setChoiceOptionName (int i, GooString* str) { choices[i].optionName = str; }

protected:
  bool combo;
  bool edit;
  bool multiselect;
  bool doNotSpellCheck;
  bool doCommitOnSelChange;

  struct ChoiceOpt {
    GooString* exportVal; //the export value ("internal" name)
    GooString* optionName; //displayed name
    bool selected; //if this choice is selected
  };

  int numChoices;
  ChoiceOpt* choices;
  GooString* editedChoice; 
};

//------------------------------------------------------------------------
// FormFieldSignature
//------------------------------------------------------------------------

class FormFieldSignature: public FormField {
public:
  FormFieldSignature(XRef *xrefA, Object *dict, const Ref& ref, std::set<int> *usedParents);

  virtual ~FormFieldSignature();
};

//------------------------------------------------------------------------
// Form
// This class handle the document-wide part of Form (things in the acroForm
// Catalog entry).
//------------------------------------------------------------------------

class Form {
public:
  Form(XRef *xrefA, Object* acroForm);

  ~Form();

  // Look up an inheritable field dictionary entry.
  static Object *fieldLookup(Dict *field, char *key, Object *obj);
  
  /* Creates a new Field of the type specified in obj's dict.
     used in Form::Form and FormField::FormField */
  static FormField *createFieldFromDict (Object* obj, XRef *xref, const Ref& aref, std::set<int> *usedParents);

  Object *getObj () const { return acroForm; }
  GBool getNeedAppearances () const { return needAppearances; }
  int getNumFields() const { return numFields; }
  FormField* getRootField(int i) const { return rootFields[i]; }

  FormWidget* findWidgetByRef (Ref aref);

  void postWidgetsLoad();
private:
  FormField** rootFields;
  int numFields;
  int size;
  XRef* xref;
  Object *acroForm;
  GBool needAppearances;
};

//------------------------------------------------------------------------
// FormPageWidgets
//------------------------------------------------------------------------

class FormPageWidgets {
public:
  FormPageWidgets (XRef *xrefA, Object* annots, unsigned int page, Form *form);
  ~FormPageWidgets();
  
  int getNumWidgets() const { return numWidgets; }
  FormWidget* getWidget(int i) const { return widgets[i]; }

private:
  FormWidget** widgets;
  int numWidgets;
  int size;
  unsigned pageNum;
  XRef* xref;
};

#endif

