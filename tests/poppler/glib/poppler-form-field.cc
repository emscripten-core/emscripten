/* poppler-form-field.cc: glib interface to poppler
 *
 * Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2006 Julien Rebetez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "poppler.h"
#include "poppler-private.h"

/**
 * SECTION:poppler-form-field
 * @short_description: Form Field
 * @title: PoppplerFormField
 */

typedef struct _PopplerFormFieldClass PopplerFormFieldClass;
struct _PopplerFormFieldClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerFormField, poppler_form_field, G_TYPE_OBJECT)

static void
poppler_form_field_finalize (GObject *object)
{
  PopplerFormField *field = POPPLER_FORM_FIELD (object);

  if (field->document)
    {
      g_object_unref (field->document);
      field->document = NULL;
    }
  field->widget = NULL;

  G_OBJECT_CLASS (poppler_form_field_parent_class)->finalize (object);
}

static void
poppler_form_field_init (PopplerFormField *field)
{
}

static void
poppler_form_field_class_init (PopplerFormFieldClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_form_field_finalize;
}

PopplerFormField *
_poppler_form_field_new (PopplerDocument *document,
			 FormWidget      *field)
{
  PopplerFormField *poppler_field;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);
  g_return_val_if_fail (field != NULL, NULL);

  poppler_field = POPPLER_FORM_FIELD (g_object_new (POPPLER_TYPE_FORM_FIELD, NULL));

  poppler_field->document = (PopplerDocument *)g_object_ref (document);
  poppler_field->widget = field;
  
  return poppler_field;
}

/* Public methods */
/**
 * poppler_form_field_get_field_type:
 * @field: a #PopplerFormField
 *
 * Gets the type of @field
 *
 * Return value: #PopplerFormFieldType of @field
 **/ 
PopplerFormFieldType
poppler_form_field_get_field_type (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), POPPLER_FORM_FIELD_UNKNOWN);
  
  switch (field->widget->getType ())
  {
    case formButton:
      return POPPLER_FORM_FIELD_BUTTON;
    case formText:
      return POPPLER_FORM_FIELD_TEXT;
    case formChoice:
      return POPPLER_FORM_FIELD_CHOICE;
    case formSignature:
      return POPPLER_FORM_FIELD_SIGNATURE;
    default:
      g_warning ("Unsupported Form Field Type");
  }

  return POPPLER_FORM_FIELD_UNKNOWN;
}

/**
 * poppler_form_field_get_id:
 * @field: a #PopplerFormField
 *
 * Gets the id of @field
 *
 * Return value: the id of @field
 **/
gint
poppler_form_field_get_id (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), -1);
  
  return field->widget->getID ();
}

/**
 * poppler_form_field_get_font_size
 * @field: a #PopplerFormField
 *
 * Gets the font size of @field
 *
 * Return value: the font size of @field
 **/
gdouble
poppler_form_field_get_font_size (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), 0);
  
  return field->widget->getFontSize ();
}

/**
 * poppler_form_field_is_read_only
 * @field: a #PopplerFormField
 *
 * Checks whether @field is read only
 *
 * Return value: %TRUE if @field is read only
 **/
gboolean
poppler_form_field_is_read_only (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), FALSE);

  return field->widget->isReadOnly ();
}

/* Button Field */
/**
 * poppler_form_field_button_get_button_type
 * @field: a #PopplerFormField
 *
 * Gets the button type of @field
 *
 * Return value: #PopplerFormButtonType of @field
 **/
PopplerFormButtonType
poppler_form_field_button_get_button_type (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formButton, POPPLER_FORM_BUTTON_PUSH);

  switch (static_cast<FormWidgetButton*>(field->widget)->getButtonType ())
    {
      case formButtonPush:
        return POPPLER_FORM_BUTTON_PUSH;
      case formButtonCheck:
        return POPPLER_FORM_BUTTON_CHECK;
      case formButtonRadio:
        return POPPLER_FORM_BUTTON_RADIO;
      default:
        g_assert_not_reached ();
    }
}

