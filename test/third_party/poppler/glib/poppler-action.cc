/* poppler-action.cc: glib wrapper for poppler	      -*- c-basic-offset: 8 -*-
 * Copyright (C) 2005, Red Hat, Inc.
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
 * SECTION:poppler-action
 * @short_description: Action links
 * @title: PopplerAction
 */

POPPLER_DEFINE_BOXED_TYPE (PopplerDest, poppler_dest, poppler_dest_copy, poppler_dest_free)

/**
 * poppler_dest_copy:
 * @dest: a #PopplerDest
 *
 * Copies @dest, creating an identical #PopplerDest.
 *
 * Return value: a new destination identical to @dest
 **/
PopplerDest *
poppler_dest_copy (PopplerDest *dest)
{
	PopplerDest *new_dest;

	new_dest = g_slice_dup (PopplerDest, dest);

	if (dest->named_dest)
		new_dest->named_dest = g_strdup (dest->named_dest);

	return new_dest;
}


/**
 * poppler_dest_free:
 * @dest: a #PopplerDest
 *
 * Frees @dest
 **/
void
poppler_dest_free (PopplerDest *dest)
{
	if (!dest)
		return;
	
	if (dest->named_dest)
		g_free (dest->named_dest);
	
	g_slice_free (PopplerDest, dest);
}

static void
poppler_action_layer_free (PopplerActionLayer *action_layer)
{
	if (!action_layer)
		return;

	if (action_layer->layers) {
		g_list_foreach (action_layer->layers, (GFunc)g_object_unref, NULL);
		g_list_free (action_layer->layers);
		action_layer->layers = NULL;
	}

	g_slice_free (PopplerActionLayer, action_layer);
}

static PopplerActionLayer *
poppler_action_layer_copy (PopplerActionLayer *action_layer)
{
	PopplerActionLayer *retval = g_slice_dup (PopplerActionLayer, action_layer);

	retval->layers = g_list_copy (action_layer->layers);
	g_list_foreach (action_layer->layers, (GFunc)g_object_ref, NULL);

	return retval;
}

POPPLER_DEFINE_BOXED_TYPE (PopplerAction, poppler_action, poppler_action_copy, poppler_action_free)

/**
 * poppler_action_free:
 * @action: a #PopplerAction
 * 
 * Frees @action
 **/
void
poppler_action_free (PopplerAction *action)
{
	if (action == NULL)
		return;

	/* Action specific stuff */
	switch (action->type) {
	case POPPLER_ACTION_GOTO_DEST:
		poppler_dest_free (action->goto_dest.dest);
		break;
	case POPPLER_ACTION_GOTO_REMOTE:
		poppler_dest_free (action->goto_remote.dest);
		g_free (action->goto_remote.file_name);
		break;
	case POPPLER_ACTION_URI:
		g_free (action->uri.uri);
		break;
	case POPPLER_ACTION_LAUNCH:
		g_free (action->launch.file_name);
		g_free (action->launch.params);
		break;
	case POPPLER_ACTION_NAMED:
		g_free (action->named.named_dest);
		break;
	case POPPLER_ACTION_MOVIE:
		if (action->movie.movie)
			g_object_unref (action->movie.movie);
		break;
	case POPPLER_ACTION_RENDITION:
		if (action->rendition.media)
			g_object_unref (action->rendition.media);
		break;
	case POPPLER_ACTION_OCG_STATE:
		if (action->ocg_state.state_list) {
			g_list_foreach (action->ocg_state.state_list, (GFunc)poppler_action_layer_free, NULL);
			g_list_free (action->ocg_state.state_list);
		}
		break;
	default:
		break;
	}
	
	g_free (action->any.title);
	g_slice_free (PopplerAction, action);
}

/**
 * poppler_action_copy:
 * @action: a #PopplerAction
 * 
 * Copies @action, creating an identical #PopplerAction.
 * 
 * Return value: a new action identical to @action
 **/
