//========================================================================
//
// StructElement.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2013 Igalia S.L.
//
//========================================================================

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "StructElement.h"
#include "StructTreeRoot.h"
#include "PDFDoc.h"
#include "Dict.h"

#include <assert.h>

class GfxState;


static GBool isPlacementName(Object *value)
{
  return value->isName("Block")
      || value->isName("Inline")
      || value->isName("Before")
      || value->isName("Start")
      || value->isName("End");
}

static GBool isWritingModeName(Object *value)
{
  return value->isName("LrTb")
      || value->isName("RlTb")
      || value->isName("TbRl");
}

static GBool isBorderStyleName(Object *value)
{
  return value->isName("None")
      || value->isName("Hidden")
      || value->isName("Dotted")
      || value->isName("Dashed")
      || value->isName("Solid")
      || value->isName("Double")
      || value->isName("Groove")
      || value->isName("Ridge")
      || value->isName("Inset")
      || value->isName("Outset");
}

static GBool isTextAlignName(Object *value)
{
  return value->isName("Start")
      || value->isName("End")
      || value->isName("Center")
      || value->isName("Justify");
}

static GBool isBlockAlignName(Object *value)
{
  return value->isName("Before")
      || value->isName("Middle")
      || value->isName("After")
      || value->isName("Justify");
}

static GBool isInlineAlignName(Object *value)
{
  return value->isName("Start")
      || value->isName("End")
      || value->isName("Center");
}

static GBool isNumber(Object *value)
{
  return value->isNum();
}

static GBool isLineHeight(Object *value)
{
  return value->isName("Normal")
      || value->isName("Auto")
      || isNumber(value);
}

static GBool isTextDecorationName(Object *value)
{
  return value->isName("None")
      || value->isName("Underline")
      || value->isName("Overline")
      || value->isName("LineThrough");
}

static GBool isRubyAlignName(Object *value)
{
  return value->isName("Start")
      || value->isName("End")
      || value->isName("Center")
      || value->isName("Justify")
      || value->isName("Distribute");
}

static GBool isRubyPositionName(Object *value)
{
  return value->isName("Before")
      || value->isName("After")
      || value->isName("Warichu")
      || value->isName("Inline");
}

static GBool isGlyphOrientationName(Object *value)
{
  return value->isName("Auto")
      || value->isName("90")
      || value->isName("180")
      || value->isName("270")
      || value->isName("360")
      || value->isName("-90")
      || value->isName("-180");
}

static GBool isListNumberingName(Object *value)
{
  return value->isName("None")
      || value->isName("Disc")
      || value->isName("Circle")
      || value->isName("Square")
      || value->isName("Decimal")
      || value->isName("UpperRoman")
      || value->isName("LowerRoman")
      || value->isName("UpperAlpha")
      || value->isName("LowerAlpha");
}

static GBool isFieldRoleName(Object *value)
{
  return value->isName("rb")
      || value->isName("cb")
      || value->isName("pb")
      || value->isName("tv");
}

static GBool isFieldCheckedName(Object *value)
{
  return value->isName("on")
      || value->isName("off")
      || value->isName("neutral");
}

static GBool isTableScopeName(Object *value)
{
  return value->isName("Row")
      || value->isName("Column")
      || value->isName("Both");
}

static GBool isRGBColor(Object *value)
{
  if (!(value->isArray() && value->arrayGetLength() == 3))
    return gFalse;

  GBool okay = gTrue;
  for (int i = 0; i < 3; i++) {
    Object obj;
    if (!value->arrayGet(i, &obj)->isNum()) {
      okay = gFalse;
      obj.free();
      break;
    }
    if (obj.getNum() < 0.0 || obj.getNum() > 1.0) {
      okay = gFalse;
      obj.free();
      break;
    }
    obj.free();
  }

  return okay;
}

static GBool isNatural(Object *value)
{
  return (value->isInt()   && value->getInt()   > 0)
      || (value->isInt64() && value->getInt64() > 0);
}

static GBool isPositive(Object *value)
{
  return value->isNum() && value->getNum() >= 0.0;
}

static GBool isNumberOrAuto(Object *value)
{
  return isNumber(value) || value->isName("Auto");
}

static GBool isTextString(Object *value)
{
  // XXX: Shall isName() also be checked?
  return value->isString();
}


#define ARRAY_CHECKER(name, checkItem, length, allowSingle, allowNulls) \
    static GBool name(Object *value) {                                  \
      if (!value->isArray())                                            \
        return allowSingle ? checkItem(value) : gFalse;                 \
                                                                        \
      if (length && value->arrayGetLength() != length)                  \
        return gFalse;                                                  \
                                                                        \
      GBool okay = gTrue;                                               \
      for (int i = 0; i < value->arrayGetLength(); i++) {               \
        Object obj;                                                     \
        value->arrayGet(i, &obj);                                       \
        if ((!allowNulls && obj.isNull()) || !checkItem(&obj)) {        \
          okay = gFalse;                                                \
          obj.free();                                                   \
          break;                                                        \
        }                                                               \
        obj.free();                                                     \
      }                                                                 \
      return okay;                                                      \
    }