/**
 * poppler_form_field_button_get_state
 * @field: a #PopplerFormField
 *
 * Queries a #PopplerFormField and returns its current state. Returns %TRUE if
 * @field is pressed in and %FALSE if it is raised.
 *
 * Return value: current state of @field
 **/
gboolean
poppler_form_field_button_get_state (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formButton, FALSE);
  
  return static_cast<FormWidgetButton*>(field->widget)->getState ();
}

/**
 * poppler_form_field_button_set_state
 * @field: a #PopplerFormField
 * @state: %TRUE or %FALSE
 *
 * Sets the status of @field. Set to %TRUE if you want the #PopplerFormField
 * to be 'pressed in', and %FALSE to raise it. 
 **/
void
poppler_form_field_button_set_state (PopplerFormField *field,
				     gboolean          state)
{
  g_return_if_fail (field->widget->getType () == formButton);

  static_cast<FormWidgetButton*>(field->widget)->setState ((GBool)state);
}

/**
 * poppler_form_field_get_partial_name:
 * @field: a #PopplerFormField
 *
 * Gets the partial name of @field.
 *
 * Return value: a new allocated string. It must be freed with g_free() when done.
 *
 * Since: 0.16
 **/
gchar*
poppler_form_field_get_partial_name (PopplerFormField *field)
{
  GooString *tmp;

  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), NULL);

  tmp = field->widget->getPartialName();

  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}

/**
 * poppler_form_field_get_mapping_name:
 * @field: a #PopplerFormField
 *
 * Gets the mapping name of @field that is used when
 * exporting interactive form field data from the document
 *
 * Return value: a new allocated string. It must be freed with g_free() when done.
 *
 * Since: 0.16
 **/
gchar*
poppler_form_field_get_mapping_name (PopplerFormField *field)
{
  GooString *tmp;

  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), NULL);

  tmp = field->widget->getMappingName();

  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}

/**
 * poppler_form_field_get_name:
 * @field: a #PopplerFormField
 *
 * Gets the fully qualified name of @field. It's constructed by concatenating
 * the partial field names of the field and all of its ancestors.
 *
 * Return value: a new allocated string. It must be freed with g_free() when done.
 *
 * Since: 0.16
 **/
gchar*
poppler_form_field_get_name (PopplerFormField *field)
{
  GooString *tmp;

  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), NULL);

  tmp = field->widget->getFullyQualifiedName();

  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}

/* Text Field */
/**
 * poppler_form_field_text_get_text_type:
 * @field: a #PopplerFormField
 *
 * Gets the text type of @field.
 *
 * Return value: #PopplerFormTextType of @field
 **/
PopplerFormTextType
poppler_form_field_text_get_text_type (PopplerFormField *field)
{
  FormWidgetText *text_field;
  
  g_return_val_if_fail (field->widget->getType () == formText, POPPLER_FORM_TEXT_NORMAL);

  text_field = static_cast<FormWidgetText*>(field->widget);
  
  if (text_field->isMultiline ())
    return POPPLER_FORM_TEXT_MULTILINE;
  else if (text_field->isFileSelect ())
    return POPPLER_FORM_TEXT_FILE_SELECT;

  return POPPLER_FORM_TEXT_NORMAL;
}

/**
 * poppler_form_field_text_get_text:
 * @field: a #PopplerFormField
 *
 * Retrieves the contents of @field.
 *
 * Return value: a new allocated string. It must be freed with g_free() when done.
 **/
gchar *
poppler_form_field_text_get_text (PopplerFormField *field)
{
  FormWidgetText *text_field;
  GooString      *tmp;

  g_return_val_if_fail (field->widget->getType () == formText, NULL);

  text_field = static_cast<FormWidgetText*>(field->widget);
  tmp = text_field->getContent ();

  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}

/**
 * poppler_form_field_text_set_text:
 * @field: a #PopplerFormField
 * @text: the new text
 *
 * Sets the text in @field to the given value, replacing the current contents.
 **/