PopplerAction *
poppler_action_copy (PopplerAction *action)
{
	PopplerAction *new_action;

	g_return_val_if_fail (action != NULL, NULL);

	/* Do a straight copy of the memory */
	new_action = g_slice_dup (PopplerAction, action);

	if (action->any.title != NULL)
		new_action->any.title = g_strdup (action->any.title);

	switch (action->type) {
	case POPPLER_ACTION_GOTO_DEST:
		new_action->goto_dest.dest = poppler_dest_copy (action->goto_dest.dest);
		break;
	case POPPLER_ACTION_GOTO_REMOTE:
		new_action->goto_remote.dest = poppler_dest_copy (action->goto_remote.dest);
		if (action->goto_remote.file_name)
			new_action->goto_remote.file_name = g_strdup (action->goto_remote.file_name);
		break;
	case POPPLER_ACTION_URI:
		if (action->uri.uri)
			new_action->uri.uri = g_strdup (action->uri.uri);
		break;
	case POPPLER_ACTION_LAUNCH:
		if (action->launch.file_name)
			new_action->launch.file_name = g_strdup (action->launch.file_name);
		if (action->launch.params)
			new_action->launch.params = g_strdup (action->launch.params);
		break;
	case POPPLER_ACTION_NAMED:
		if (action->named.named_dest)
			new_action->named.named_dest = g_strdup (action->named.named_dest);
		break;
	case POPPLER_ACTION_MOVIE:
		if (action->movie.movie)
			new_action->movie.movie = (PopplerMovie *)g_object_ref (action->movie.movie);
		break;
	case POPPLER_ACTION_RENDITION:
		if (action->rendition.media)
			new_action->rendition.media = (PopplerMedia *)g_object_ref (action->rendition.media);
		break;
	case POPPLER_ACTION_OCG_STATE:
		if (action->ocg_state.state_list) {
			GList *l;
			GList *new_list = NULL;

			for (l = action->ocg_state.state_list; l; l = g_list_next (l)) {
				PopplerActionLayer *alayer = (PopplerActionLayer *)l->data;
				new_list = g_list_prepend (new_list, poppler_action_layer_copy (alayer));
			}

			new_action->ocg_state.state_list = g_list_reverse (new_list);
		}

		break;
	default:
		break;
	}
	    
	return new_action;
}

PopplerDest *
dest_new_goto (PopplerDocument *document,
	       LinkDest        *link_dest)
{
	PopplerDest *dest;

	dest = g_slice_new0 (PopplerDest);

	if (link_dest == NULL) {
		dest->type = POPPLER_DEST_UNKNOWN;
		return dest;
	}

	switch (link_dest->getKind ()) {
	case destXYZ:
		dest->type = POPPLER_DEST_XYZ;
		break;
	case destFit:
		dest->type = POPPLER_DEST_FIT;
		break;
	case destFitH:
		dest->type = POPPLER_DEST_FITH;
		break;
	case destFitV:
		dest->type = POPPLER_DEST_FITV;
		break;
	case destFitR:
		dest->type = POPPLER_DEST_FITR;
		break;
	case destFitB:
		dest->type = POPPLER_DEST_FITB;
		break;
	case destFitBH:
		dest->type = POPPLER_DEST_FITBH;
		break;
	case destFitBV:
		dest->type = POPPLER_DEST_FITBV;
		break;
	default:
		dest->type = POPPLER_DEST_UNKNOWN;
	}

	if (link_dest->isPageRef ()) {
		if (document) {
			Ref page_ref = link_dest->getPageRef ();
			dest->page_num = document->doc->findPage (page_ref.num, page_ref.gen);
		} else {
			/* FIXME: We don't keep areound the page_ref for the
			 * remote doc, so we can't look this up.  Guess that
			 * it's 0*/
			dest->page_num = 0;
		}
	} else {
		dest->page_num = link_dest->getPageNum ();
	}

	dest->left = link_dest->getLeft ();
	dest->bottom = link_dest->getBottom ();
	dest->right = link_dest->getRight ();
	dest->top = link_dest->getTop ();
	dest->zoom = link_dest->getZoom ();
	dest->change_left = link_dest->getChangeLeft ();
	dest->change_top = link_dest->getChangeTop ();
	dest->change_zoom = link_dest->getChangeZoom ();
	
	if (document && dest->page_num > 0) {
		PopplerPage *page;

		page = poppler_document_get_page (document, dest->page_num - 1);

		if (page) {
			dest->left -= page->page->getCropBox ()->x1;
			dest->bottom -= page->page->getCropBox ()->x1;
			dest->right -= page->page->getCropBox ()->y1;
			dest->top -= page->page->getCropBox ()->y1;

			g_object_unref (page);
		} else {
			g_warning ("Invalid page %d in Link Destination\n", dest->page_num);
			dest->page_num = 0;
		}
	}
	
	return dest;
}

static PopplerDest *
dest_new_named (GooString *named_dest)
{
	PopplerDest *dest;

	dest = g_slice_new0 (PopplerDest);

	if (named_dest == NULL) {
		dest->type = POPPLER_DEST_UNKNOWN;
		return dest;
	}

	dest->type = POPPLER_DEST_NAMED;
	dest->named_dest = g_strdup (named_dest->getCString ());

	return dest;
}

