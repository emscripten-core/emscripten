/* poppler-link.h: qt interface to poppler
 * Copyright (C) 2006, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2007-2008, 2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2010, Guillermo Amaral <gamaral@kdab.com>
 * Adapting code from
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

#ifndef _POPPLER_LINK_H_
#define _POPPLER_LINK_H_

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QSharedDataPointer>
#include "poppler-export.h"

namespace Poppler {

class LinkPrivate;
class LinkGotoPrivate;
class LinkExecutePrivate;
class LinkBrowsePrivate;
class LinkActionPrivate;
class LinkSoundPrivate;
class LinkJavaScriptPrivate;
class LinkMoviePrivate;
class LinkDestinationData;
class LinkDestinationPrivate;
class SoundObject;

/**
 * \short A destination.
 *
 * The LinkDestination class represent a "destination" (in terms of visual
 * viewport to be displayed) for \link Poppler::LinkGoto GoTo\endlink links,
 * and items in the table of contents (TOC) of a document.
 *
 * Coordinates are in 0..1 range
 */
class POPPLER_QT4_EXPORT LinkDestination
{
	public:
		/**
		 * The possible kind of "viewport destination".
		 */
		enum Kind
		{
			/**
			 * The new viewport is specified in terms of:
			 * - possibile new left coordinate (see isChangeLeft() )
			 * - possibile new top coordinate (see isChangeTop() )
			 * - possibile new zoom level (see isChangeZoom() )
			 */
			destXYZ = 1,
			destFit = 2,
			destFitH = 3,
			destFitV = 4,
			destFitR = 5,
			destFitB = 6,
			destFitBH = 7,
			destFitBV = 8
		};

		/// \cond PRIVATE
		LinkDestination(const LinkDestinationData &data);
		LinkDestination(const QString &description);
		/// \endcond
		/**
		 * Copy constructor.
		 */
		LinkDestination(const LinkDestination &other);
		/**
		 * Destructor.
		 */
		~LinkDestination();

		// Accessors.
		/**
		 * The kind of destination.
		 */
		Kind kind() const;
		/**
		 * Which page is the target of this destination.
		 *
		 * \note this number is 1-based, so for a 5 pages document the
		 *       valid page numbers go from 1 to 5 (both included).
		 */
		int pageNumber() const;
		/**
		 * The new left for the viewport of the target page, in case
		 * it is specified to be changed (see isChangeLeft() )
		 */
		double left() const;
		double bottom() const;
		double right() const;
		/**
		 * The new top for the viewport of the target page, in case
		 * it is specified to be changed (see isChangeTop() )
		 */
		double top() const;
		double zoom() const;
		/**
		 * Whether the left of the viewport on the target page should
		 * be changed.
		 *
		 * \see left()
		 */
		bool isChangeLeft() const;
		/**
		 * Whether the top of the viewport on the target page should
		 * be changed.
		 *
		 * \see top()
		 */
		bool isChangeTop() const;
		/**
		 * Whether the zoom level should be changed.
		 *
		 * \see zoom()
		 */
		bool isChangeZoom() const;

		/**
		 * Return a string repesentation of this destination.
		 */
		QString toString() const;

		/**
		 * Return the name of this destination.
		 *
		 * \since 0.12
		 */
		QString destinationName() const;

		/**
		 * Assignment operator.
		 */
		LinkDestination& operator=(const LinkDestination &other);

	private:
		QSharedDataPointer< LinkDestinationPrivate > d;
};

/**
 * \short Encapsulates data that describes a link.
 *
 * This is the base class for links. It makes mandatory for inherited
 * kind of links to reimplement the linkType() method and return the type of
 * the link described by the reimplemented class.
 */
class POPPLER_QT4_EXPORT Link
{
	public:
		/// \cond PRIVATE
		Link( const QRectF &linkArea );
		/// \endcond
		