void
poppler_form_field_text_set_text (PopplerFormField *field,
				  const gchar      *text)
{
  GooString *goo_tmp;
  gchar *tmp;
  gsize length = 0;
	
  g_return_if_fail (field->widget->getType () == formText);

  tmp = text ? g_convert (text, -1, "UTF-16BE", "UTF-8", NULL, &length, NULL) : NULL;
  goo_tmp = new GooString (tmp, length);
  g_free (tmp);
  static_cast<FormWidgetText*>(field->widget)->setContent (goo_tmp);
  delete goo_tmp;
}

/**
 * poppler_form_field_text_get_max_len:
 * @field: a #PopplerFormField
 *
 * Retrieves the maximum allowed length of the text in @field
 *
 * Return value: the maximum allowed number of characters in @field, or -1 if there is no maximum.
 **/
gint
poppler_form_field_text_get_max_len (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, 0);

  return static_cast<FormWidgetText*>(field->widget)->getMaxLen ();
}

/**
 * poppler_form_field_text_do_spell_check:
 * @field: a #PopplerFormField
 *
 * Checks whether spell checking should be done for the contents of @field
 *
 * Return value: %TRUE if spell checking should be done for @field
 **/
gboolean
poppler_form_field_text_do_spell_check (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, FALSE);

  return !static_cast<FormWidgetText*>(field->widget)->noSpellCheck ();
}

gboolean
poppler_form_field_text_do_scroll (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, FALSE);

  return !static_cast<FormWidgetText*>(field->widget)->noScroll ();
}

/**
 * poppler_form_field_text_is_rich_text:
 * @field: a #PopplerFormField
 *
 * Checks whether the contents of @field are rich text
 *
 * Return value: %TRUE if the contents of @field are rich text
 **/
gboolean
poppler_form_field_text_is_rich_text (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, FALSE);

  return static_cast<FormWidgetText*>(field->widget)->isRichText ();
}

/**
 * poppler_form_field_text_is_password:
 * @field: a #PopplerFormField
 *
 * Checks whether content of @field is a password and it must be hidden
 *
 * Return value: %TRUE if the content of @field is a password
 **/
gboolean
poppler_form_field_text_is_password (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, FALSE);

  return static_cast<FormWidgetText*>(field->widget)->isPassword ();
}

/* Choice Field */
/**
 * poppler_form_field_choice_get_choice_type:
 * @field: a #PopplerFormField
 *
 * Gets the choice type of @field
 *
 * Return value: #PopplerFormChoiceType of @field
 **/
PopplerFormChoiceType
poppler_form_field_choice_get_choice_type (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, POPPLER_FORM_CHOICE_COMBO);

  if (static_cast<FormWidgetChoice*>(field->widget)->isCombo ())
    return POPPLER_FORM_CHOICE_COMBO;
  else
    return POPPLER_FORM_CHOICE_LIST;
}

/**
 * poppler_form_field_choice_is_editable:
 * @field: a #PopplerFormField
 *
 * Checks whether @field is editable
 *
 * Return value: %TRUE if @field is editable
 **/ 
gboolean
poppler_form_field_choice_is_editable (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return static_cast<FormWidgetChoice*>(field->widget)->hasEdit ();
}

/**
 * poppler_form_field_choice_can_select_multiple:
 * @field: a #PopplerFormField
 *
 * Checks whether @field allows multiple choices to be selected
 *
 * Return value: %TRUE if @field allows multiple choices to be selected
 **/
gboolean
poppler_form_field_choice_can_select_multiple (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return static_cast<FormWidgetChoice*>(field->widget)->isMultiSelect ();
}

/**
 * poppler_form_field_choice_do_spell_check:
 * @field: a #PopplerFormField
 *
 * Checks whether spell checking should be done for the contents of @field
 *
 * Return value: %TRUE if spell checking should be done for @field
 **/
gboolean
poppler_form_field_choice_do_spell_check (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return !static_cast<FormWidgetChoice*>(field->widget)->noSpellCheck ();
}