static void
build_goto_dest (PopplerDocument *document,
		 PopplerAction   *action,
		 LinkGoTo        *link)
{
	LinkDest *link_dest;
	GooString *named_dest;

	/* Return if it isn't OK */
	if (! link->isOk ()) {
		action->goto_dest.dest = dest_new_goto (NULL, NULL);
		return;
	}
	
	link_dest = link->getDest ();
	named_dest = link->getNamedDest ();

	if (link_dest != NULL) {
		action->goto_dest.dest = dest_new_goto (document, link_dest);
	} else if (named_dest != NULL) {
		action->goto_dest.dest = dest_new_named (named_dest);
	} else {
		action->goto_dest.dest = dest_new_goto (document, NULL);
	}
}

static void
build_goto_remote (PopplerAction *action,
		   LinkGoToR     *link)
{
	LinkDest *link_dest;
	GooString *named_dest;
	
	/* Return if it isn't OK */
	if (! link->isOk ()) {
		action->goto_remote.dest = dest_new_goto (NULL, NULL);
		return;
	}

	action->goto_remote.file_name = _poppler_goo_string_to_utf8 (link->getFileName());

	link_dest = link->getDest ();
	named_dest = link->getNamedDest ();
	
	if (link_dest != NULL) {
		action->goto_remote.dest = dest_new_goto (NULL, link_dest);
	} else if (named_dest != NULL) {
		action->goto_remote.dest = dest_new_named (named_dest);
	} else {
		action->goto_remote.dest = dest_new_goto (NULL, NULL);
	}
}

static void
build_launch (PopplerAction *action,
	      LinkLaunch    *link)
{
	if (link->getFileName()) {
		action->launch.file_name = g_strdup (link->getFileName()->getCString ());
	}
	if (link->getParams()) {
		action->launch.params = g_strdup (link->getParams()->getCString ());
	}
}

static void
build_uri (PopplerAction *action,
	   LinkURI       *link)
{
	gchar *uri;

	uri = link->getURI()->getCString ();
	if (uri != NULL)
		action->uri.uri = g_strdup (uri);
}

static void
build_named (PopplerAction *action,
	     LinkNamed     *link)
{
	gchar *name;

	name = link->getName ()->getCString ();
	if (name != NULL)
		action->named.named_dest = g_strdup (name);
}

static AnnotMovie *
find_annot_movie_for_action (PopplerDocument *document,
			     LinkMovie       *link)
{
  AnnotMovie *annot = NULL;
  XRef *xref = document->doc->getXRef ();
  Object annotObj;

  if (link->hasAnnotRef ()) {
    Ref *ref = link->getAnnotRef ();

    xref->fetch (ref->num, ref->gen, &annotObj);
  } else if (link->hasAnnotTitle ()) {
    Object annots;
    GooString *title = link->getAnnotTitle ();
    int i;

    for (i = 1; i <= document->doc->getNumPages (); ++i) {
      Page *p = document->doc->getPage (i);
      if (!p) continue;

      if (p->getAnnots (&annots)->isArray ()) {
        int j;
	GBool found = gFalse;

	for (j = 0; j < annots.arrayGetLength () && !found; ++j) {
          if (annots.arrayGet(j, &annotObj)->isDict()) {
	    Object obj1;

	    if (!annotObj.dictLookup ("Subtype", &obj1)->isName ("Movie")) {
	      obj1.free ();
	      continue;
	    }
	    obj1.free ();

	    if (annotObj.dictLookup ("T", &obj1)->isString()) {
	      GooString *t = obj1.getString ();

	      if (title->cmp(t) == 0)
	        found = gTrue;
	    }
	    obj1.free ();
	  }
	  if (!found)
	    annotObj.free ();
	}
	if (found) {
	  annots.free ();
	  break;
	} else {
          annotObj.free ();
	}
      }
      annots.free ();
    }
  }

  if (annotObj.isDict ()) {
    Object tmp;

    annot = new AnnotMovie (xref, annotObj.getDict(), document->doc->getCatalog (), &tmp);
    if (!annot->isOk ()) {
      delete annot;
      annot = NULL;
    }
  }
  annotObj.free ();

  return annot;
}

static void
build_movie (PopplerDocument *document,
	     PopplerAction   *action,
	     LinkMovie       *link)
{
	AnnotMovie *annot;

	switch (link->getOperation ()) {
	case LinkMovie::operationTypePause:
		action->movie.operation = POPPLER_ACTION_MOVIE_PAUSE;
		break;
	case LinkMovie::operationTypeResume:
		action->movie.operation = POPPLER_ACTION_MOVIE_RESUME;
		break;
	case LinkMovie::operationTypeStop:
		action->movie.operation = POPPLER_ACTION_MOVIE_STOP;
		break;
	default:
	case LinkMovie::operationTypePlay:
		action->movie.operation = POPPLER_ACTION_MOVIE_PLAY;
		break;
	}

	annot = find_annot_movie_for_action (document, link);
	if (annot) {
		action->movie.movie = _poppler_movie_new (annot->getMovie());
		delete annot;
	}
}

