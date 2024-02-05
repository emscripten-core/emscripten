#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "poppler.h"

#define FAIL(msg) \
	do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

static gchar *
poppler_format_date (GTime utime)
{
	time_t time = (time_t) utime;
	char s[256];
	const char *fmt_hack = "%c";
	size_t len;
#ifdef HAVE_LOCALTIME_R
	struct tm t;
	if (time == 0 || !localtime_r (&time, &t)) return NULL;
	len = strftime (s, sizeof (s), fmt_hack, &t);
#else
	struct tm *t;
	if (time == 0 || !(t = localtime (&time)) ) return NULL;
	len = strftime (s, sizeof (s), fmt_hack, t);
#endif

	if (len == 0 || s[0] == '\0') return NULL;

	return g_locale_to_utf8 (s, -1, NULL, NULL, NULL);
}

static void
print_index (PopplerIndexIter *iter, gint deph)
{
  do
    {
      PopplerAction    *action;
      PopplerIndexIter *child;
      int               i;

      action = poppler_index_iter_get_action (iter);
      for (i = 0; i < deph; i++)
        g_print (" ");
      g_print ("+ %s\n", action->any.title);
      poppler_action_free (action);
      child = poppler_index_iter_get_child (iter);
      if (child)
        print_index (child, deph + 1);
      poppler_index_iter_free (child);
    }
  while (poppler_index_iter_next (iter));
}

static void
print_layers (PopplerLayersIter *iter, gint deph)
{
  do
    {
      PopplerLayersIter *child;
      PopplerLayer      *layer;
      gint               i;

      for (i = 0; i < deph; i++)
        g_print (" ");

      layer = poppler_layers_iter_get_layer (iter);
      if (layer)
        {
	  g_print ("+ %s (%s)\n", poppler_layer_get_title (layer),
		   poppler_layer_is_visible (layer) ?
		   "Visible" : "Hidden");
	  g_object_unref (layer);
	}
      
      child = poppler_layers_iter_get_child (iter);
      if (child)
        {
	  gchar *title;

	  title = poppler_layers_iter_get_title (iter);
	  if (title)
	    {
	      g_print ("+ %s\n", title);
	      g_free (title);
	    }
	  print_layers (child, deph + 1);
	}
      poppler_layers_iter_free (child);
    }
  while (poppler_layers_iter_next (iter));
}
 
static void
print_document_info (PopplerDocument *document)
{
  gchar *title, *format, *author, *subject, *keywords, *creator, *producer, *linearized;
  GTime creation_date, mod_date;
  gchar *strdate;
  PopplerPageLayout layout;
  PopplerPageMode mode;
  PopplerViewerPreferences view_prefs;
  PopplerPermissions permissions;
  PopplerFontInfo *font_info;
  PopplerFontsIter *fonts_iter;
  PopplerIndexIter *index_iter;
  GEnumValue *enum_value;

  g_object_get (document,
		"title", &title,
		"format", &format,
		"author", &author,
		"subject", &subject,
		"keywords", &keywords,
		"creation-date", &creation_date,
		"mod-date", &mod_date,
		"creator", &creator,
		"producer", &producer,	
		"linearized", &linearized,
		"page-mode", &mode,
		"page-layout", &layout,
		"viewer-preferences", &view_prefs,
		"permissions", &permissions,
		NULL);

  printf ("\t---------------------------------------------------------\n");
  printf ("\tDocument Metadata\n");
  printf ("\t---------------------------------------------------------\n");
  if (title)  printf   ("\ttitle:\t\t%s\n", title);
  if (format) printf   ("\tformat:\t\t%s\n", format);
  if (author) printf   ("\tauthor:\t\t%s\n", author);
  if (subject) printf  ("\tsubject:\t%s\n", subject);
  if (keywords) printf ("\tkeywords:\t%s\n", keywords);
  if (creator) printf ("\tcreator:\t%s\n", creator);
  if (producer) printf ("\tproducer:\t%s\n", producer);
  if (linearized) printf ("\tlinearized:\t%s\n", linearized);
  
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_MODE), mode);
  g_print ("\tpage mode:\t%s\n", enum_value->value_name);
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_LAYOUT), layout);
  g_print ("\tpage layout:\t%s\n", enum_value->value_name);

  strdate = poppler_format_date (creation_date);
  if (strdate)
    {
      g_print ("\tcreation date:\t%s\n", strdate);
      g_free (strdate);
    }
  strdate = poppler_format_date (mod_date);
  if (strdate)
    {
      g_print ("\tmodified date:\t%s\n", strdate);
      g_free (strdate);
    }

  g_print ("\tfonts:\n");
  font_info = poppler_font_info_new (document);
  while (poppler_font_info_scan (font_info, 20, &fonts_iter)) {
    if (fonts_iter) {
      do {
        g_print ("\t\t\t%s\n", poppler_fonts_iter_get_name (fonts_iter));
      } while (poppler_fonts_iter_next (fonts_iter));
      poppler_fonts_iter_free (fonts_iter);
    }
  }
  g_object_unref (font_info);

  index_iter = poppler_index_iter_new (document);
  if (index_iter)
    {
      g_print ("\tindex:\n");
      print_index (index_iter, 0);
      poppler_index_iter_free (index_iter);
    }

  printf ("\t---------------------------------------------------------\n");
  printf ("\tDocument Permissions\n");
  printf ("\t---------------------------------------------------------\n");

  printf ("\tOk to Print: %s\n",
	  permissions & POPPLER_PERMISSIONS_OK_TO_PRINT ? "Yes" : "No");
  printf ("\tOk to Modify: %s\n",
	  permissions & POPPLER_PERMISSIONS_OK_TO_MODIFY ? "Yes" : "No");
  printf ("\tOk to Copy: %s\n",
	  permissions & POPPLER_PERMISSIONS_OK_TO_COPY ? "Yes" : "No");
  printf ("\tOk to Add Notes: %s\n",
	  permissions & POPPLER_PERMISSIONS_OK_TO_ADD_NOTES ? "Yes" : "No");
  printf ("\tOk to Fill Forms: %s\n",
	  permissions & POPPLER_PERMISSIONS_OK_TO_FILL_FORM ? "Yes" : "No");

  printf ("\n");
  
  /* FIXME: print out the view prefs when we support it */

  g_free (title);
  g_free (format);
  g_free (author);
  g_free (subject);
  g_free (keywords);
  g_free (creator);
  g_free (producer); 
  g_free (linearized);
}

