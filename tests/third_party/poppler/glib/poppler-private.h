#ifndef __POPPLER_PRIVATE_H__
#define __POPPLER_PRIVATE_H__

#include <config.h>

#ifndef __GI_SCANNER__
#include <PDFDoc.h>
#include <PSOutputDev.h>
#include <Link.h>
#include <Movie.h>
#include <Rendition.h>
#include <Form.h>
#include <Gfx.h>
#include <FontInfo.h>
#include <TextOutputDev.h>
#include <Catalog.h>
#include <OptionalContent.h>
#include <CairoOutputDev.h>
#endif

struct _PopplerDocument
{
  /*< private >*/
  GObject parent_instance;
  PDFDoc *doc;

  GList *layers;
  GList *layers_rbgroups;
  CairoOutputDev *output_dev;
};

struct _PopplerPSFile
{
  /*< private >*/
  GObject parent_instance;

  PopplerDocument *document;
  PSOutputDev *out;
  char *filename;
  int first_page;
  int last_page;
  double paper_width;
  double paper_height;
  gboolean duplex;
};

struct _PopplerFontInfo
{
  /*< private >*/
  GObject parent_instance;
  PopplerDocument *document;
  FontInfoScanner *scanner;
};

struct _PopplerPage
{
  /*< private >*/
  GObject parent_instance;
  PopplerDocument *document;
  Page *page;
  int index;
  TextPage *text;
  Annots *annots;
};

struct _PopplerFormField
{
  /*< private >*/
  GObject parent_instance;
  PopplerDocument *document;
  FormWidget *widget;
};

struct _PopplerAnnot
{
  GObject  parent_instance;
  Annot   *annot;
};

typedef struct _Layer {
  /*< private >*/
  GList *kids;
  gchar *label;
  OptionalContentGroup *oc;
} Layer;

struct _PopplerLayer
{
  /*< private >*/
  GObject parent_instance;
  PopplerDocument *document;
  Layer *layer;
  GList *rbgroup;
  gchar *title;
};

GList         *_poppler_document_get_layers (PopplerDocument *document);
GList         *_poppler_document_get_layer_rbgroup (PopplerDocument *document,
						    Layer           *layer);
PopplerPage   *_poppler_page_new   (PopplerDocument *document,
				    Page            *page,
				    int              index);
PopplerAction *_poppler_action_new (PopplerDocument *document,
				    LinkAction      *link,
				    const gchar     *title);
PopplerLayer  *_poppler_layer_new (PopplerDocument  *document,
				   Layer            *layer,
				   GList            *rbgroup);
PopplerDest   *_poppler_dest_new_goto (PopplerDocument *document,
				       LinkDest        *link_dest);
PopplerFormField *_poppler_form_field_new (PopplerDocument *document,
					   FormWidget      *field);
PopplerAttachment *_poppler_attachment_new (EmbFile *file);
PopplerMovie      *_poppler_movie_new (Movie *movie);
PopplerMedia      *_poppler_media_new (MediaRendition *media);
PopplerAnnot      *_poppler_annot_new           (Annot *annot);
PopplerAnnot      *_poppler_annot_text_new      (Annot *annot);
PopplerAnnot      *_poppler_annot_free_text_new (Annot *annot);
PopplerAnnot      *_poppler_annot_file_attachment_new (Annot *annot);
PopplerAnnot      *_poppler_annot_movie_new (Annot *annot);
PopplerAnnot      *_poppler_annot_screen_new (Annot *annot);

char *_poppler_goo_string_to_utf8(GooString *s);
gboolean _poppler_convert_pdf_date_to_gtime (GooString *date,
					     time_t    *gdate);

/*
 * A convenience macro for boxed type implementations, which defines a
 * type_name_get_type() function registering the boxed type.
 */
#define POPPLER_DEFINE_BOXED_TYPE(TypeName, type_name, copy_func, free_func)          \
GType                                                                                 \
type_name##_get_type (void)                                                           \
{                                                                                     \
        static volatile gsize g_define_type_id__volatile = 0;                         \
	if (g_once_init_enter (&g_define_type_id__volatile)) {                        \
	        GType g_define_type_id =                                              \
		    g_boxed_type_register_static (g_intern_static_string (#TypeName), \
		                                  (GBoxedCopyFunc) copy_func,         \
						  (GBoxedFreeFunc) free_func);        \
		g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);    \
	}                                                                             \
	return g_define_type_id__volatile;                                            \
}

#endif
