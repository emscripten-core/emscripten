
/* Generated data (by glib-mkenums) */

#include <config.h>

#include "poppler-enums.h"

/* enumerations from "poppler-action.h" */
#include "poppler-action.h"
GType
poppler_action_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ACTION_UNKNOWN, "POPPLER_ACTION_UNKNOWN", "unknown" },
      { POPPLER_ACTION_NONE, "POPPLER_ACTION_NONE", "none" },
      { POPPLER_ACTION_GOTO_DEST, "POPPLER_ACTION_GOTO_DEST", "goto-dest" },
      { POPPLER_ACTION_GOTO_REMOTE, "POPPLER_ACTION_GOTO_REMOTE", "goto-remote" },
      { POPPLER_ACTION_LAUNCH, "POPPLER_ACTION_LAUNCH", "launch" },
      { POPPLER_ACTION_URI, "POPPLER_ACTION_URI", "uri" },
      { POPPLER_ACTION_NAMED, "POPPLER_ACTION_NAMED", "named" },
      { POPPLER_ACTION_MOVIE, "POPPLER_ACTION_MOVIE", "movie" },
      { POPPLER_ACTION_RENDITION, "POPPLER_ACTION_RENDITION", "rendition" },
      { POPPLER_ACTION_OCG_STATE, "POPPLER_ACTION_OCG_STATE", "ocg-state" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerActionType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_dest_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_DEST_UNKNOWN, "POPPLER_DEST_UNKNOWN", "unknown" },
      { POPPLER_DEST_XYZ, "POPPLER_DEST_XYZ", "xyz" },
      { POPPLER_DEST_FIT, "POPPLER_DEST_FIT", "fit" },
      { POPPLER_DEST_FITH, "POPPLER_DEST_FITH", "fith" },
      { POPPLER_DEST_FITV, "POPPLER_DEST_FITV", "fitv" },
      { POPPLER_DEST_FITR, "POPPLER_DEST_FITR", "fitr" },
      { POPPLER_DEST_FITB, "POPPLER_DEST_FITB", "fitb" },
      { POPPLER_DEST_FITBH, "POPPLER_DEST_FITBH", "fitbh" },
      { POPPLER_DEST_FITBV, "POPPLER_DEST_FITBV", "fitbv" },
      { POPPLER_DEST_NAMED, "POPPLER_DEST_NAMED", "named" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerDestType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_action_movie_operation_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ACTION_MOVIE_PLAY, "POPPLER_ACTION_MOVIE_PLAY", "play" },
      { POPPLER_ACTION_MOVIE_PAUSE, "POPPLER_ACTION_MOVIE_PAUSE", "pause" },
      { POPPLER_ACTION_MOVIE_RESUME, "POPPLER_ACTION_MOVIE_RESUME", "resume" },
      { POPPLER_ACTION_MOVIE_STOP, "POPPLER_ACTION_MOVIE_STOP", "stop" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerActionMovieOperation"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_action_layer_action_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ACTION_LAYER_ON, "POPPLER_ACTION_LAYER_ON", "on" },
      { POPPLER_ACTION_LAYER_OFF, "POPPLER_ACTION_LAYER_OFF", "off" },
      { POPPLER_ACTION_LAYER_TOGGLE, "POPPLER_ACTION_LAYER_TOGGLE", "toggle" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerActionLayerAction"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

/* enumerations from "poppler-document.h" */
#include "poppler-document.h"
GType
poppler_page_layout_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_PAGE_LAYOUT_UNSET, "POPPLER_PAGE_LAYOUT_UNSET", "unset" },
      { POPPLER_PAGE_LAYOUT_SINGLE_PAGE, "POPPLER_PAGE_LAYOUT_SINGLE_PAGE", "single-page" },
      { POPPLER_PAGE_LAYOUT_ONE_COLUMN, "POPPLER_PAGE_LAYOUT_ONE_COLUMN", "one-column" },
      { POPPLER_PAGE_LAYOUT_TWO_COLUMN_LEFT, "POPPLER_PAGE_LAYOUT_TWO_COLUMN_LEFT", "two-column-left" },
      { POPPLER_PAGE_LAYOUT_TWO_COLUMN_RIGHT, "POPPLER_PAGE_LAYOUT_TWO_COLUMN_RIGHT", "two-column-right" },
      { POPPLER_PAGE_LAYOUT_TWO_PAGE_LEFT, "POPPLER_PAGE_LAYOUT_TWO_PAGE_LEFT", "two-page-left" },
      { POPPLER_PAGE_LAYOUT_TWO_PAGE_RIGHT, "POPPLER_PAGE_LAYOUT_TWO_PAGE_RIGHT", "two-page-right" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerPageLayout"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_page_mode_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_PAGE_MODE_UNSET, "POPPLER_PAGE_MODE_UNSET", "unset" },
      { POPPLER_PAGE_MODE_NONE, "POPPLER_PAGE_MODE_NONE", "none" },
      { POPPLER_PAGE_MODE_USE_OUTLINES, "POPPLER_PAGE_MODE_USE_OUTLINES", "use-outlines" },
      { POPPLER_PAGE_MODE_USE_THUMBS, "POPPLER_PAGE_MODE_USE_THUMBS", "use-thumbs" },
      { POPPLER_PAGE_MODE_FULL_SCREEN, "POPPLER_PAGE_MODE_FULL_SCREEN", "full-screen" },
      { POPPLER_PAGE_MODE_USE_OC, "POPPLER_PAGE_MODE_USE_OC", "use-oc" },
      { POPPLER_PAGE_MODE_USE_ATTACHMENTS, "POPPLER_PAGE_MODE_USE_ATTACHMENTS", "use-attachments" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerPageMode"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_font_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_FONT_TYPE_UNKNOWN, "POPPLER_FONT_TYPE_UNKNOWN", "unknown" },
      { POPPLER_FONT_TYPE_TYPE1, "POPPLER_FONT_TYPE_TYPE1", "type1" },
      { POPPLER_FONT_TYPE_TYPE1C, "POPPLER_FONT_TYPE_TYPE1C", "type1c" },
      { POPPLER_FONT_TYPE_TYPE1COT, "POPPLER_FONT_TYPE_TYPE1COT", "type1cot" },
      { POPPLER_FONT_TYPE_TYPE3, "POPPLER_FONT_TYPE_TYPE3", "type3" },
      { POPPLER_FONT_TYPE_TRUETYPE, "POPPLER_FONT_TYPE_TRUETYPE", "truetype" },
      { POPPLER_FONT_TYPE_TRUETYPEOT, "POPPLER_FONT_TYPE_TRUETYPEOT", "truetypeot" },
      { POPPLER_FONT_TYPE_CID_TYPE0, "POPPLER_FONT_TYPE_CID_TYPE0", "cid-type0" },
      { POPPLER_FONT_TYPE_CID_TYPE0C, "POPPLER_FONT_TYPE_CID_TYPE0C", "cid-type0c" },
      { POPPLER_FONT_TYPE_CID_TYPE0COT, "POPPLER_FONT_TYPE_CID_TYPE0COT", "cid-type0cot" },
      { POPPLER_FONT_TYPE_CID_TYPE2, "POPPLER_FONT_TYPE_CID_TYPE2", "cid-type2" },
      { POPPLER_FONT_TYPE_CID_TYPE2OT, "POPPLER_FONT_TYPE_CID_TYPE2OT", "cid-type2ot" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerFontType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_viewer_preferences_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GFlagsValue values[] = {
      { POPPLER_VIEWER_PREFERENCES_UNSET, "POPPLER_VIEWER_PREFERENCES_UNSET", "unset" },
      { POPPLER_VIEWER_PREFERENCES_HIDE_TOOLBAR, "POPPLER_VIEWER_PREFERENCES_HIDE_TOOLBAR", "hide-toolbar" },
      { POPPLER_VIEWER_PREFERENCES_HIDE_MENUBAR, "POPPLER_VIEWER_PREFERENCES_HIDE_MENUBAR", "hide-menubar" },
      { POPPLER_VIEWER_PREFERENCES_HIDE_WINDOWUI, "POPPLER_VIEWER_PREFERENCES_HIDE_WINDOWUI", "hide-windowui" },
      { POPPLER_VIEWER_PREFERENCES_FIT_WINDOW, "POPPLER_VIEWER_PREFERENCES_FIT_WINDOW", "fit-window" },
      { POPPLER_VIEWER_PREFERENCES_CENTER_WINDOW, "POPPLER_VIEWER_PREFERENCES_CENTER_WINDOW", "center-window" },
      { POPPLER_VIEWER_PREFERENCES_DISPLAY_DOC_TITLE, "POPPLER_VIEWER_PREFERENCES_DISPLAY_DOC_TITLE", "display-doc-title" },
      { POPPLER_VIEWER_PREFERENCES_DIRECTION_RTL, "POPPLER_VIEWER_PREFERENCES_DIRECTION_RTL", "direction-rtl" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_flags_register_static (g_intern_static_string ("PopplerViewerPreferences"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_permissions_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GFlagsValue values[] = {
      { POPPLER_PERMISSIONS_OK_TO_PRINT, "POPPLER_PERMISSIONS_OK_TO_PRINT", "ok-to-print" },
      { POPPLER_PERMISSIONS_OK_TO_MODIFY, "POPPLER_PERMISSIONS_OK_TO_MODIFY", "ok-to-modify" },
      { POPPLER_PERMISSIONS_OK_TO_COPY, "POPPLER_PERMISSIONS_OK_TO_COPY", "ok-to-copy" },
      { POPPLER_PERMISSIONS_OK_TO_ADD_NOTES, "POPPLER_PERMISSIONS_OK_TO_ADD_NOTES", "ok-to-add-notes" },
      { POPPLER_PERMISSIONS_OK_TO_FILL_FORM, "POPPLER_PERMISSIONS_OK_TO_FILL_FORM", "ok-to-fill-form" },
      { POPPLER_PERMISSIONS_FULL, "POPPLER_PERMISSIONS_FULL", "full" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_flags_register_static (g_intern_static_string ("PopplerPermissions"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

/* enumerations from "poppler-form-field.h" */
#include "poppler-form-field.h"
GType
poppler_form_field_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_FORM_FIELD_UNKNOWN, "POPPLER_FORM_FIELD_UNKNOWN", "unknown" },
      { POPPLER_FORM_FIELD_BUTTON, "POPPLER_FORM_FIELD_BUTTON", "button" },
      { POPPLER_FORM_FIELD_TEXT, "POPPLER_FORM_FIELD_TEXT", "text" },
      { POPPLER_FORM_FIELD_CHOICE, "POPPLER_FORM_FIELD_CHOICE", "choice" },
      { POPPLER_FORM_FIELD_SIGNATURE, "POPPLER_FORM_FIELD_SIGNATURE", "signature" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerFormFieldType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_form_button_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_FORM_BUTTON_PUSH, "POPPLER_FORM_BUTTON_PUSH", "push" },
      { POPPLER_FORM_BUTTON_CHECK, "POPPLER_FORM_BUTTON_CHECK", "check" },
      { POPPLER_FORM_BUTTON_RADIO, "POPPLER_FORM_BUTTON_RADIO", "radio" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerFormButtonType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_form_text_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_FORM_TEXT_NORMAL, "POPPLER_FORM_TEXT_NORMAL", "normal" },
      { POPPLER_FORM_TEXT_MULTILINE, "POPPLER_FORM_TEXT_MULTILINE", "multiline" },
      { POPPLER_FORM_TEXT_FILE_SELECT, "POPPLER_FORM_TEXT_FILE_SELECT", "file-select" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerFormTextType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_form_choice_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_FORM_CHOICE_COMBO, "POPPLER_FORM_CHOICE_COMBO", "combo" },
      { POPPLER_FORM_CHOICE_LIST, "POPPLER_FORM_CHOICE_LIST", "list" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerFormChoiceType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

/* enumerations from "poppler-annot.h" */
#include "poppler-annot.h"
GType
poppler_annot_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ANNOT_UNKNOWN, "POPPLER_ANNOT_UNKNOWN", "unknown" },
      { POPPLER_ANNOT_TEXT, "POPPLER_ANNOT_TEXT", "text" },
      { POPPLER_ANNOT_LINK, "POPPLER_ANNOT_LINK", "link" },
      { POPPLER_ANNOT_FREE_TEXT, "POPPLER_ANNOT_FREE_TEXT", "free-text" },
      { POPPLER_ANNOT_LINE, "POPPLER_ANNOT_LINE", "line" },
      { POPPLER_ANNOT_SQUARE, "POPPLER_ANNOT_SQUARE", "square" },
      { POPPLER_ANNOT_CIRCLE, "POPPLER_ANNOT_CIRCLE", "circle" },
      { POPPLER_ANNOT_POLYGON, "POPPLER_ANNOT_POLYGON", "polygon" },
      { POPPLER_ANNOT_POLY_LINE, "POPPLER_ANNOT_POLY_LINE", "poly-line" },
      { POPPLER_ANNOT_HIGHLIGHT, "POPPLER_ANNOT_HIGHLIGHT", "highlight" },
      { POPPLER_ANNOT_UNDERLINE, "POPPLER_ANNOT_UNDERLINE", "underline" },
      { POPPLER_ANNOT_SQUIGGLY, "POPPLER_ANNOT_SQUIGGLY", "squiggly" },
      { POPPLER_ANNOT_STRIKE_OUT, "POPPLER_ANNOT_STRIKE_OUT", "strike-out" },
      { POPPLER_ANNOT_STAMP, "POPPLER_ANNOT_STAMP", "stamp" },
      { POPPLER_ANNOT_CARET, "POPPLER_ANNOT_CARET", "caret" },
      { POPPLER_ANNOT_INK, "POPPLER_ANNOT_INK", "ink" },
      { POPPLER_ANNOT_POPUP, "POPPLER_ANNOT_POPUP", "popup" },
      { POPPLER_ANNOT_FILE_ATTACHMENT, "POPPLER_ANNOT_FILE_ATTACHMENT", "file-attachment" },
      { POPPLER_ANNOT_SOUND, "POPPLER_ANNOT_SOUND", "sound" },
      { POPPLER_ANNOT_MOVIE, "POPPLER_ANNOT_MOVIE", "movie" },
      { POPPLER_ANNOT_WIDGET, "POPPLER_ANNOT_WIDGET", "widget" },
      { POPPLER_ANNOT_SCREEN, "POPPLER_ANNOT_SCREEN", "screen" },
      { POPPLER_ANNOT_PRINTER_MARK, "POPPLER_ANNOT_PRINTER_MARK", "printer-mark" },
      { POPPLER_ANNOT_TRAP_NET, "POPPLER_ANNOT_TRAP_NET", "trap-net" },
      { POPPLER_ANNOT_WATERMARK, "POPPLER_ANNOT_WATERMARK", "watermark" },
      { POPPLER_ANNOT_3D, "POPPLER_ANNOT_3D", "3d" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerAnnotType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_annot_flag_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GFlagsValue values[] = {
      { POPPLER_ANNOT_FLAG_UNKNOWN, "POPPLER_ANNOT_FLAG_UNKNOWN", "unknown" },
      { POPPLER_ANNOT_FLAG_INVISIBLE, "POPPLER_ANNOT_FLAG_INVISIBLE", "invisible" },
      { POPPLER_ANNOT_FLAG_HIDDEN, "POPPLER_ANNOT_FLAG_HIDDEN", "hidden" },
      { POPPLER_ANNOT_FLAG_PRINT, "POPPLER_ANNOT_FLAG_PRINT", "print" },
      { POPPLER_ANNOT_FLAG_NO_ZOOM, "POPPLER_ANNOT_FLAG_NO_ZOOM", "no-zoom" },
      { POPPLER_ANNOT_FLAG_NO_ROTATE, "POPPLER_ANNOT_FLAG_NO_ROTATE", "no-rotate" },
      { POPPLER_ANNOT_FLAG_NO_VIEW, "POPPLER_ANNOT_FLAG_NO_VIEW", "no-view" },
      { POPPLER_ANNOT_FLAG_READ_ONLY, "POPPLER_ANNOT_FLAG_READ_ONLY", "read-only" },
      { POPPLER_ANNOT_FLAG_LOCKED, "POPPLER_ANNOT_FLAG_LOCKED", "locked" },
      { POPPLER_ANNOT_FLAG_TOGGLE_NO_VIEW, "POPPLER_ANNOT_FLAG_TOGGLE_NO_VIEW", "toggle-no-view" },
      { POPPLER_ANNOT_FLAG_LOCKED_CONTENTS, "POPPLER_ANNOT_FLAG_LOCKED_CONTENTS", "locked-contents" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_flags_register_static (g_intern_static_string ("PopplerAnnotFlag"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_annot_markup_reply_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ANNOT_MARKUP_REPLY_TYPE_R, "POPPLER_ANNOT_MARKUP_REPLY_TYPE_R", "r" },
      { POPPLER_ANNOT_MARKUP_REPLY_TYPE_GROUP, "POPPLER_ANNOT_MARKUP_REPLY_TYPE_GROUP", "group" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerAnnotMarkupReplyType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_annot_external_data_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_3D, "POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_3D", "3d" },
      { POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_UNKNOWN, "POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_UNKNOWN", "unknown" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerAnnotExternalDataType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_annot_text_state_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ANNOT_TEXT_STATE_MARKED, "POPPLER_ANNOT_TEXT_STATE_MARKED", "marked" },
      { POPPLER_ANNOT_TEXT_STATE_UNMARKED, "POPPLER_ANNOT_TEXT_STATE_UNMARKED", "unmarked" },
      { POPPLER_ANNOT_TEXT_STATE_ACCEPTED, "POPPLER_ANNOT_TEXT_STATE_ACCEPTED", "accepted" },
      { POPPLER_ANNOT_TEXT_STATE_REJECTED, "POPPLER_ANNOT_TEXT_STATE_REJECTED", "rejected" },
      { POPPLER_ANNOT_TEXT_STATE_CANCELLED, "POPPLER_ANNOT_TEXT_STATE_CANCELLED", "cancelled" },
      { POPPLER_ANNOT_TEXT_STATE_COMPLETED, "POPPLER_ANNOT_TEXT_STATE_COMPLETED", "completed" },
      { POPPLER_ANNOT_TEXT_STATE_NONE, "POPPLER_ANNOT_TEXT_STATE_NONE", "none" },
      { POPPLER_ANNOT_TEXT_STATE_UNKNOWN, "POPPLER_ANNOT_TEXT_STATE_UNKNOWN", "unknown" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerAnnotTextState"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_annot_free_text_quadding_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED, "POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED", "left-justified" },
      { POPPLER_ANNOT_FREE_TEXT_QUADDING_CENTERED, "POPPLER_ANNOT_FREE_TEXT_QUADDING_CENTERED", "centered" },
      { POPPLER_ANNOT_FREE_TEXT_QUADDING_RIGHT_JUSTIFIED, "POPPLER_ANNOT_FREE_TEXT_QUADDING_RIGHT_JUSTIFIED", "right-justified" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerAnnotFreeTextQuadding"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

/* enumerations from "poppler.h" */
#include "poppler.h"
GType
poppler_error_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ERROR_INVALID, "POPPLER_ERROR_INVALID", "invalid" },
      { POPPLER_ERROR_ENCRYPTED, "POPPLER_ERROR_ENCRYPTED", "encrypted" },
      { POPPLER_ERROR_OPEN_FILE, "POPPLER_ERROR_OPEN_FILE", "open-file" },
      { POPPLER_ERROR_BAD_CATALOG, "POPPLER_ERROR_BAD_CATALOG", "bad-catalog" },
      { POPPLER_ERROR_DAMAGED, "POPPLER_ERROR_DAMAGED", "damaged" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerError"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_orientation_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_ORIENTATION_PORTRAIT, "POPPLER_ORIENTATION_PORTRAIT", "portrait" },
      { POPPLER_ORIENTATION_LANDSCAPE, "POPPLER_ORIENTATION_LANDSCAPE", "landscape" },
      { POPPLER_ORIENTATION_UPSIDEDOWN, "POPPLER_ORIENTATION_UPSIDEDOWN", "upsidedown" },
      { POPPLER_ORIENTATION_SEASCAPE, "POPPLER_ORIENTATION_SEASCAPE", "seascape" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerOrientation"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_page_transition_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_PAGE_TRANSITION_REPLACE, "POPPLER_PAGE_TRANSITION_REPLACE", "replace" },
      { POPPLER_PAGE_TRANSITION_SPLIT, "POPPLER_PAGE_TRANSITION_SPLIT", "split" },
      { POPPLER_PAGE_TRANSITION_BLINDS, "POPPLER_PAGE_TRANSITION_BLINDS", "blinds" },
      { POPPLER_PAGE_TRANSITION_BOX, "POPPLER_PAGE_TRANSITION_BOX", "box" },
      { POPPLER_PAGE_TRANSITION_WIPE, "POPPLER_PAGE_TRANSITION_WIPE", "wipe" },
      { POPPLER_PAGE_TRANSITION_DISSOLVE, "POPPLER_PAGE_TRANSITION_DISSOLVE", "dissolve" },
      { POPPLER_PAGE_TRANSITION_GLITTER, "POPPLER_PAGE_TRANSITION_GLITTER", "glitter" },
      { POPPLER_PAGE_TRANSITION_FLY, "POPPLER_PAGE_TRANSITION_FLY", "fly" },
      { POPPLER_PAGE_TRANSITION_PUSH, "POPPLER_PAGE_TRANSITION_PUSH", "push" },
      { POPPLER_PAGE_TRANSITION_COVER, "POPPLER_PAGE_TRANSITION_COVER", "cover" },
      { POPPLER_PAGE_TRANSITION_UNCOVER, "POPPLER_PAGE_TRANSITION_UNCOVER", "uncover" },
      { POPPLER_PAGE_TRANSITION_FADE, "POPPLER_PAGE_TRANSITION_FADE", "fade" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerPageTransitionType"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_page_transition_alignment_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_PAGE_TRANSITION_HORIZONTAL, "POPPLER_PAGE_TRANSITION_HORIZONTAL", "horizontal" },
      { POPPLER_PAGE_TRANSITION_VERTICAL, "POPPLER_PAGE_TRANSITION_VERTICAL", "vertical" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerPageTransitionAlignment"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_page_transition_direction_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_PAGE_TRANSITION_INWARD, "POPPLER_PAGE_TRANSITION_INWARD", "inward" },
      { POPPLER_PAGE_TRANSITION_OUTWARD, "POPPLER_PAGE_TRANSITION_OUTWARD", "outward" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerPageTransitionDirection"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_selection_style_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_SELECTION_GLYPH, "POPPLER_SELECTION_GLYPH", "glyph" },
      { POPPLER_SELECTION_WORD, "POPPLER_SELECTION_WORD", "word" },
      { POPPLER_SELECTION_LINE, "POPPLER_SELECTION_LINE", "line" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerSelectionStyle"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_print_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GFlagsValue values[] = {
      { POPPLER_PRINT_DOCUMENT, "POPPLER_PRINT_DOCUMENT", "document" },
      { POPPLER_PRINT_MARKUP_ANNOTS, "POPPLER_PRINT_MARKUP_ANNOTS", "markup-annots" },
      { POPPLER_PRINT_STAMP_ANNOTS_ONLY, "POPPLER_PRINT_STAMP_ANNOTS_ONLY", "stamp-annots-only" },
      { POPPLER_PRINT_ALL, "POPPLER_PRINT_ALL", "all" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_flags_register_static (g_intern_static_string ("PopplerPrintFlags"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}

GType
poppler_backend_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
 
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { POPPLER_BACKEND_UNKNOWN, "POPPLER_BACKEND_UNKNOWN", "unknown" },
      { POPPLER_BACKEND_SPLASH, "POPPLER_BACKEND_SPLASH", "splash" },
      { POPPLER_BACKEND_CAIRO, "POPPLER_BACKEND_CAIRO", "cairo" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = 
       g_enum_register_static (g_intern_static_string ("PopplerBackend"), values);
      
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
    
  return g_define_type_id__volatile;
}



/* Generated data ends here */