ARRAY_CHECKER(isRGBColorOrOptionalArray4, isRGBColor,        4, gTrue,  gTrue );
ARRAY_CHECKER(isPositiveOrOptionalArray4, isPositive,        4, gTrue,  gTrue );
ARRAY_CHECKER(isPositiveOrArray4,         isPositive,        4, gTrue,  gFalse);
ARRAY_CHECKER(isBorderStyle,              isBorderStyleName, 4, gTrue,  gTrue );
ARRAY_CHECKER(isNumberArray4,             isNumber,          4, gFalse, gFalse);
ARRAY_CHECKER(isNumberOrArrayN,           isNumber,          0, gTrue,  gFalse);
ARRAY_CHECKER(isTableHeaders,             isTextString,      0, gFalse, gFalse);


// Type of functions used to do type-checking on attribute values
typedef GBool (*AttributeCheckFunc)(Object*);

// Maps attributes to their names and whether the attribute can be inherited.
struct AttributeMapEntry {
  Attribute::Type    type;
  const char        *name;
  const Object      *defval;
  GBool              inherit;
  AttributeCheckFunc check;
};

struct AttributeDefaults {
  Object Inline;
  Object LrTb;
  Object Normal;
  Object Distribute;
  Object off;
  Object Zero;
  Object Auto;
  Object Start;
  Object None;
  Object Before;
  Object Nat1;

  AttributeDefaults() {
    Inline.initName("Inline");
    LrTb.initName("LrTb");
    Normal.initName("Normal");
    Distribute.initName("Distribute");
    off.initName("off");

    Zero.initReal(0.0);
    Auto.initName("Auto");
    Start.initName("Start");
    None.initName("None");
    Before.initName("Before");
    Nat1.initInt(1);
  }
};

static const AttributeDefaults attributeDefaults;


#define ATTR_LIST_END \
  { Attribute::Unknown, NULL, NULL, gFalse, NULL }

#define ATTR_WITH_DEFAULT(name, inherit, check, defval) \
  { Attribute::name,           \
    #name,                     \
    &attributeDefaults.defval, \
    inherit,                   \
    check }

#define ATTR(name, inherit, check) \
  { Attribute::name,           \
    #name,                     \
    NULL,                      \
    inherit,                   \
    check }

