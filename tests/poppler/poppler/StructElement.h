//========================================================================
//
// StructElement.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2013 Igalia S.L.
//
//========================================================================

#ifndef STRUCTELEMENT_H
#define STRUCTELEMENT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "goo/GooString.h"
#include "Object.h"
#include <vector>
#include <set>

class GooString;
class Dict;
class StructElement;
class StructTreeRoot;


class Attribute {
public:
  enum Type {
    Unknown = 0,        // Uninitialized, parsing error, etc.
    UserProperty,       // User defined attribute (i.e. non-standard)

    // Common standard attributes
    Placement, WritingMode, BackgroundColor, BorderColor, BorderStyle,
    BorderThickness, Color, Padding,

    // Block element standard attributes
    SpaceBefore, SpaceAfter, StartIndent, EndIndent, TextIndent, TextAlign,
    BBox, Width, Height, BlockAlign, InlineAlign, TBorderStyle, TPadding,

    // Inline element standard attributes
    BaselineShift, LineHeight, TextDecorationColor, TextDecorationThickness,
    TextDecorationType, RubyAlign, RubyPosition, GlyphOrientationVertical,

    // Column-only standard attributes
    ColumnCount, ColumnGap, ColumnWidths,

    // List-only standard attributes
    ListNumbering,

    // PrintField-only standard attributes
    Role, checked, Desc,

    // Table-only standard attributes
    RowSpan, ColSpan, Headers, Scope, Summary,
  };

  enum Owner {
    UnknownOwner = 0,
    // User-defined attributes
    UserProperties,
    // Standard attributes
    Layout, List, PrintField, Table,
    // Translation to other formats
    XML_1_00, HTML_3_20, HTML_4_01, OEB_1_00, RTF_1_05, CSS_1_00, CSS_2_00,
  };

  // Creates a standard attribute. The name is predefined, and the
  // value is type-checked to conform to the PDF specification.
  Attribute(Type type, Object *value);

  // Creates an UserProperty attribute, with an arbitrary name and value.
  Attribute(const char *name, Object *value);

  GBool isOk() const { return type != Unknown; }

  // Name, type and value can be set only on construction.
  Type getType() const { return type; }
  Owner getOwner() const { return owner; }
  const char *getTypeName() const;
  const char *getOwnerName() const;
  Object *getValue() const { return &value; }
  static Object *getDefaultValue(Type type);

  const char *getName() const { return type == UserProperty ? name.getCString() : getTypeName(); }

  // The revision is optional, and defaults to zero.
  Guint getRevision() const { return revision; }
  void setRevision(Guint revisionA) { revision = revisionA; }

  // Hidden elements should not be displayed by the user agent
  GBool isHidden() const { return hidden; }
  void setHidden(GBool hiddenA) { hidden = hiddenA; }

  // The formatted value may be in the PDF, or be left undefined (NULL).
  // In the later case the user agent should provide a default representation.
  const char *getFormattedValue() const { return formatted ? formatted->getCString() : NULL; }
  void setFormattedValue(const char *formattedA);

  ~Attribute();

private:
  Type type;
  Owner owner;
  Guint revision;
  mutable GooString name;
  mutable Object value;
  GBool hidden;
  GooString *formatted;

  GBool checkType(StructElement *element = NULL);
  static Type getTypeForName(const char *name, StructElement *element = NULL);
  static Attribute *parseUserProperty(Dict *property);

  friend class StructElement;
};


class StructElement {
public:
  enum Type {
    Unknown = 0,
    MCID,                                   // MCID reference, used internally
    OBJR,                                   // Object reference, used internally

    Document, Part, Art, Sect, Div,         // Structural elements

    Span, Quote, Note, Reference, BibEntry, // Inline elements
    Code, Link, Annot,
    BlockQuote, Caption, NonStruct,
    TOC, TOCI, Index, Private,

    P, H, H1, H2, H3, H4, H5, H6,           // Paragraph-like

    L, LI, Lbl, LBody,                      // List elements

    Table, TR, TH, TD, THead, TFoot, TBody, // Table elements

    Ruby, RB, RT, RP,                       // Ruby text elements
    Warichu, WT, WP,

    Figure, Formula, Form,                  // Illustration-like elements
  };

  static const Ref InvalidRef;

  const char *getTypeName() const;
  Type getType() const { return type; }
  GBool isOk() const { return type != Unknown; }
  GBool isBlock() const;
  GBool isInline() const;