static void
build_rendition (PopplerAction *action,
		 LinkRendition *link)
{
	action->rendition.op = link->getOperation();
	if (link->hasRenditionObject())
		action->rendition.media = _poppler_media_new (link->getMedia());
	// TODO: annotation reference
}

static PopplerLayer *
get_layer_for_ref (PopplerDocument *document,
		   GList           *layers,
		   Ref             *ref,
		   gboolean         preserve_rb)
{
	GList *l;

	for (l = layers; l; l = g_list_next (l)) {
		Layer *layer = (Layer *)l->data;

		if (layer->oc) {
			Ref ocgRef = layer->oc->getRef();

			if (ref->num == ocgRef.num && ref->gen == ocgRef.gen) {
				GList *rb_group = NULL;

				if (preserve_rb)
					rb_group = _poppler_document_get_layer_rbgroup (document, layer);
				return _poppler_layer_new (document, layer, rb_group);
			}
		}

		if (layer->kids) {
			PopplerLayer *retval = get_layer_for_ref (document, layer->kids, ref, preserve_rb);
			if (retval)
				return retval;
		}
	}

	return NULL;
}

static void
build_ocg_state (PopplerDocument *document,
		 PopplerAction   *action,
		 LinkOCGState    *ocg_state)
{
	GooList *st_list = ocg_state->getStateList();
	GBool    preserve_rb = ocg_state->getPreserveRB();
	gint     i, j;
	GList   *layer_state = NULL;

	if (!document->layers) {
		if (!_poppler_document_get_layers (document))
			return;
	}

	for (i = 0; i < st_list->getLength(); ++i) {
		LinkOCGState::StateList *list = (LinkOCGState::StateList *)st_list->get(i);
		PopplerActionLayer *action_layer = g_new0 (PopplerActionLayer, 1);

		switch (list->st) {
		case LinkOCGState::On:
			action_layer->action = POPPLER_ACTION_LAYER_ON;
			break;
		case LinkOCGState::Off:
			action_layer->action = POPPLER_ACTION_LAYER_OFF;
			break;
		case LinkOCGState::Toggle:
			action_layer->action = POPPLER_ACTION_LAYER_TOGGLE;
			break;
		}

		for (j = 0; j < list->list->getLength(); ++j) {
			Ref *ref = (Ref *)list->list->get(j);
			PopplerLayer *layer = get_layer_for_ref (document, document->layers, ref, preserve_rb);

			action_layer->layers = g_list_prepend (action_layer->layers, layer);
		}

		layer_state = g_list_prepend (layer_state, action_layer);
	}

	action->ocg_state.state_list = g_list_reverse (layer_state);
}

PopplerAction *
_poppler_action_new (PopplerDocument *document,
		     LinkAction      *link,
		     const gchar     *title)
{
	PopplerAction *action;

	action = g_slice_new0 (PopplerAction);

	if (title)
		action->any.title = g_strdup (title);

	if (link == NULL) {
		action->type = POPPLER_ACTION_NONE;
		return action;
	}

	switch (link->getKind ()) {
	case actionGoTo:
		action->type = POPPLER_ACTION_GOTO_DEST;
		build_goto_dest (document, action, dynamic_cast <LinkGoTo *> (link));
		break;
	case actionGoToR:
		action->type = POPPLER_ACTION_GOTO_REMOTE;
		build_goto_remote (action, dynamic_cast <LinkGoToR *> (link));
		break;
	case actionLaunch:
		action->type = POPPLER_ACTION_LAUNCH;
		build_launch (action, dynamic_cast <LinkLaunch *> (link));
		break;
	case actionURI:
		action->type = POPPLER_ACTION_URI;
		build_uri (action, dynamic_cast <LinkURI *> (link));
		break;
	case actionNamed:
		action->type = POPPLER_ACTION_NAMED;
		build_named (action, dynamic_cast <LinkNamed *> (link));
		break;
	case actionMovie:
		action->type = POPPLER_ACTION_MOVIE;
		build_movie (document, action, dynamic_cast<LinkMovie*> (link));
		break;
	case actionRendition:
		action->type = POPPLER_ACTION_RENDITION;
		build_rendition (action, dynamic_cast<LinkRendition*> (link));
		break;
	case actionOCGState:
		action->type = POPPLER_ACTION_OCG_STATE;
		build_ocg_state (document, action, dynamic_cast<LinkOCGState*> (link));
		break;
	case actionUnknown:
	default:
		action->type = POPPLER_ACTION_UNKNOWN;
		break;
	}

	return action;
}

PopplerDest *
_poppler_dest_new_goto (PopplerDocument *document,
			LinkDest        *link_dest)
{
	return dest_new_goto (document, link_dest);
}
