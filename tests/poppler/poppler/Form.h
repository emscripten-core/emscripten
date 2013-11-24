//========================================================================
//
// Form.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2006 Julien Rebetez <julienr@svn.gnome.org>
// Copyright 2007, 2008, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2007-2010, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright 2010 Mark Riedesel <mark@klowner.com>
// Copyright 2011 Pino Toscano <pino@kde.org>
// Copyright 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright 2013 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#ifndef FORM_H
#define FORM_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"
#include "Annot.h"

#include <set>

class GooString;
class Array;
class Dict;
class Annot;
class AnnotWidget;
class Annots;
class LinkAction;
class GfxResources;
class PDFDoc;

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

enum VariableTextQuadding {
  quaddingLeftJustified,
  quaddingCentered,
  quaddingRightJustified
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

  // Check if point is inside the field bounding rect
  GBool inRect(double x, double y) const;

  // Get the field bounding rect
  void getRect(double *x1, double *y1, double *x2, double *y2) const;

  unsigned getID () { return ID; }
  void setID (unsigned int i) { ID=i; }

  FormField *getField () { return field; }
  FormFieldType getType() { return type; }

  Object* getObj() { return &obj; }
  Ref getRef() { return ref; }

  void setChildNum (unsigned i) { childNum = i; }
  unsigned getChildNum () { return childNum; }

  double getFontSize() const;

  GooString *getPartialName() const;
  GooString *getAlternateUiName() const;
  GooString *getMappingName() const;
  GooString *getFullyQualifiedName();

  GBool isModified () const;

  bool isReadOnly() const;

  LinkAction *getActivationAction();
  LinkAction *getAdditionalAction(Annot::FormAdditionalActionsType type);

  // return the unique ID corresponding to pageNum/fieldNum
  static int encodeID (unsigned pageNum, unsigned fieldNum);
  // decode id and retrieve pageNum and fieldNum
  static void decodeID (unsigned id, unsigned* pageNum, unsigned* fieldNum);

  void createWidgetAnnotation();
  AnnotWidget *getWidgetAnnotation() const { return widget; }

  virtual void updateWidgetAppearance() = 0;

#ifdef DEBUG_FORMS
  void print(int indent = 0);
#endif

protected:
  FormWidget(PDFDoc *docA, Object *aobj, unsigned num, Ref aref, FormField *fieldA);

  AnnotWidget *widget;
  FormField* field;
  FormFieldType type;
  Object obj;
  Ref ref;
  PDFDoc *doc;
  XRef *xref;

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
};

//------------------------------------------------------------------------
// FormWidgetButton
//------------------------------------------------------------------------

class FormWidgetButton: public FormWidget {
public:
  FormWidgetButton(PDFDoc *docA, Object *dict, unsigned num, Ref ref, FormField *p);
  ~FormWidgetButton ();

  FormButtonType getButtonType() const;
  
  void setState (GBool state);
  GBool getState ();

  char* getOnStr();
  void setAppearanceState(const char *state);
  void updateWidgetAppearance();

protected:
  GooString *onStr;
  FormFieldButton *parent;
};

//------------------------------------------------------------------------
// FormWidgetText
//------------------------------------------------------------------------

class FormWidgetText: public FormWidget {
public:
  FormWidgetText(PDFDoc *docA, Object *dict, unsigned num, Ref ref, FormField *p);
  //return the field's content (UTF16BE)
  GooString* getContent() ;
  //return a copy of the field's content (UTF16BE)
  GooString* getContentCopy();

  //except a UTF16BE string
  void setContent(GooString* new_content);

  void updateWidgetAppearance();

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
  FormWidgetChoice(PDFDoc *docA, Object *dict, unsigned num, Ref ref, FormField *p);
  ~FormWidgetChoice();

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

  void updateWidgetAppearance();
  bool isSelected (int i);

  bool isCombo () const; 
  bool hasEdit () const; 
  bool isMultiSelect () const; 
  bool noSpellCheck () const; 
  bool commitOnSelChange () const; 
  bool isListBox () const;
protected:
  bool _checkRange (int i);
  FormFieldChoice *parent;
};

//------------------------------------------------------------------------
// FormWidgetSignature
//------------------------------------------------------------------------

class FormWidgetSignature: public FormWidget {
public:
  FormWidgetSignature(PDFDoc *docA, Object *dict, unsigned num, Ref ref, FormField *p);
  void updateWidgetAppearance();
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
  FormField(PDFDoc *docA, Object *aobj, const Ref& aref, FormField *parent, std::set<int> *usedParents, FormFieldType t=formUndef);

  virtual ~FormField();

  // Accessors.
  FormFieldType getType() { return type; }
  Object* getObj() { return &obj; }
  Ref getRef() { return ref; }

  void setReadOnly (bool b) { readOnly = b; }
  bool isReadOnly () const { return readOnly; }

  GooString* getDefaultAppearance() const { return defaultAppearance; }
  GBool hasTextQuadding() const { return hasQuadding; }
  VariableTextQuadding getTextQuadding() const { return quadding; }

  GooString *getPartialName() const { return partialName; }
  GooString *getAlternateUiName() const { return alternateUiName; }
  GooString *getMappingName() const { return mappingName; }
  GooString *getFullyQualifiedName();

  FormWidget* findWidgetByRef (Ref aref);
  int getNumWidgets() { return terminal ? numChildren : 0; }
  FormWidget *getWidget(int i) { return terminal ? widgets[i] : NULL; }

  // only implemented in FormFieldButton
  virtual void fillChildrenSiblingsID ();