  inline GBool isContent() const { return (type == MCID) || isObjectRef(); }
  inline GBool isObjectRef() const { return (type == OBJR && c->ref.num != -1 && c->ref.gen != -1); }

  int getMCID() const { return c->mcid; }
  Ref getObjectRef() const { return c->ref; }
  Ref getParentRef() { return isContent() ? parent->getParentRef() : s->parentRef.getRef(); }
  GBool hasPageRef() const;
  GBool getPageRef(Ref& ref) const;
  StructTreeRoot *getStructTreeRoot() { return treeRoot; }

  // Optional element identifier.
  const GooString *getID() const { return isContent() ? NULL : s->id; }
  GooString *getID() { return isContent() ? NULL : s->id; }

  // Optional ISO language name, e.g. en_US
  GooString *getLanguage() {
    if (!isContent() && s->language) return s->language;
    return parent ? parent->getLanguage() : NULL;
  }
  const GooString *getLanguage() const {
    if (!isContent() && s->language) return s->language;
    return parent ? parent->getLanguage() : NULL;
  }

  // Optional revision number, defaults to zero.
  Guint getRevision() const { return isContent() ? 0 : s->revision; }
  void setRevision(Guint revision) { if (isContent()) s->revision = revision; }

  // Optional element title, in human-readable form.
  const GooString *getTitle() const { return isContent() ? NULL : s->title; }
  GooString *getTitle() { return isContent() ? NULL : s->title; }

  // Optional element expanded abbreviation text.
  const GooString *getExpandedAbbr() const { return isContent() ? NULL : s->expandedAbbr; }
  GooString *getExpandedAbbr() { return isContent() ? NULL : s->expandedAbbr; }

  unsigned getNumElements() const { return isContent() ? 0 : s->elements.size(); }
  const StructElement *getElement(int i) const { return isContent() ? NULL : s->elements.at(i); }
  StructElement *getElement(int i) { return isContent() ? NULL : s->elements.at(i); }

  void appendElement(StructElement *element) {
    if (!isContent() && element && element->isOk()) {
      s->elements.push_back(element);
    }
  }

  unsigned getNumAttributes() const { return isContent() ? 0 : s->attributes.size(); }
  const Attribute *getAttribute(int i) const { return isContent() ? NULL : s->attributes.at(i); }
  Attribute *getAttribute(int i) { return isContent() ? NULL : s->attributes.at(i); }

  void appendAttribute(Attribute *attribute) {
    if (!isContent() && attribute) {
      s->attributes.push_back(attribute);
    }
  }

  const Attribute* findAttribute(Attribute::Type attributeType, GBool inherit = gFalse,
                                 Attribute::Owner owner = Attribute::UnknownOwner) const;

  const GooString *getAltText() const { return isContent() ? NULL : s->altText; }
  GooString *getAltText() { return isContent() ? NULL : s->altText; }

  const GooString *getActualText() const { return isContent() ? NULL : s->actualText; }
  GooString *getActualText() { return isContent() ? NULL : s->actualText; }

  ~StructElement();

private:
  typedef std::vector<Attribute*>     AttrPtrArray;
  typedef std::vector<StructElement*> ElemPtrArray;

  struct StructData {
    Object       parentRef;
    GooString   *altText;
    GooString   *actualText;
    GooString   *id;
    GooString   *title;
    GooString   *expandedAbbr;
    GooString   *language;
    Guint        revision;
    ElemPtrArray elements;
    AttrPtrArray attributes;

    StructData();
    ~StructData();
  };

  // Data in content elements (MCID, MCR)
  struct ContentData {
    union {
      int mcid;
      Ref ref;
    };

    ContentData(int mcidA): mcid(mcidA) {}
    ContentData(const Ref& r) { ref.num = r.num; ref.gen = r.gen; }
  };

  // Common data
  Type type;
  StructTreeRoot *treeRoot;
  StructElement *parent;
  mutable Object pageRef;

  union {
    StructData  *s;
    ContentData *c;
  };

  StructElement(Dict *elementDict, StructTreeRoot *treeRootA, StructElement *parentA, std::set<int> &seen);
  StructElement(int mcid, StructTreeRoot *treeRootA, StructElement *parentA);
  StructElement(const Ref &ref, StructTreeRoot *treeRootA, StructElement *parentA);

  void parse(Dict* elementDict);
  StructElement* parseChild(Object *ref, Object* childObj, std::set<int> &seen);
  void parseChildren(Dict* element, std::set<int> &seen);
  void parseAttributes(Dict *element, GBool keepExisting = gFalse);

  friend class StructTreeRoot;
};

#endif