static const AttributeMapEntry attributeMapCommonShared[] =
{
  ATTR_WITH_DEFAULT(Placement,       gFalse, isPlacementName, Inline),
  ATTR_WITH_DEFAULT(WritingMode,     gTrue,  isWritingModeName, LrTb),
  ATTR             (BackgroundColor, gFalse, isRGBColor),
  ATTR             (BorderColor,     gTrue,  isRGBColorOrOptionalArray4),
  ATTR_WITH_DEFAULT(BorderStyle,     gFalse, isBorderStyle, None),
  ATTR             (BorderThickness, gTrue,  isPositiveOrOptionalArray4),
  ATTR_WITH_DEFAULT(Padding,         gFalse, isPositiveOrArray4, Zero),
  ATTR             (Color,           gTrue,  isRGBColor),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonBlock[] =
{
  ATTR_WITH_DEFAULT(SpaceBefore, gFalse, isPositive, Zero),
  ATTR_WITH_DEFAULT(SpaceAfter,  gFalse, isPositive, Zero),
  ATTR_WITH_DEFAULT(StartIndent, gTrue,  isNumber,   Zero),
  ATTR_WITH_DEFAULT(EndIndent,   gTrue,  isNumber,   Zero),
  ATTR_WITH_DEFAULT(TextIndent,  gTrue,  isNumber,   Zero),
  ATTR_WITH_DEFAULT(TextAlign,   gTrue,  isTextAlignName, Start),
  ATTR             (BBox,        gFalse, isNumberArray4),
  ATTR_WITH_DEFAULT(Width,       gFalse, isNumberOrAuto, Auto),
  ATTR_WITH_DEFAULT(Height,      gFalse, isNumberOrAuto, Auto),
  ATTR_WITH_DEFAULT(BlockAlign,  gTrue,  isBlockAlignName, Before),
  ATTR_WITH_DEFAULT(InlineAlign, gTrue,  isInlineAlignName, Start),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonInline[] =
{
  ATTR_WITH_DEFAULT(BaselineShift,            gFalse, isNumber, Zero),
  ATTR_WITH_DEFAULT(LineHeight,               gTrue,  isLineHeight, Normal),
  ATTR             (TextDecorationColor,      gTrue,  isRGBColor),
  ATTR             (TextDecorationThickness,  gTrue,  isPositive),
  ATTR_WITH_DEFAULT(TextDecorationType,       gFalse, isTextDecorationName, None),
  ATTR_WITH_DEFAULT(GlyphOrientationVertical, gTrue,  isGlyphOrientationName, Auto),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonRubyText[] =
{
  ATTR_WITH_DEFAULT(RubyPosition, gTrue, isRubyPositionName, Before),
  ATTR_WITH_DEFAULT(RubyAlign,    gTrue, isRubyAlignName, Distribute),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonColumns[] =
{
  ATTR_WITH_DEFAULT(ColumnCount,  gFalse, isNatural, Nat1),
  ATTR             (ColumnGap,    gFalse, isNumberOrArrayN),
  ATTR             (ColumnWidths, gFalse, isNumberOrArrayN),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonList[] = {
  ATTR_WITH_DEFAULT(ListNumbering, gTrue, isListNumberingName, None),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonPrintField[] =
{
  ATTR             (Role,    gFalse, isFieldRoleName),
  ATTR_WITH_DEFAULT(checked, gFalse, isFieldCheckedName, off),
  ATTR             (Desc,    gFalse, isTextString),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonTable[] =
{
  ATTR(Headers, gFalse, isTableHeaders),
  ATTR(Scope,   gFalse, isTableScopeName),
  ATTR(Summary, gFalse, isTextString),
  ATTR_LIST_END
};

static const AttributeMapEntry attributeMapCommonTableCell[] =
{
  ATTR_WITH_DEFAULT(RowSpan,      gFalse, isNatural, Nat1),
  ATTR_WITH_DEFAULT(ColSpan,      gFalse, isNatural, Nat1),
  ATTR_WITH_DEFAULT(TBorderStyle, gTrue,  isBorderStyle, None),
  ATTR_WITH_DEFAULT(TPadding,     gTrue,  isPositiveOrArray4, Zero),
  ATTR_LIST_END
};

#undef ATTR_WITH_DEFAULT
#undef ATTR


static const AttributeMapEntry *attributeMapAll[] = {
  attributeMapCommonShared,
  attributeMapCommonBlock,
  attributeMapCommonInline,
  attributeMapCommonRubyText,
  attributeMapCommonColumns,
  attributeMapCommonList,
  attributeMapCommonPrintField,
  attributeMapCommonTable,
  attributeMapCommonTableCell,
  NULL,
};

static const AttributeMapEntry *attributeMapShared[] = {
  attributeMapCommonShared,
  NULL,
};

static const AttributeMapEntry *attributeMapBlock[] = {
  attributeMapCommonShared,
  attributeMapCommonBlock,
  NULL,
};

static const AttributeMapEntry *attributeMapInline[] = {
  attributeMapCommonShared,
  attributeMapCommonInline,
  NULL,
};

static const AttributeMapEntry *attributeMapTableCell[] = {
  attributeMapCommonShared,
  attributeMapCommonBlock,
  attributeMapCommonTable,
  attributeMapCommonTableCell,
  NULL,
};

static const AttributeMapEntry *attributeMapRubyText[] = {
  attributeMapCommonShared,
  attributeMapCommonInline,
  attributeMapCommonRubyText,
  NULL,
};

static const AttributeMapEntry *attributeMapColumns[] = {
  attributeMapCommonShared,
  attributeMapCommonInline,
  attributeMapCommonColumns,
  NULL,
};

static const AttributeMapEntry *attributeMapList[] = {
  attributeMapCommonShared,
  attributeMapCommonList,
  NULL,
};

static const AttributeMapEntry *attributeMapTable[] = {
  attributeMapCommonShared,
  attributeMapCommonBlock,
  attributeMapCommonTable,
  NULL,
};

static const AttributeMapEntry *attributeMapIllustration[] = {
  // XXX: Illustrations may have some attributes from the "shared", "inline",
  //      the "block" sets. This is a loose specification; making it better
  //      means duplicating entries from the sets. This seems good enough...
  attributeMapCommonShared,
  attributeMapCommonBlock,
  attributeMapCommonInline,
  NULL,
};

// Table mapping owners of attributes to their names.
static const struct OwnerMapEntry {
  Attribute::Owner owner;
  const char      *name;
} ownerMap[] = {
  // XXX: Those are sorted in the owner priority resolution order. If the
  //      same attribute is defined with two owners, the order in the table
  //      can be used to know which one has more priority.
  { Attribute::XML_1_00,       "XML-1.00"       },
  { Attribute::HTML_3_20,      "HTML-3.20"      },
  { Attribute::HTML_4_01,      "HTML-4.01"      },
  { Attribute::OEB_1_00,       "OEB-1.00"       },
  { Attribute::RTF_1_05,       "RTF-1.05"       },
  { Attribute::CSS_1_00,       "CSS-1.00"       },
  { Attribute::CSS_2_00,       "CSS-2.00"       },
  { Attribute::Layout,         "Layout"         },
  { Attribute::PrintField,     "PrintField"     },
  { Attribute::Table,          "Table"          },
  { Attribute::List,           "List"           },
  { Attribute::UserProperties, "UserProperties" },
};


static GBool ownerHasMorePriority(Attribute::Owner a, Attribute::Owner b)
{
  unsigned aIndex, bIndex;

  for (unsigned i = aIndex = bIndex = 0; i < sizeof(ownerMap) / sizeof(ownerMap[0]); i++) {
    if (ownerMap[i].owner == a)
      aIndex = i;
    if (ownerMap[i].owner == b)
      bIndex = i;
  }

  return aIndex < bIndex;
}


// Maps element types to their names and also serves as lookup table
// for additional element type attributes.

enum ElementType {
  elementTypeUndefined,
  elementTypeInline,
  elementTypeBlock,
};

static const struct TypeMapEntry {
  StructElement::Type       type;
  const char               *name;
  ElementType               elementType;
  const AttributeMapEntry **attributes;
} typeMap[] = {
  { StructElement::Document,   "Document",   elementTypeInline,    attributeMapShared       },
  { StructElement::Part,       "Part",       elementTypeInline,    attributeMapShared       },
  { StructElement::Art,        "Art",        elementTypeInline,    attributeMapColumns      },
  { StructElement::Sect,       "Sect",       elementTypeInline,    attributeMapColumns      },
  { StructElement::Div,        "Div",        elementTypeInline,    attributeMapColumns      },
  { StructElement::BlockQuote, "BlockQuote", elementTypeInline,    attributeMapInline       },
  { StructElement::Caption,    "Caption",    elementTypeInline,    attributeMapInline       },
  { StructElement::NonStruct,  "NonStruct",  elementTypeInline,    attributeMapInline       },
  { StructElement::Index,      "Index",      elementTypeInline,    attributeMapInline       },
  { StructElement::Private,    "Private",    elementTypeInline,    attributeMapInline       },
  { StructElement::Span,       "Span",       elementTypeInline,    attributeMapInline       },
  { StructElement::Quote,      "Quote",      elementTypeInline,    attributeMapInline       },
  { StructElement::Note,       "Note",       elementTypeInline,    attributeMapInline       },
  { StructElement::Reference,  "Reference",  elementTypeInline,    attributeMapInline       },
  { StructElement::BibEntry,   "BibEntry",   elementTypeInline,    attributeMapInline       },
  { StructElement::Code,       "Code",       elementTypeInline,    attributeMapInline       },
  { StructElement::Link,       "Link",       elementTypeInline,    attributeMapInline       },
  { StructElement::Annot,      "Annot",      elementTypeInline,    attributeMapInline       },
  { StructElement::Ruby,       "Ruby",       elementTypeInline,    attributeMapRubyText     },
  { StructElement::RB,         "RB",         elementTypeUndefined, attributeMapRubyText     },
  { StructElement::RT,         "RT",         elementTypeUndefined, attributeMapRubyText     },
  { StructElement::RP,         "RP",         elementTypeUndefined, attributeMapShared       },
  { StructElement::Warichu,    "Warichu",    elementTypeInline,    attributeMapRubyText     },
  { StructElement::WT,         "WT",         elementTypeUndefined, attributeMapShared       },
  { StructElement::WP,         "WP",         elementTypeUndefined, attributeMapShared       },
  { StructElement::P,          "P",          elementTypeBlock,     attributeMapBlock        },
  { StructElement::H,          "H",          elementTypeBlock,     attributeMapBlock        },
  { StructElement::H1,         "H1",         elementTypeBlock,     attributeMapBlock        },
  { StructElement::H2,         "H2",         elementTypeBlock,     attributeMapBlock        },
  { StructElement::H3,         "H3",         elementTypeBlock,     attributeMapBlock        },
  { StructElement::H4,         "H4",         elementTypeBlock,     attributeMapBlock        },
  { StructElement::H5,         "H5",         elementTypeBlock,     attributeMapBlock        },
  { StructElement::H6,         "H6",         elementTypeBlock,     attributeMapBlock        },
  { StructElement::L,          "L",          elementTypeBlock,     attributeMapList         },
  { StructElement::LI,         "LI",         elementTypeBlock,     attributeMapBlock        },
  { StructElement::Lbl,        "Lbl",        elementTypeBlock,     attributeMapBlock        },
  { StructElement::LBody,      "LBody",      elementTypeUndefined, attributeMapBlock        },
  { StructElement::Table,      "Table",      elementTypeBlock,     attributeMapTable        },
  { StructElement::TR,         "TR",         elementTypeUndefined, attributeMapShared       },
  { StructElement::TH,         "TH",         elementTypeUndefined, attributeMapTableCell    },
  { StructElement::TD,         "TD",         elementTypeUndefined, attributeMapTableCell    },
  { StructElement::THead,      "THead",      elementTypeUndefined, attributeMapShared       },
  { StructElement::TFoot,      "TFoot",      elementTypeUndefined, attributeMapShared       },
  { StructElement::TBody,      "TBody",      elementTypeUndefined, attributeMapShared       },
  { StructElement::Figure,     "Figure",     elementTypeUndefined, attributeMapIllustration },
  { StructElement::Formula,    "Formula",    elementTypeUndefined, attributeMapIllustration },
  { StructElement::Form,       "Form",       elementTypeUndefined, attributeMapIllustration },
  { StructElement::TOC,        "TOC",        elementTypeUndefined, attributeMapShared       },
  { StructElement::TOCI,       "TOCI",       elementTypeUndefined, attributeMapShared       },
};


//------------------------------------------------------------------------
// Helpers for the attribute and structure type tables
//------------------------------------------------------------------------

static inline const AttributeMapEntry *
getAttributeMapEntry(const AttributeMapEntry **entryList, Attribute::Type type)
{
  assert(entryList);
  while (*entryList) {
    const AttributeMapEntry *entry = *entryList;
    while (entry->type != Attribute::Unknown) {
      assert(entry->name);
      if (type == entry->type)
        return entry;
      entry++;
    }
    entryList++;
  }
  return NULL;
}

static inline const AttributeMapEntry *
getAttributeMapEntry(const AttributeMapEntry **entryList, const char *name)
{
  assert(entryList);
  while (*entryList) {
    const AttributeMapEntry *entry = *entryList;
    while (entry->type != Attribute::Unknown) {
      assert(entry->name);
      if (strcmp(name, entry->name) == 0)
        return entry;
      entry++;
    }
    entryList++;
  }
  return NULL;
}

static inline const OwnerMapEntry *getOwnerMapEntry(Attribute::Owner owner)
{
  for (unsigned i = 0; i < sizeof(ownerMap) / sizeof(ownerMap[0]); i++) {
    if (owner == ownerMap[i].owner)
      return &ownerMap[i];
  }
  return NULL;
}

static inline const OwnerMapEntry *getOwnerMapEntry(const char *name)
{
  for (unsigned i = 0; i < sizeof(ownerMap) / sizeof(ownerMap[0]); i++) {
    if (strcmp(name, ownerMap[i].name) == 0)
      return &ownerMap[i];
  }
  return NULL;
}

static const char *ownerToName(Attribute::Owner owner)
{
  const OwnerMapEntry *entry = getOwnerMapEntry(owner);
  return entry ? entry->name : "UnknownOwner";
}

static Attribute::Owner nameToOwner(const char *name)
{
  const OwnerMapEntry *entry = getOwnerMapEntry(name);
  return entry ? entry->owner : Attribute::UnknownOwner;
}

static inline const TypeMapEntry *getTypeMapEntry(StructElement::Type type)
{
  for (unsigned i = 0; i < sizeof(typeMap) / sizeof(typeMap[0]); i++) {
    if (type == typeMap[i].type)
      return &typeMap[i];
  }
  return NULL;
}

static inline const TypeMapEntry *getTypeMapEntry(const char *name)
{
  for (unsigned i = 0; i < sizeof(typeMap) / sizeof(typeMap[0]); i++) {
    if (strcmp(name, typeMap[i].name) == 0)
      return &typeMap[i];
  }
  return NULL;
}

static const char *typeToName(StructElement::Type type)
{
  if (type == StructElement::MCID)
    return "MarkedContent";
  if (type == StructElement::OBJR)
    return "ObjectReference";

  const TypeMapEntry *entry = getTypeMapEntry(type);
  return entry ? entry->name : "Unknown";
}

static StructElement::Type nameToType(const char *name)
{
  const TypeMapEntry *entry = getTypeMapEntry(name);
  return entry ? entry->type : StructElement::Unknown;
}


//------------------------------------------------------------------------
// Attribute
//------------------------------------------------------------------------

Attribute::Attribute(const char *nameA, Object *valueA):
  type(UserProperty),
  owner(UserProperties),
  revision(0),
  name(nameA),
  value(),
  hidden(gFalse),
  formatted(NULL)
{
  assert(valueA);
  valueA->copy(&value);
}

Attribute::Attribute(Type type, Object *valueA):
  type(type),
  owner(UserProperties), // TODO: Determine corresponding owner from Type
  revision(0),
  name(),
  value(),
  hidden(gFalse),
  formatted(NULL)
{
  assert(valueA);

  valueA->copy(&value);

  if (!checkType())
    type = Unknown;
}

Attribute::~Attribute()
{
  delete formatted;
  value.free();
}

const char *Attribute::getTypeName() const
{
  if (type == UserProperty)
    return name.getCString();

  const AttributeMapEntry *entry = getAttributeMapEntry(attributeMapAll, type);
  if (entry)
    return entry->name;

  return "Unknown";
}

const char *Attribute::getOwnerName() const
{
  return ownerToName(owner);
}

Object *Attribute::getDefaultValue(Attribute::Type type)
{
  const AttributeMapEntry *entry = getAttributeMapEntry(attributeMapAll, type);
  return entry ? const_cast<Object*>(entry->defval) : NULL;
}

void Attribute::setFormattedValue(const char *formattedA)
{
  if (formattedA) {
    if (formatted)
      formatted->Set(formattedA);
    else
      formatted = new GooString(formattedA);
  } else {
    delete formatted;
    formatted = NULL;
  }
}

GBool Attribute::checkType(StructElement *element)
{
  // If an element is passed, tighther type-checking can be done.
  if (!element)
    return gTrue;

  const TypeMapEntry *elementTypeEntry = getTypeMapEntry(element->getType());
  if (elementTypeEntry && elementTypeEntry->attributes) {
    const AttributeMapEntry *entry = getAttributeMapEntry(elementTypeEntry->attributes, type);
    if (entry) {
      if (entry->check && !((*entry->check)(&value))) {
        return gFalse;
      }
    } else {
      // No entry: the attribute is not valid for the containing element.
      return gFalse;
    }
  }

  return gTrue;
}

Attribute::Type Attribute::getTypeForName(const char *name, StructElement *element)
{
  const AttributeMapEntry **attributes = attributeMapAll;
  if (element) {
    const TypeMapEntry *elementTypeEntry = getTypeMapEntry(element->getType());
    if (elementTypeEntry && elementTypeEntry->attributes) {
      attributes = elementTypeEntry->attributes;
    }
  }

  const AttributeMapEntry *entry = getAttributeMapEntry(attributes, name);
  return entry ? entry->type : Unknown;
}

Attribute *Attribute::parseUserProperty(Dict *property)
{
  Object obj, value;
  const char *name = NULL;

  if (property->lookup("N", &obj)->isString())
    name = obj.getString()->getCString();
  else if (obj.isName())
    name = obj.getName();
  else {
    error(errSyntaxError, -1, "N object is wrong type ({0:s})", obj.getTypeName());
    obj.free();
    return NULL;
  }

  if (property->lookup("V", &value)->isNull()) {
    error(errSyntaxError, -1, "V object is wrong type ({0:s})", value.getTypeName());
    value.free();
    obj.free();
    return NULL;
  }

  Attribute *attribute = new Attribute(name, &value);
  value.free();
  obj.free();

  if (property->lookup("F", &obj)->isString()) {
    attribute->setFormattedValue(obj.getString()->getCString());
  } else if (!obj.isNull()) {
    error(errSyntaxWarning, -1, "F object is wrong type ({0:s})", obj.getTypeName());
  }
  obj.free();

  if (property->lookup("H", &obj)->isBool()) {
    attribute->setHidden(obj.getBool());
  } else if (!obj.isNull()) {
    error(errSyntaxWarning, -1, "H object is wrong type ({0:s})", obj.getTypeName());
  }
  obj.free();

  return attribute;
}


//------------------------------------------------------------------------
// StructElement
//------------------------------------------------------------------------

StructElement::StructData::StructData():
  altText(0),
  actualText(0),
  id(0),
  title(0),
  expandedAbbr(0),
  language(0),
  revision(0)
{
}

StructElement::StructData::~StructData()
{
  delete altText;
  delete actualText;
  delete id;
  delete title;
  delete language;
  parentRef.free();
  for (ElemPtrArray::iterator i = elements.begin(); i != elements.end(); ++i) delete *i;
  for (AttrPtrArray::iterator i = attributes.begin(); i != attributes.end(); ++i) delete *i;
}


StructElement::StructElement(Dict *element,
                             StructTreeRoot *treeRootA,
                             StructElement *parentA,
                             std::set<int> &seen):
  type(Unknown),
  treeRoot(treeRootA),
  parent(parentA),
  s(new StructData())
{
  assert(treeRoot);
  assert(element);

  parse(element);
  parseChildren(element, seen);
}

StructElement::StructElement(int mcid, StructTreeRoot *treeRootA, StructElement *parentA):
  type(MCID),
  treeRoot(treeRootA),
  parent(parentA),
  c(new ContentData(mcid))
{
  assert(treeRoot);
  assert(parent);
}

StructElement::StructElement(const Ref& ref, StructTreeRoot *treeRootA, StructElement *parentA):
  type(OBJR),
  treeRoot(treeRootA),
  parent(parentA),
  c(new ContentData(ref))
{
  assert(treeRoot);
  assert(parent);
}

StructElement::~StructElement()
{
  if (isContent())
    delete c;
  else
    delete s;
  pageRef.free();
}

GBool StructElement::isBlock() const
{
  const TypeMapEntry *entry = getTypeMapEntry(type);
  return entry ? (entry->elementType == elementTypeBlock) : gFalse;
}

GBool StructElement::isInline() const
{
  const TypeMapEntry *entry = getTypeMapEntry(type);
  return entry ? (entry->elementType == elementTypeInline) : gFalse;
}

GBool StructElement::hasPageRef() const
{
  return pageRef.isRef() || (parent && parent->hasPageRef());
}

bool StructElement::getPageRef(Ref& ref) const
{
  if (pageRef.isRef()) {
    ref = pageRef.getRef();
    return gTrue;
  }

  if (parent)
    return parent->getPageRef(ref);

  return gFalse;
}

const char *StructElement::getTypeName() const
{
  return typeToName(type);
}

const Attribute *StructElement::findAttribute(Attribute::Type attributeType, GBool inherit,
                                              Attribute::Owner attributeOwner) const
{
  if (isContent())
    return parent->findAttribute(attributeType, inherit, attributeOwner);

  if (attributeType == Attribute::Unknown || attributeType == Attribute::UserProperty)
    return NULL;

  const Attribute *result = NULL;

  if (attributeOwner == Attribute::UnknownOwner) {
    // Search for the attribute, no matter who the owner is
    for (unsigned i = 0; i < getNumAttributes(); i++) {
      const Attribute *attr = getAttribute(i);
      if (attributeType == attr->getType()) {
        if (!result || ownerHasMorePriority(attr->getOwner(), result->getOwner()))
          result = attr;
      }
    }
  } else {
    // Search for the attribute, with a specific owner
    for (unsigned i = 0; i < getNumAttributes(); i++) {
      const Attribute *attr = getAttribute(i);
      if (attributeType == attr->getType() && attributeOwner == attr->getOwner()) {
        result = attr;
        break;
      }
    }
  }

  if (result)
    return result;

  if (inherit && parent) {
    const AttributeMapEntry *entry = getAttributeMapEntry(attributeMapAll, attributeType);
    assert(entry);
    // TODO: Take into account special inheritance cases, for example:
    //       inline elements which have been changed to be block using
    //       "/Placement/Block" have slightly different rules.
    if (entry->inherit)
      return parent->findAttribute(attributeType, inherit, attributeOwner);
  }

  return NULL;
}

static StructElement::Type roleMapResolve(Dict *roleMap, const char *name, const char *curName, Object *resolved)
{
  // Circular reference
  if (curName && !strcmp(name, curName))
    return StructElement::Unknown;

  if (roleMap->lookup(curName ? curName : name, resolved)->isName()) {
    StructElement::Type type = nameToType(resolved->getName());
    return type == StructElement::Unknown
      ? roleMapResolve(roleMap, name, resolved->getName(), resolved)
      : type;
  }

  if (!resolved->isNull())
    error(errSyntaxWarning, -1, "RoleMap entry is wrong type ({0:s})", resolved->getTypeName());
  return StructElement::Unknown;
}

void StructElement::parse(Dict *element)
{
  Object obj;

  // Type is optional, but if present must be StructElem
  if (!element->lookup("Type", &obj)->isNull() && !obj.isName("StructElem")) {
    error(errSyntaxError, -1, "Type of StructElem object is wrong");
    obj.free();
    return;
  }
  obj.free();

  // Parent object reference (required).
  if (!element->lookupNF("P", &s->parentRef)->isRef()) {
    error(errSyntaxError, -1, "P object is wrong type ({0:s})", obj.getTypeName());
    return;
  }

  // Check whether the S-type is valid for the top level
  // element and create a node of the appropriate type.
  if (!element->lookup("S", &obj)->isName()) {
    error(errSyntaxError, -1, "S object is wrong type ({0:s})", obj.getTypeName());
    obj.free();
    return;
  }

  // Type name may not be standard, resolve through RoleMap first.
  if (treeRoot->getRoleMap()) {
    Object resolvedName;
    type = roleMapResolve(treeRoot->getRoleMap(), obj.getName(), NULL, &resolvedName);
  }

  // Resolving through RoleMap may leave type as Unknown, e.g. for types
  // which are not present in it, yet they are standard element types.
  if (type == Unknown)
    type = nameToType(obj.getName());

  // At this point either the type name must have been resolved.
  if (type == Unknown) {
    error(errSyntaxError, -1, "StructElem object is wrong type ({0:s})", obj.getName());
    obj.free();
    return;
  }
  obj.free();

  // Object ID (optional), to be looked at the IDTree in the tree root.
  if (element->lookup("ID", &obj)->isString()) {
    s->id = obj.takeString();
  }
  obj.free();

  // Page reference (optional) in which at least one of the child items
  // is to be rendered in. Note: each element stores only the /Pg value
  // contained by it, and StructElement::getPageRef() may look in parent
  // elements to find the page where an element belongs.
  element->lookupNF("Pg", &pageRef);

  // Revision number (optional).
  if (element->lookup("R", &obj)->isInt()) {
    s->revision = obj.getInt();
  }
  obj.free();

  // Element title (optional).
  if (element->lookup("T", &obj)->isString()) {
    s->title = obj.takeString();
  }
  obj.free();

  // Language (optional).
  if (element->lookup("Lang", &obj)->isString()) {
    s->language = obj.takeString();
  }
  obj.free();

  // Alternative text (optional).
  if (element->lookup("Alt", &obj)->isString()) {
    s->altText = obj.takeString();
  }
  obj.free();

  // Expanded form of an abbreviation (optional).
  if (element->lookup("E", &obj)->isString()) {
    s->expandedAbbr = obj.takeString();
  }
  obj.free();

  // Actual text (optional).
  if (element->lookup("ActualText", &obj)->isString()) {
    s->actualText = obj.takeString();
  }
  obj.free();

  // Attributes directly attached to the element (optional).
  if (element->lookup("A", &obj)->isDict()) {
    parseAttributes(obj.getDict());
  } else if (obj.isArray()) {
    Object iobj;
    unsigned attrIndex = getNumAttributes();
    for (int i = 0; i < obj.arrayGetLength(); i++) {
      if (obj.arrayGet(i, &iobj)->isDict()) {
        attrIndex = getNumAttributes();
        parseAttributes(obj.getDict());
      } else if (iobj.isInt()) {
        const int revision = iobj.getInt();
        // Set revision numbers for the elements previously created.
        for (unsigned j = attrIndex; j < getNumAttributes(); j++)
          getAttribute(j)->setRevision(revision);
      } else {
        error(errSyntaxWarning, -1, "A item is wrong type ({0:s})", iobj.getTypeName());
      }
      iobj.free();
    }
  } else if (!obj.isNull()) {
    error(errSyntaxWarning, -1, "A is wrong type ({0:s})", obj.getTypeName());
  }
  obj.free();

  // Attributes referenced indirectly through the ClassMap (optional).
  if (treeRoot->getClassMap()) {
    Object classes;
    if (element->lookup("C", &classes)->isName()) {
      Object attr;
      if (treeRoot->getClassMap()->lookup(classes.getName(), &attr)->isDict()) {
        parseAttributes(attr.getDict(), gTrue);
      } else if (attr.isArray()) {
        for (int i = 0; i < attr.arrayGetLength(); i++) {
          Object iobj;
          unsigned attrIndex = getNumAttributes();
          if (attr.arrayGet(i, &iobj)->isDict()) {
            attrIndex = getNumAttributes();
            parseAttributes(iobj.getDict(), gTrue);
          } else if (iobj.isInt()) {
            // Set revision numbers for the elements previously created.
            const int revision = iobj.getInt();
            for (unsigned j = attrIndex; j < getNumAttributes(); j++)
              getAttribute(j)->setRevision(revision);
          } else {
            error(errSyntaxWarning, -1, "C item is wrong type ({0:s})", iobj.getTypeName());
          }
          iobj.free();
        }
      } else if (!attr.isNull()) {
        error(errSyntaxWarning, -1, "C object is wrong type ({0:s})", classes.getTypeName());
      }
      classes.free();
      attr.free();
    }
  }
}

StructElement *StructElement::parseChild(Object *ref,
                                         Object *childObj,
                                         std::set<int> &seen)
{
  assert(childObj);
  assert(ref);

  StructElement *child = NULL;

  if (childObj->isInt()) {
    child = new StructElement(childObj->getInt(), treeRoot, this);
  } else if (childObj->isDict("MCR")) {
    /*
     * TODO: The optional Stm/StwOwn attributes are not handled, so all the
     *      page will be always scanned when calling StructElement::getText().
     */
    Object mcidObj;
    Object pageRefObj;

    if (!childObj->dictLookup("MCID", &mcidObj)->isInt()) {
      error(errSyntaxError, -1, "MCID object is wrong type ({0:s})", mcidObj.getTypeName());
      mcidObj.free();
      return NULL;
    }

    child = new StructElement(mcidObj.getInt(), treeRoot, this);
    mcidObj.free();

    if (childObj->dictLookupNF("Pg", &pageRefObj)->isRef()) {
      child->pageRef = pageRefObj;
    } else {
      pageRefObj.free();
    }
  } else if (childObj->isDict("OBJR")) {
    Object refObj;

    if (childObj->dictLookupNF("Obj", &refObj)->isRef()) {
      Object pageRefObj;

      child = new StructElement(refObj.getRef(), treeRoot, this);

      if (childObj->dictLookupNF("Pg", &pageRefObj)->isRef()) {
        child->pageRef = pageRefObj;
      } else {
        pageRefObj.free();
      }
    } else {
      error(errSyntaxError, -1, "Obj object is wrong type ({0:s})", refObj.getTypeName());
    }
    refObj.free();
  } else if (childObj->isDict()) {
    if (!ref->isRef()) {
      error(errSyntaxError, -1,
            "Structure element dictionary is not an indirect reference ({0:s})",
            ref->getTypeName());
    } else if (seen.find(ref->getRefNum()) == seen.end()) {
      seen.insert(ref->getRefNum());
      child = new StructElement(childObj->getDict(), treeRoot, this, seen);
    } else {
      error(errSyntaxWarning, -1,
            "Loop detected in structure tree, skipping subtree at object {0:i}:{0:i}",
            ref->getRefNum(), ref->getRefGen());
    }
  } else {
    error(errSyntaxWarning, -1, "K has a child of wrong type ({0:s})", childObj->getTypeName());
  }

  if (child) {
    if (child->isOk()) {
      appendElement(child);
      if (ref->isRef())
        treeRoot->parentTreeAdd(ref->getRef(), child);
    } else {
      delete child;
      child = NULL;
    }
  }

  return child;
}

void StructElement::parseChildren(Dict *element, std::set<int> &seen)
{
  Object kids;

  if (element->lookup("K", &kids)->isArray()) {
    for (int i = 0; i < kids.arrayGetLength(); i++) {
      Object obj, ref;
      parseChild(kids.arrayGetNF(i, &ref), kids.arrayGet(i, &obj), seen);
      obj.free();
      ref.free();
    }
  } else if (kids.isDict() || kids.isInt()) {
    Object ref;
    parseChild(element->lookupNF("K", &ref), &kids, seen);
    ref.free();
  }

  kids.free();
}

void StructElement::parseAttributes(Dict *attributes, GBool keepExisting)
{
  Object owner;
  if (attributes->lookup("O", &owner)->isName("UserProperties")) {
    // In this case /P is an array of UserProperty dictionaries
    Object userProperties;
    if (attributes->lookup("P", &userProperties)->isArray()) {
      for (int i = 0; i < userProperties.arrayGetLength(); i++) {
        Object property;
        if (userProperties.arrayGet(i, &property)->isDict()) {
          Attribute *attribute = Attribute::parseUserProperty(property.getDict());
          if (attribute && attribute->isOk()) {
            appendAttribute(attribute);
          } else {
            error(errSyntaxWarning, -1, "Item in P is invalid");
            delete attribute;
          }
        } else {
          error(errSyntaxWarning, -1, "Item in P is wrong type ({0:s})", property.getTypeName());
        }
        property.free();
      }
    }
    userProperties.free();
  } else if (owner.isName()) {
    // In this case /P contains standard attributes.
    // Check first if the owner is a valid standard one.
    Attribute::Owner ownerValue = nameToOwner(owner.getName());
    if (ownerValue != Attribute::UnknownOwner) {
      // Iterate over the entries of the "attributes" dictionary.
      // The /O entry (owner) is skipped.
      for (int i = 0; i < attributes->getLength(); i++) {
        const char *key = attributes->getKey(i);
        if (strcmp(key, "O") != 0) {
          Attribute::Type type = Attribute::getTypeForName(key, this);

          // Check if the attribute is already defined.
          if (keepExisting) {
            GBool exists = gFalse;
            for (unsigned j = 0; j < getNumAttributes(); j++) {
              if (getAttribute(j)->getType() == type) {
                exists = gTrue;
                break;
              }
            }
            if (exists)
              continue;
          }

          if (type != Attribute::Unknown) {
            Object value;
            GBool typeCheckOk = gTrue;
            Attribute *attribute = new Attribute(type, attributes->getVal(i, &value));
            value.free();

            if (attribute->isOk() && (typeCheckOk = attribute->checkType(this))) {
              appendAttribute(attribute);
            } else {
              // It is not needed to free "value", the Attribute instance
              // owns the contents, so deleting "attribute" is enough.
              if (!typeCheckOk) {
                error(errSyntaxWarning, -1, "Attribute {0:s} value is of wrong type ({1:s})",
                      attribute->getTypeName(), attribute->getValue()->getTypeName());
              }
              delete attribute;
            }
          } else {
            error(errSyntaxWarning, -1, "Wrong Attribute '{0:s}' in element {1:s}", key, getTypeName());
          }
        }
      }
    } else {
      error(errSyntaxWarning, -1, "O object is invalid value ({0:s})", owner.getName());
    }
  } else if (!owner.isNull()) {
    error(errSyntaxWarning, -1, "O is wrong type ({0:s})", owner.getTypeName());
  }
  owner.free();
}