		/**
		 * The possible kinds of link.
		 *
		 * Inherited classes must return an unique identifier
		 */
		enum LinkType
		{
		    None,     ///< Unknown link
		    Goto,     ///< A "Go To" link
		    Execute,  ///< A command to be executed
		    Browse,   ///< An URL to be browsed (eg "http://poppler.freedesktop.org")
		    Action,   ///< A "standard" action to be executed in the viewer
		    Sound,    ///< A link representing a sound to be played
		    Movie,    ///< An action to be executed on a movie
		    JavaScript    ///< A JavaScript code to be interpreted \since 0.10
		};

		/**
		 * The type of this link.
		 */
		virtual LinkType linkType() const;

		/**
		 * Destructor.
		 */
		virtual ~Link();
		
		/**
		 * The area of a Page where the link should be active.
		 *
		 * \note this can be a null rect, in this case the link represents
		 * a general action. The area is given in 0..1 range
		 */
		QRectF linkArea() const;
		
	protected:
		/// \cond PRIVATE
		Link( LinkPrivate &dd );
		Q_DECLARE_PRIVATE( Link )
		LinkPrivate *d_ptr;
		/// \endcond
		
	private:
		Q_DISABLE_COPY( Link )
};


/**
 * \brief Viewport reaching request.
 *
 * With a LinkGoto link, the document requests the specified viewport to be
 * reached (aka, displayed in a viewer). Furthermore, if a file name is specified,
 * then the destination refers to that document (and not to the document the
 * current LinkGoto belongs to).
 */
class POPPLER_QT4_EXPORT LinkGoto : public Link
{
	public:
		/**
		 * Create a new Goto link.
		 *
		 * \param linkArea the active area of the link
		 * \param extFileName if not empty, the file name to be open
		 * \param destination the destination to be reached
		 */
		LinkGoto( const QRectF &linkArea, QString extFileName, const LinkDestination & destination );
		/**
		 * Destructor.
		 */
		~LinkGoto();

		/**
		 * Whether the destination is in an external document
		 * (i.e. not the current document)
		 */
		bool isExternal() const;
		// query for goto parameters
		/**
		 * The file name of the document the destination() refers to,
		 * or an empty string in case it refers to the current document.
		 */
		QString fileName() const;
		/**
		 * The destination to reach.
		 */
		LinkDestination destination() const;
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkGoto )
		Q_DISABLE_COPY( LinkGoto )
};

/**
 * \brief Generic execution request.
 *
 * The LinkExecute link represent a "file name" execution request. The result
 * depends on the \link fileName() file name\endlink:
 * - if it is a document, then it is requested to be open
 * - otherwise, it represents an executable to be run with the specified parameters
 */
class POPPLER_QT4_EXPORT LinkExecute : public Link
{
	public:
		/**
		 * The file name to be executed
		 */
		QString fileName() const;
		/**
		 * The parameters for the command.
		 */
		QString parameters() const;

		/**
		 * Create a new Execute link.
		 *
		 * \param linkArea the active area of the link
		 * \param file the file name to be open, or the program to be execute
		 * \param params the parameters for the program to execute
		 */
		LinkExecute( const QRectF &linkArea, const QString & file, const QString & params );
		/**
		 * Destructor.
		 */
		~LinkExecute();
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkExecute )
		Q_DISABLE_COPY( LinkExecute )
};

/**
 * \brief An URL to browse.
 *
 * The LinkBrowse link holds a URL (eg 'http://poppler.freedesktop.org',
 * 'mailto:john@some.org', etc) to be open.
 *
 * The format of the URL is specified by RFC 2396 (http://www.ietf.org/rfc/rfc2396.txt)
 */
class POPPLER_QT4_EXPORT LinkBrowse : public Link
{
	public:
		/**
		 * The URL to open
		 */
		QString url() const;