gboolean
poppler_form_field_choice_commit_on_change (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return static_cast<FormWidgetChoice*>(field->widget)->commitOnSelChange ();
}

/**
 * poppler_form_field_choice_get_n_items:
 * @field: a #PopplerFormField
 *
 * Returns the number of items on @field
 *
 * Return value: the number of items on @field
 **/
gint
poppler_form_field_choice_get_n_items (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, -1);

  return static_cast<FormWidgetChoice*>(field->widget)->getNumChoices ();
}

/**
 * poppler_form_field_choice_get_item:
 * @field: a #PopplerFormField
 * @index: the index of the item
 *
 * Returns the contents of the item on @field at the given index
 *
 * Return value: a new allocated string. It must be freed with g_free() when done.
 **/
gchar *
poppler_form_field_choice_get_item (PopplerFormField *field,
				    gint              index)
{
  GooString *tmp;
  
  g_return_val_if_fail (field->widget->getType () == formChoice, NULL);

  tmp = static_cast<FormWidgetChoice*>(field->widget)->getChoice (index);
  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}

/**
 * poppler_form_field_choice_is_item_selected:
 * @field: a #PopplerFormField
 * @index: the index of the item
 *
 * Checks whether the item at the given index on @field is currently selected 
 *
 * Return value: %TRUE if item at @index is currently selected
 **/
gboolean
poppler_form_field_choice_is_item_selected (PopplerFormField *field,
					    gint              index)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return static_cast<FormWidgetChoice*>(field->widget)->isSelected (index);
}

/**
 * poppler_form_field_choice_select_item:
 * @field: a #PopplerFormField
 * @index: the index of the item
 *
 * Selects the item at the given index on @field
 **/
void
poppler_form_field_choice_select_item (PopplerFormField *field,
				       gint              index)
{
  g_return_if_fail (field->widget->getType () == formChoice);

  static_cast<FormWidgetChoice*>(field->widget)->select (index);
}

/**
 * poppler_form_field_choice_unselect_all:
 * @field: a #PopplerFormField
 *
 * Unselects all the items on @field
 **/
void
poppler_form_field_choice_unselect_all (PopplerFormField *field)
{
  g_return_if_fail (field->widget->getType () == formChoice);

  static_cast<FormWidgetChoice*>(field->widget)->deselectAll ();
}

/**
 * poppler_form_field_choice_toggle_item:
 * @field: a #PopplerFormField
 * @index: the index of the item
 *
 * Changes the state of the item at the given index
 **/
void
poppler_form_field_choice_toggle_item (PopplerFormField *field,
				       gint              index)
{
  g_return_if_fail (field->widget->getType () == formChoice);

  static_cast<FormWidgetChoice*>(field->widget)->toggle (index);
}

/**
 * poppler_form_field_choice_set_text:
 * @field: a #PopplerFormField
 * @text: the new text
 *
 * Sets the text in @field to the given value, replacing the current contents
 **/
void
poppler_form_field_choice_set_text (PopplerFormField *field,
				    const gchar      *text)
{
  GooString *goo_tmp;
  gchar *tmp;
  gsize length = 0;

  g_return_if_fail (field->widget->getType () == formChoice);

  tmp = text ? g_convert (text, -1, "UTF-16BE", "UTF-8", NULL, &length, NULL) : NULL;
  goo_tmp = new GooString (tmp, length);
  g_free (tmp);
  static_cast<FormWidgetChoice*>(field->widget)->setEditChoice (goo_tmp);
  delete goo_tmp;
}

/**
 * poppler_form_field_choice_get_text:
 * @field: a #PopplerFormField
 *
 * Retrieves the contents of @field.
 *
 * Return value: a new allocated string. It must be freed with g_free() when done.
 **/
gchar *
poppler_form_field_choice_get_text (PopplerFormField *field)
{
  GooString *tmp;
  
  g_return_val_if_fail (field->widget->getType () == formChoice, NULL);

  tmp = static_cast<FormWidgetChoice*>(field->widget)->getEditChoice ();
  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}
