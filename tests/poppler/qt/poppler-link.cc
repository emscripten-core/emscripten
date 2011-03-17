/* poppler-link.cc: qt interface to poppler
 * Copyright (C) 2006, 2008 Albert Astals Cid
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

#include <poppler-qt.h>
#include <poppler-private.h>

#include <qstringlist.h>

#include <Link.h>

namespace Poppler {

	LinkDestination::LinkDestination(const LinkDestinationData &data)
	{
		bool deleteDest = false;
		LinkDest *ld = data.ld;
		
		if ( data.namedDest && !ld )
		{
			deleteDest = true;
			ld = data.doc->doc.findDest( data.namedDest );
		}
		
		if (!ld) return;
		
		if (ld->getKind() == ::destXYZ) m_kind = destXYZ;
		else if (ld->getKind() == ::destFit) m_kind = destFit;
		else if (ld->getKind() == ::destFitH) m_kind = destFitH;
		else if (ld->getKind() == ::destFitV) m_kind = destFitV;
		else if (ld->getKind() == ::destFitR) m_kind = destFitR;
		else if (ld->getKind() == ::destFitB) m_kind = destFitB;
		else if (ld->getKind() == ::destFitBH) m_kind = destFitBH;
		else if (ld->getKind() == ::destFitBV) m_kind = destFitBV;

		if ( !ld->isPageRef() ) m_pageNum = ld->getPageNum();
		else
		{
			Ref ref = ld->getPageRef();
			m_pageNum = data.doc->doc.findPage( ref.num, ref.gen );
		}
		double left = ld->getLeft();
		double bottom = ld->getBottom();
		double right = ld->getRight();
		double top = ld->getTop();
		m_zoom = ld->getZoom();
		m_changeLeft = ld->getChangeLeft();
		m_changeTop = ld->getChangeTop();
		m_changeZoom = ld->getChangeZoom();
		
		int leftAux = 0, topAux = 0, rightAux = 0, bottomAux = 0;
		
#if defined(HAVE_SPLASH)
		SplashOutputDev *sod = data.doc->getOutputDev();
		sod->cvtUserToDev( left, top, &leftAux, &topAux );
		sod->cvtUserToDev( right, bottom, &rightAux, &bottomAux );
#endif
		
		m_left = leftAux;
		m_top = topAux;
		m_right = rightAux;
		m_bottom = bottomAux;
		
		if (deleteDest) delete ld;
	}
	
	LinkDestination::LinkDestination(const QString &description)
	{
		QStringList tokens = QStringList::split(';', description);
		m_kind = static_cast<Kind>(tokens[0].toInt());
		m_pageNum = tokens[1].toInt();
		m_left = tokens[2].toDouble();
		m_bottom = tokens[3].toDouble();
		m_right = tokens[4].toDouble();
		m_top = tokens[5].toDouble();
		m_zoom = tokens[6].toDouble();
		m_changeLeft = static_cast<bool>(tokens[7].toInt());
		m_changeTop = static_cast<bool>(tokens[8].toInt());
		m_changeZoom = static_cast<bool>(tokens[9].toInt());
	}
	
	LinkDestination::Kind LinkDestination::kind() const
	{
		return m_kind;
	}
	
	int LinkDestination::pageNumber() const
	{
		return m_pageNum;
	}
	
	double LinkDestination::left() const
	{
		return m_left;
	}
	
	double LinkDestination::bottom() const
	{
		return m_bottom;
	}
	
	double LinkDestination::right() const
	{
		return m_right;
	}
	
	double LinkDestination::top() const
	{
		return m_top;
	}
	
	double LinkDestination::zoom() const
	{
		return m_zoom;
	}
	
	bool LinkDestination::isChangeLeft() const
	{
		return m_changeLeft;
	}
	
	bool LinkDestination::isChangeTop() const
	{
		return m_changeTop;
	}
	
	bool LinkDestination::isChangeZoom() const
	{
		return m_changeZoom;
	}
	
	QString LinkDestination::toString() const
	{
		QString s = QString::number( (Q_INT8)m_kind );
		s += ";" + QString::number( m_pageNum );
		s += ";" + QString::number( m_left );
		s += ";" + QString::number( m_bottom );
		s += ";" + QString::number( m_right );
		s += ";" + QString::number( m_top );
		s += ";" + QString::number( m_zoom );
		s += ";" + QString::number( (Q_INT8)m_changeLeft );
		s += ";" + QString::number( (Q_INT8)m_changeTop );
		s += ";" + QString::number( (Q_INT8)m_changeZoom );
		return s;
	}
	
	
	// Link
	Link::~Link()
	{
	}
	
	Link::Link(const QRect &linkArea) : m_linkArea(linkArea)
	{
	}
	
	Link::LinkType Link::linkType() const
	{
		return None;
	}
	
	QRect Link::linkArea() const
	{
		return m_linkArea;
	}
	
	// LinkGoto
	LinkGoto::LinkGoto( const QRect &linkArea, QString extFileName, const LinkDestination & destination ) : Link(linkArea), m_extFileName(extFileName), m_destination(destination)
	{
	}
	
	bool LinkGoto::isExternal() const
	{
		return !m_extFileName.isEmpty();
	}
	
	const QString &LinkGoto::fileName() const
	{
		return m_extFileName;
	}
	
	const LinkDestination &LinkGoto::destination() const
	{
		return m_destination;
	}
	
	Link::LinkType LinkGoto::linkType() const
	{
		return Goto;
	}
	
	// LinkExecute
	LinkExecute::LinkExecute( const QRect &linkArea, const QString & file, const QString & params ) : Link(linkArea), m_fileName(file), m_parameters(params)
	{
	}
	
	const QString & LinkExecute::fileName() const
	{
		return m_fileName;
	}
	const QString & LinkExecute::parameters() const
	{
		return m_parameters;
	}

	Link::LinkType LinkExecute::linkType() const
	{
		return Execute;
	}

	// LinkBrowse
	LinkBrowse::LinkBrowse( const QRect &linkArea, const QString &url ) : Link(linkArea), m_url(url)
	{
	}
	
	const QString & LinkBrowse::url() const
	{
		return m_url;
	}
	
	Link::LinkType LinkBrowse::linkType() const
	{
		return Browse;
	}

	// LinkAction
	LinkAction::LinkAction( const QRect &linkArea, ActionType actionType ) : Link(linkArea), m_type(actionType)
	{
	}
		
	LinkAction::ActionType LinkAction::actionType() const
	{
		return m_type;
	}

	Link::LinkType LinkAction::linkType() const
	{
		return Action;
	}

	// LinkMovie
	LinkMovie::LinkMovie( const QRect &linkArea ) : Link(linkArea)
	{
	}
	
	Link::LinkType LinkMovie::linkType() const
	{
		return Movie;
	}

}