		/**
		 * Create a new browse link.
		 *
		 * \param linkArea the active area of the link
		 * \param url the URL to be open
		 */
		LinkBrowse( const QRectF &linkArea, const QString &url );
		/**
		 * Destructor.
		 */
		~LinkBrowse();
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkBrowse )
		Q_DISABLE_COPY( LinkBrowse )
};	

/**
 * \brief "Standard" action request.
 *
 * The LinkAction class represents a link that request a "standard" action
 * to be performed by the viewer on the displayed document.
 */
class POPPLER_QT4_EXPORT LinkAction : public Link
{
	public:
		/**
		 * The possible types of actions
		 */
		enum ActionType { PageFirst = 1,
		                  PagePrev = 2,
		                  PageNext = 3,
		                  PageLast = 4,
		                  HistoryBack = 5,
		                  HistoryForward = 6,
		                  Quit = 7,
		                  Presentation = 8,
		                  EndPresentation = 9,
		                  Find = 10,
		                  GoToPage = 11,
		                  Close = 12,
		                  Print = 13    ///< \since 0.16
		};

		/**
		 * The action of the current LinkAction
		 */
		ActionType actionType() const;

		/**
		 * Create a new Action link, that executes a specified action
		 * on the document.
		 *
		 * \param linkArea the active area of the link
		 * \param actionType which action should be executed
		 */
		LinkAction( const QRectF &linkArea, ActionType actionType );
		/**
		 * Destructor.
		 */
		~LinkAction();
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkAction )
		Q_DISABLE_COPY( LinkAction )
};

/**
 * Sound: a sound to be played.
 *
 * \since 0.6
 */
class POPPLER_QT4_EXPORT LinkSound : public Link
{
	public:
		// create a Link_Sound
		LinkSound( const QRectF &linkArea, double volume, bool sync, bool repeat, bool mix, SoundObject *sound );
		/**
		 * Destructor.
		 */
		virtual ~LinkSound();

		LinkType linkType() const;

		/**
		 * The volume to be used when playing the sound.
		 *
		 * The volume is in the range [ -1, 1 ], where:
		 * - a negative number: no volume (mute)
		 * - 1: full volume
		 */
		double volume() const;
		/**
		 * Whether the playback of the sound should be synchronous
		 * (thus blocking, waiting for the end of the sound playback).
		 */
		bool synchronous() const;
		/**
		 * Whether the sound should be played continuously (that is,
		 * started again when it ends)
		 */
		bool repeat() const;
		/**
		 * Whether the playback of this sound can be mixed with
		 * playbacks with other sounds of the same document.
		 *
		 * \note When false, any other playback must be stopped before
		 *       playing the sound.
		 */
		bool mix() const;
		/**
		 * The sound object to be played
		 */
		SoundObject *sound() const;

	private:
		Q_DECLARE_PRIVATE( LinkSound )
		Q_DISABLE_COPY( LinkSound )
};

/**
 * JavaScript: a JavaScript code to be interpreted.
 *
 * \since 0.10
 */
class POPPLER_QT4_EXPORT LinkJavaScript : public Link
{
	public:
		/**
		 * Create a new JavaScript link.
		 *
		 * \param linkArea the active area of the link
		 * \param js the JS code to be interpreted
		 */
		LinkJavaScript( const QRectF &linkArea, const QString &js );
		/**
		 * Destructor.
		 */
		virtual ~LinkJavaScript();

		LinkType linkType() const;

		/**
		 * The JS code
		 */
		QString script() const;

	private:
		Q_DECLARE_PRIVATE( LinkJavaScript )
		Q_DISABLE_COPY( LinkJavaScript )
};	

#if 0
/** Movie: Not yet defined -> think renaming to 'Media' link **/
class POPPLER_QT4_EXPORT LinkMovie : public Link
// TODO this (Movie link)
{
	public:
		LinkMovie( const QRectF &linkArea );
		~LinkMovie();
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkMovie )
		Q_DISABLE_COPY( LinkMovie )
};
#endif

}

#endif