static const gchar *
transition_effect_name (PopplerPageTransitionType type)
{
  switch (type)
    {
      case POPPLER_PAGE_TRANSITION_REPLACE:
	return "Replace";
      case POPPLER_PAGE_TRANSITION_SPLIT:
	return "Split";
      case POPPLER_PAGE_TRANSITION_BLINDS:
	return "Blinds";
      case POPPLER_PAGE_TRANSITION_BOX:
	return "Box";
      case POPPLER_PAGE_TRANSITION_WIPE:
	return "Wipe";
      case POPPLER_PAGE_TRANSITION_DISSOLVE:
	return "Dissolve";
      case POPPLER_PAGE_TRANSITION_GLITTER:
	return "Glitter";
      case POPPLER_PAGE_TRANSITION_FLY:
	return "Fly";
      case POPPLER_PAGE_TRANSITION_PUSH:
	return "Push";
      case POPPLER_PAGE_TRANSITION_COVER:
	return "Cover";
      case POPPLER_PAGE_TRANSITION_UNCOVER:
	return "Uncover";
      case POPPLER_PAGE_TRANSITION_FADE:
	return "Fade";
    }

  return "Unknown";
}

static void
print_page_transition (PopplerPageTransition *transition)
{
  printf ("\t\tEffect: %s\n", transition_effect_name (transition->type));
  printf ("\t\tAlignment: %s\n",
	  transition->alignment == POPPLER_PAGE_TRANSITION_HORIZONTAL ?
	  "Horizontal" : "Vertical");
  printf ("\t\tDirection: %s\n",
	  transition->direction == POPPLER_PAGE_TRANSITION_INWARD ?
	  "Inward" : "Outward");
  printf ("\t\tDuration: %d\n", transition->duration);
  printf ("\t\tAngle: %d\n", transition->angle);
  printf ("\t\tScale: %.2f\n", transition->scale);
  printf ("\t\tRectangular: %s\n", transition->rectangular ? "Yes" : "No");
}

