/* poppler-link.h: qt interface to poppler
 * Copyright (C) 2006, Albert Astals Cid <aacid@kde.org>
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

#include <qstring.h>
#include <qrect.h>

namespace Poppler {

class LinkDestinationData;

class LinkDestination
{
	public:
		enum Kind
		{
			destXYZ = 1,
			destFit = 2,
			destFitH = 3,
			destFitV = 4,
			destFitR = 5,
			destFitB = 6,
			destFitBH = 7,
			destFitBV = 8
		};

		LinkDestination(const LinkDestinationData &data);
		LinkDestination(const QString &description);

		// Accessors.
		Kind kind() const;
		int pageNumber() const;
		double left() const;
		double bottom() const;
		double right() const;
		double top() const;
		double zoom() const;
		bool isChangeLeft() const;
		bool isChangeTop() const;
		bool isChangeZoom() const;

		QString toString() const;

	private:
		Kind m_kind; // destination type
		int m_pageNum; // page number
		double m_left, m_bottom; // position
		double m_right, m_top;
		double m_zoom; // zoom factor
		bool m_changeLeft, m_changeTop; // for destXYZ links, which position
		bool m_changeZoom; //   components to change
};

/**
 * @short Encapsulates data that describes a link.
 *
 * This is the base class for links. It makes mandatory for inherited
 * widgets to reimplement the 'linkType' method and return the type of
 * the link described by the reimplemented class.
 */
class Link
{
	public:
		Link( const QRect &linkArea );
		
		// get link type (inherited classes mustreturn an unique identifier)
		enum LinkType { None, Goto, Execute, Browse, Action, Movie };
		virtual LinkType linkType() const;

		// virtual destructor
		virtual ~Link();
		
		QRect linkArea() const;
		
	private:
		QRect m_linkArea;
};


/** Goto: a viewport and maybe a reference to an external filename **/
class LinkGoto : public Link
{
	public:
		LinkGoto( const QRect &linkArea, QString extFileName, const LinkDestination & destination );
		
		// query for goto parameters
		bool isExternal() const;
		const QString & fileName() const;
		const LinkDestination & destination() const;
		LinkType linkType() const;

	private:
		QString m_extFileName;
		LinkDestination m_destination;
};

/** Execute: filename and parameters to execute **/
class LinkExecute : public Link
{
	public:
		// query for filename / parameters
		const QString & fileName() const;
		const QString & parameters() const;

		// create a Link_Execute
		LinkExecute( const QRect &linkArea, const QString & file, const QString & params );
		LinkType linkType() const;

	private:
		QString m_fileName;
		QString m_parameters;
};

/** Browse: an URL to open, ranging from 'http://' to 'mailto:' etc.. **/
class LinkBrowse : public Link
{
	public:
		// query for URL
		const QString & url() const;

		// create a Link_Browse
		LinkBrowse( const QRect &linkArea, const QString &url );
		LinkType linkType() const;

	private:
		QString m_url;
};	

/** Action: contains an action to perform on document / viewer **/
class LinkAction : public Link
{
	public:
		// define types of actions
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
		                  Close = 12 };

		// query for action type
		ActionType actionType() const;

		// create a Link_Action
		LinkAction( const QRect &linkArea, ActionType actionType );
		LinkType linkType() const;

	private:
		ActionType m_type;
};

/** Movie: Not yet defined -> think renaming to 'Media' link **/
class LinkMovie : public Link
// TODO this (Movie link)
{
	public:
		LinkMovie( const QRect &linkArea );
		LinkType linkType() const;
};

}

#endif