  void createWidgetAnnotations();

#ifdef DEBUG_FORMS
  void printTree(int indent = 0);
  virtual void print(int indent = 0);
#endif


 protected:
  void _createWidget (Object *obj, Ref aref);
  void createChildren(std::set<int> *usedParents);
  void updateChildrenAppearance();

  FormFieldType type;           // field type
  Ref ref;
  bool terminal;
  Object obj;
  PDFDoc *doc;
  XRef *xref;
  FormField **children;
  FormField *parent;
  int numChildren;
  FormWidget **widgets;
  bool readOnly;

  GooString *partialName; // T field
  GooString *alternateUiName; // TU field
  GooString *mappingName; // TM field
  GooString *fullyQualifiedName;

  // Variable Text
  GooString *defaultAppearance;
  GBool hasQuadding;
  VariableTextQuadding quadding;

private:
  FormField() {}
};


//------------------------------------------------------------------------
// FormFieldButton
//------------------------------------------------------------------------

class FormFieldButton: public FormField {
public:
  FormFieldButton(PDFDoc *docA, Object *dict, const Ref& ref, FormField *parent, std::set<int> *usedParents);

  FormButtonType getButtonType () { return btype; }

  bool noToggleToOff () const { return noAllOff; }

  // returns gTrue if the state modification is accepted
  GBool setState (char *state);
  GBool getState(char *state);

  char *getAppearanceState() { return appearanceState.isName() ? appearanceState.getName() : NULL; }

  void fillChildrenSiblingsID ();
  
  void setNumSiblings (int num);
  void setSibling (int i, FormFieldButton *id) { siblings[i] = id; }

  //For radio buttons, return the fields of the other radio buttons in the same group
  FormFieldButton* getSibling (int i) const { return siblings[i]; }
  int getNumSiblings () const { return numSiblings; }

#ifdef DEBUG_FORMS
  void print(int indent = 0);
#endif

  virtual ~FormFieldButton();
protected:
  void updateState(char *state);

  FormFieldButton** siblings; // IDs of dependent buttons (each button of a radio field has all the others buttons
                               // of the same field in this array)
  int numSiblings;

  FormButtonType btype;
  int size;
  int active_child; //only used for combo box
  bool noAllOff;
  Object appearanceState; // V
};

//------------------------------------------------------------------------
// FormFieldText
//------------------------------------------------------------------------

class FormFieldText: public FormField {
public:
  FormFieldText(PDFDoc *docA, Object *dict, const Ref& ref, FormField *parent, std::set<int> *usedParents);
  
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

#ifdef DEBUG_FORMS
  void print(int indent = 0);
#endif
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
  FormFieldChoice(PDFDoc *docA, Object *aobj, const Ref& ref, FormField *parent, std::set<int> *usedParents);

  virtual ~FormFieldChoice();

  int getNumChoices() { return numChoices; }
  GooString* getChoice(int i) { return choices ? choices[i].optionName : NULL; }
  GooString* getExportVal (int i) { return choices ? choices[i].exportVal : NULL; }
  // For multi-select choices it returns the first one
  GooString* getSelectedChoice();

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

  int getTopIndex() const { return topIdx; }

#ifdef DEBUG_FORMS
  void print(int indent = 0);
#endif

protected:
  void unselectAll();
  void updateSelection();

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
  int topIdx; // TI
};

//------------------------------------------------------------------------
// FormFieldSignature
//------------------------------------------------------------------------

class FormFieldSignature: public FormField {
public:
  FormFieldSignature(PDFDoc *docA, Object *dict, const Ref& ref, FormField *parent, std::set<int> *usedParents);

  virtual ~FormFieldSignature();

#ifdef DEBUG_FORMS
  void print(int indent = 0);
#endif
};

//------------------------------------------------------------------------
// Form
// This class handle the document-wide part of Form (things in the acroForm
// Catalog entry).
//------------------------------------------------------------------------

class Form {
public:
  Form(PDFDoc *docA, Object* acroForm);

  ~Form();

  // Look up an inheritable field dictionary entry.
  static Object *fieldLookup(Dict *field, const char *key, Object *obj);
  
  /* Creates a new Field of the type specified in obj's dict.
     used in Form::Form and FormField::FormField */
  static FormField *createFieldFromDict (Object* obj, PDFDoc *docA, const Ref& aref, FormField *parent, std::set<int> *usedParents);

  Object *getObj () const { return acroForm; }
  GBool getNeedAppearances () const { return needAppearances; }
  int getNumFields() const { return numFields; }
  FormField* getRootField(int i) const { return rootFields[i]; }
  GooString* getDefaultAppearance() const { return defaultAppearance; }
  VariableTextQuadding getTextQuadding() const { return quadding; }
  GfxResources* getDefaultResources() const { return defaultResources; }
  Object* getDefaultResourcesObj() { return &resDict; }

  FormWidget* findWidgetByRef (Ref aref);

  void postWidgetsLoad();
private:
  FormField** rootFields;
  int numFields;
  int size;
  PDFDoc *doc;
  XRef* xref;
  Object *acroForm;
  GBool needAppearances;
  GfxResources *defaultResources;
  Object resDict;

  // Variable Text
  GooString *defaultAppearance;
  VariableTextQuadding quadding;
};

//------------------------------------------------------------------------
// FormPageWidgets
//------------------------------------------------------------------------

class FormPageWidgets {
public:
  FormPageWidgets (Annots* annots, unsigned int page, Form *form);
  ~FormPageWidgets();
  
  int getNumWidgets() const { return numWidgets; }
  FormWidget* getWidget(int i) const { return widgets[i]; }

private:
  FormWidget** widgets;
  int numWidgets;
  int size;
};

#endif