static void
form_field_text_print (PopplerFormField *field)
{
  PopplerFormTextType type;
  gchar *text;

  type = poppler_form_field_text_get_text_type (field);
  printf ("\t\tType:\t\tText\n");
  printf ("\t\tMultiline:\t%s\n",
	  type == POPPLER_FORM_TEXT_MULTILINE ? "Yes" : "No");
  printf ("\t\tFileSelect:\t%s\n",
	  type == POPPLER_FORM_TEXT_FILE_SELECT ? "Yes" : "No");
  printf ("\t\tDoSpellCheck:\t%s\n",
	  poppler_form_field_text_do_spell_check (field) ? "Yes" : "No");
  printf ("\t\tDoScroll:\t%s\n",
	  poppler_form_field_text_do_scroll (field) ? "Yes" : "No");
  printf ("\t\tIsRichText:\t%s\n",
	  poppler_form_field_text_is_rich_text (field) ? "Yes" : "No");
  printf ("\t\tPassword:\t%s\n",
	  poppler_form_field_text_is_password (field) ? "Yes" : "No");
  printf ("\t\tMaxLen:\t\t%d\n", poppler_form_field_text_get_max_len (field));
  text = poppler_form_field_text_get_text (field);
  printf ("\t\tContent:\t%s\n", text ? text : "");
  g_free (text);
}

static void
form_field_button_print (PopplerFormField *field)
{
  PopplerFormButtonType button_type;
  const gchar *button_type_str;

  button_type = poppler_form_field_button_get_button_type (field);

  switch (button_type)
    {
      case POPPLER_FORM_BUTTON_PUSH:
        button_type_str = "Push";
	break;
      case POPPLER_FORM_BUTTON_CHECK:
        button_type_str = "Check box";
	break;
      case POPPLER_FORM_BUTTON_RADIO:
        button_type_str = "Radio Button";
	break;
      default:
        g_assert_not_reached ();
    }
  
  printf ("\t\tType:\t\tButton\n");
  printf ("\t\tButton type:\t%s\n", button_type_str);
  if (button_type != POPPLER_FORM_BUTTON_PUSH)
    printf ("\t\tState:\t\t%s\n",
	    poppler_form_field_button_get_state (field) ? "Active" : "Inactive");
}

static void
form_field_choice_print (PopplerFormField *field)
{
  gint i, n_items;
  
  printf ("\t\tType:\t\tChoice\n");
  printf ("\t\tSubType:\t%s\n",
	  poppler_form_field_choice_get_choice_type (field) == POPPLER_FORM_CHOICE_COMBO ?
	  "Combo" : "List");
  printf ("\t\tEditable:\t%s\n",
	  poppler_form_field_choice_is_editable (field) ? "Yes" : "No");
  printf ("\t\tCan select multiple: %s\n",
	  poppler_form_field_choice_can_select_multiple (field) ? "Yes" : "No");
  printf ("\t\tDoSpellCheck:\t%s\n",
	  poppler_form_field_choice_do_spell_check (field) ? "Yes" : "No");
  printf ("\t\tCommit on change: %s\n",
	  poppler_form_field_choice_commit_on_change (field) ? "Yes" : "No");
  
  n_items = poppler_form_field_choice_get_n_items (field);
  for (i = 0; i < n_items; i++)
    {
      gchar *item;

      item = poppler_form_field_choice_get_item (field, i);
      printf ("\t\t\tItem %d: %s %s\n", i, item ? item : "",
	      poppler_form_field_choice_is_item_selected (field, i) ?
	      "(selected)" : "");
      g_free (item);
    }
}

static void
form_field_print (PopplerFormField *field)
{
  printf ("\t\tFont Size:\t%.2f\n",
	  poppler_form_field_get_font_size (field));
  printf ("\t\tReadOnly:\t%s\n",
	  poppler_form_field_is_read_only (field) ? "Yes" : "No");
  
  switch (poppler_form_field_get_field_type (field))
    {
    case POPPLER_FORM_FIELD_TEXT:
      form_field_text_print (field);
      break;
    case POPPLER_FORM_FIELD_BUTTON:
      form_field_button_print (field);
      break;
    case POPPLER_FORM_FIELD_CHOICE:
      form_field_choice_print (field);
      break;
    default:
      printf ("\t\tUnknown form field\n");
    }
  printf ("\n");
}

static void
annot_print (PopplerAnnot *annot)
{
  GEnumValue *enum_value;
  gchar *text;
  
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_ANNOT_TYPE),
				 poppler_annot_get_annot_type (annot));
  g_print ("\t\tType: %s\n", enum_value->value_name);
  text = poppler_annot_get_contents (annot);
  g_print ("\t\tContents: %s\n", text);
  g_free (text);
  printf ("\n");
}

