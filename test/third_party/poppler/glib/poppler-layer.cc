/* poppler-layer.cc: glib interface to poppler
 *
 * Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
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

#include "poppler-layer.h"
#include "poppler-private.h"

/**
 * SECTION:poppler-layer
 * @short_description: Layers
 * @title: PopplerLayer
 */

typedef struct _PopplerLayerClass PopplerLayerClass;
struct _PopplerLayerClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerLayer, poppler_layer, G_TYPE_OBJECT)

static void
poppler_layer_finalize (GObject *object)
{
  PopplerLayer *poppler_layer = POPPLER_LAYER (object);

  if (poppler_layer->document)
    {
      g_object_unref (poppler_layer->document);
      poppler_layer->document = NULL;
    }

  if (poppler_layer->title)
    {
      g_free (poppler_layer->title);
      poppler_layer->title = NULL;
    }
  poppler_layer->layer = NULL;
  poppler_layer->rbgroup = NULL;

  G_OBJECT_CLASS (poppler_layer_parent_class)->finalize (object);
}

static void
poppler_layer_init (PopplerLayer *layer)
{
}

static void
poppler_layer_class_init (PopplerLayerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_layer_finalize;
}

PopplerLayer *
_poppler_layer_new (PopplerDocument *document,
		    Layer           *layer,
		    GList           *rbgroup)
{
  PopplerLayer *poppler_layer;
  GooString    *layer_name;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);
  g_return_val_if_fail (layer != NULL, NULL);

  poppler_layer = POPPLER_LAYER (g_object_new (POPPLER_TYPE_LAYER, NULL));

  poppler_layer->document = (PopplerDocument *)g_object_ref (document);
  poppler_layer->layer = layer;
  poppler_layer->rbgroup = rbgroup;
  layer_name = layer->oc->getName ();
  poppler_layer->title = layer_name ? _poppler_goo_string_to_utf8 (layer_name) : NULL;
  
  return poppler_layer;
}

/**
 * poppler_layer_get_title
 * @layer: a #PopplerLayer
 *
 * Returns the name of the layer suitable for
 * presentation as a title in a viewer's GUI
 *
 * Return value: a string containing the title of the layer
 *
 * Since: 0.12
 **/
const gchar *
poppler_layer_get_title (PopplerLayer *poppler_layer)
{
  g_return_val_if_fail (POPPLER_IS_LAYER (poppler_layer), NULL);

  return poppler_layer->title;
}

/**
 * poppler_layer_is_visible
 * @layer: a #PopplerLayer
 *
 * Returns whether @layer is visible
 *
 * Return value: %TRUE if @layer is visible
 *
 * Since: 0.12
 **/
gboolean
poppler_layer_is_visible (PopplerLayer *poppler_layer)
{
  g_return_val_if_fail (POPPLER_IS_LAYER (poppler_layer), FALSE);

  return poppler_layer->layer->oc->getState () == OptionalContentGroup::On;
}

/**
 * poppler_layer_show
 * @layer: a #PopplerLayer
 *
 * Shows @layer
 *
 * Since: 0.12
 **/
void
poppler_layer_show (PopplerLayer *poppler_layer)
{
  GList *l;
  Layer *layer;
  
  g_return_if_fail (POPPLER_IS_LAYER (poppler_layer));

  layer = poppler_layer->layer;

  if (layer->oc->getState () == OptionalContentGroup::On)
    return;
  
  layer->oc->setState (OptionalContentGroup::On);
  
  for (l = poppler_layer->rbgroup; l && l->data; l = g_list_next (l)) {
    OptionalContentGroup *oc = (OptionalContentGroup *)l->data;

    if (oc != layer->oc)
      oc->setState (OptionalContentGroup::Off);
  }
}

/**
 * poppler_layer_hide
 * @layer: a #PopplerLayer
 *
 * Hides @layer. If @layer is the parent of other nested layers,
 * such layers will be also hidden and will be blocked until @layer
 * is shown again
 *
 * Since: 0.12
 **/
void
poppler_layer_hide (PopplerLayer *poppler_layer)
{
  Layer *layer;
  
  g_return_if_fail (POPPLER_IS_LAYER (poppler_layer));

  layer = poppler_layer->layer;
  
  if (layer->oc->getState () == OptionalContentGroup::Off)
    return;
  
  layer->oc->setState (OptionalContentGroup::Off);
}


/**
 * poppler_layer_is_parent
 * @layer: a #PopplerLayer
 *
 * Returns whether @layer is parent of other nested layers.
 *
 * Return value: %TRUE if @layer is a parent layer
 *
 * Since: 0.12
 **/
gboolean
poppler_layer_is_parent (PopplerLayer *poppler_layer)
{
  g_return_val_if_fail (POPPLER_IS_LAYER (poppler_layer), FALSE);

  return poppler_layer->layer->kids != NULL;
}

/**
 * poppler_layer_get_radio_button_group_id
 * @layer: a #PopplerLayer
 *
 * Returns the numeric ID the radio button group associated with @layer.
 *
 * Return value: the ID of the radio button group associated with @layer,
 * or 0 if the layer is not associated to any radio button group
 *
 * Since: 0.12
 **/
gint 
poppler_layer_get_radio_button_group_id (PopplerLayer *poppler_layer)
{
  g_return_val_if_fail (POPPLER_IS_LAYER (poppler_layer), FALSE);

  return GPOINTER_TO_INT (poppler_layer->rbgroup);
}