int main (int argc, char *argv[])
{
  PopplerDocument *document;
  PopplerBackend backend;
  PopplerPage *page;
  PopplerPageTransition *transition;
  PopplerFormField *field;
  GEnumValue *enum_value;
  char *label;
  GError *error;
  GdkPixbuf *pixbuf, *thumb;
  double width, height;
  GList *list, *l;
  char *text;
  double duration;
  gint num_images;
  gint num_forms;
  gint num_links;
  gint num_annots;
  gint form_id = 0;
  PopplerLayersIter *layers_iter;

  if (argc != 3)
    FAIL ("usage: test-poppler-glib file://FILE PAGE");

  g_type_init ();

  g_print ("Poppler version %s\n", poppler_get_version ());
  backend = poppler_get_backend ();
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_BACKEND), backend);
  g_print ("Backend is %s\n", enum_value->value_name);

  error = NULL;
  document = poppler_document_new_from_file (argv[1], NULL, &error);
  if (document == NULL)
    FAIL (error->message);

  print_document_info (document); 

  page = poppler_document_get_page_by_label (document, argv[2]);
  if (page == NULL)
    FAIL ("page not found");

  poppler_page_get_size (page, &width, &height);
  printf ("\tpage size:\t%f inches by %f inches\n", width / 72, height / 72);

  duration = poppler_page_get_duration (page);
  if (duration > 0)
    printf ("\tpage duration:\t%f second(s)\n", duration);
  else
    printf ("\tpage duration:\tno duration for page\n");

  transition = poppler_page_get_transition (page);
  if (transition) {
    printf ("\tpage transition:\n");
    print_page_transition (transition);
    poppler_page_transition_free (transition);
  } else {
    printf ("\tpage transition:no transition effect for page\n");
  }

  thumb = poppler_page_get_thumbnail_pixbuf (page);
  if (thumb != NULL) {
    gdk_pixbuf_save (thumb, "thumb.png", "png", &error, NULL);
    if (error != NULL)
      FAIL (error->message);
    else
      printf ("\tthumbnail:\tsaved as thumb.png\n");
    g_object_unref (G_OBJECT (thumb));
  }
  else
    printf ("\tthumbnail:\tno thumbnail for page\n");

  g_object_get (page, "label", &label, NULL);
  printf ("\tpage label:\t%s\n", label);
  g_free (label);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 220, 220);
  gdk_pixbuf_fill (pixbuf, 0x00106000);
  poppler_page_render_to_pixbuf (page, 100, 100, 200, 200, 1, 0, pixbuf);

  gdk_pixbuf_save (pixbuf, "slice.png", "png", &error, NULL);
  printf ("\tslice:\t\tsaved 200x200 slice at (100, 100) as slice.png\n");
  if (error != NULL) {
    FAIL (error->message);
    g_error_free (error);
  }

  g_object_unref (G_OBJECT (pixbuf));

  list = poppler_page_get_link_mapping (page);
  num_links = g_list_length (list);
  if (num_links > 0)
    printf ("\tFound %d links at positions:\n", num_links);
  else
    printf ("\tNo links found\n");
  
  for (l = list; l != NULL; l = l->next)
    {
      PopplerLinkMapping *mapping = (PopplerLinkMapping *)l->data;
      
      printf ("\t\t(%f, %f) - (%f, %f)\n",
	      mapping->area.x1,
	      mapping->area.y1,
	      mapping->area.x2,
	      mapping->area.y2);
      enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_ACTION_TYPE),
				     mapping->action->type);
      g_print ("\t\t\tAction: %s (%d)\n", enum_value->value_name, mapping->action->type);
      switch (mapping->action->type)
        {
	  case POPPLER_ACTION_GOTO_DEST:
	    printf("\t\t\tDest title: %s\n", mapping->action->goto_dest.title);
	    printf("\t\t\tNamed dest: %s\n", mapping->action->goto_dest.dest->named_dest);
	    break;
	  default:
	    printf("\t\t\tDetails unimplemented for this action type\n");
	}
    }
  poppler_page_free_link_mapping (list); 
  
  text = poppler_page_get_text (page);
  if (text)
    {
      FILE *file = fopen ("dump.txt", "w");
      if (file)
	{
	  fwrite (text, strlen (text), 1, file);
	  fclose (file);
	}
      g_free (text);
    }

  list = poppler_page_find_text (page, "Bitwise");
  printf ("\n");  
  printf ("\tFound text \"Bitwise\" at positions:\n");
  for (l = list; l != NULL; l = l->next)
    {
      PopplerRectangle *rect = (PopplerRectangle *)l->data;

      printf ("  (%f,%f)-(%f,%f)\n", rect->x1, rect->y1, rect->x2, rect->y2);
    }

  list = poppler_page_get_image_mapping (page);
  num_images = g_list_length (list);
  printf ("\n");
  if (num_images > 0)
    printf ("\tFound %d images at positions:\n", num_images);
  else
    printf ("\tNo images found\n");
  for (l = list; l != NULL; l = l->next)
    {
      PopplerImageMapping *mapping;
      cairo_surface_t     *image;

      mapping = (PopplerImageMapping *)l->data;
      printf ("\t\t(%f, %f) - (%f, %f)\n",
	      mapping->area.x1,
	      mapping->area.y1,
	      mapping->area.x2,
	      mapping->area.y2);

      image = poppler_page_get_image (page, mapping->image_id);
      printf ("\t\tImage: %p\n", image);
      cairo_surface_destroy (image);
    }
  poppler_page_free_image_mapping (list);

  list = poppler_page_get_form_field_mapping (page);
  num_forms = g_list_length (list);
  printf ("\n");
  if (num_forms > 0)
    printf ("\tFound %d form fields at positions:\n", num_forms);
  else
    printf ("\tNo forms fields found\n");
  for (l = list; l != NULL; l = l->next)
    {
      PopplerFormFieldMapping *mapping;

      mapping = (PopplerFormFieldMapping *)l->data;

      form_id = poppler_form_field_get_id (mapping->field);
      
      printf ("\t\tId: %d: (%f, %f) - (%f, %f)\n",
	      form_id,
	      mapping->area.x1,
	      mapping->area.y1,
	      mapping->area.x2,
	      mapping->area.y2);
      form_field_print (mapping->field);
    }
  poppler_page_free_form_field_mapping (list);

  if (num_forms > 0)
    {
      field = poppler_document_get_form_field (document, form_id);
      printf ("\tForm field for id %d\n", form_id);
      form_field_print (field);
      g_object_unref (field);
    }  

  list = poppler_page_get_annot_mapping (page);
  num_annots = g_list_length (list);
  if (num_annots > 0)
    printf ("\tFound %d annotations at positions:\n", num_annots);
  else
    printf ("\tNo annotations found\n");
  for (l = list; l != NULL; l = l->next)
    {
      PopplerAnnotMapping *mapping = (PopplerAnnotMapping *)l->data;
	    
      printf ("\t\t(%f, %f) - (%f, %f)\n",
	      mapping->area.x1,
	      mapping->area.y1,
	      mapping->area.x2,
	      mapping->area.y2);

      annot_print (mapping->annot);
    }
  poppler_page_free_annot_mapping (list);
  
  if (poppler_document_has_attachments (document))
    {
      int i = 0;

      g_print ("Attachments found:\n\n");

      list = poppler_document_get_attachments (document);
      for (l = list; l; l = l->next)
	{
	  PopplerAttachment *attachment;
	  char *filename, *strdate;

	  filename = g_strdup_printf ("/tmp/attach%d", i);
	  attachment = (PopplerAttachment *)l->data;
	  g_print ("\tname: %s\n", attachment->name);
	  g_print ("\tdescription: %s\n", attachment->description);
	  g_print ("\tsize: %" G_GSIZE_FORMAT "\n", attachment->size);
	  strdate = poppler_format_date (attachment->ctime);
	  if (strdate)
	    {
	      g_print ("\tcreation date: %s\n", strdate);
	      g_free (strdate);
	    }
	  strdate = poppler_format_date (attachment->mtime);
	  if (strdate)
	    {
	      g_print ("\tmodification date: %s\n", strdate);
	      g_free (strdate);
	    }
	  poppler_attachment_save (attachment, filename, NULL);
	  g_free (filename);
	  g_print ("\n");
	  i++;
	}
      g_list_foreach (list, (GFunc) g_object_unref, NULL);
      g_list_free (list);
    }
  else
    g_print ("\tNo attachments found\n");

  layers_iter = poppler_layers_iter_new (document);
  if (layers_iter)
    {
      g_print ("\tLayers:\n");
      print_layers (layers_iter, 0);
      poppler_layers_iter_free (layers_iter);
    }
  else
    g_print ("\tNo layers found\n");

  g_object_unref (G_OBJECT (page));
  g_object_unref (G_OBJECT (document));

  return 0;
}
