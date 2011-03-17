/* poppler-link.cc: qt interface to poppler
 * Copyright (C) 2006-2007, Albert Astals Cid
 * Copyright (C) 2007-2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
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

#include <poppler-qt4.h>
#include <poppler-private.h>

#include <QtCore/QStringList>

#include "Link.h"

namespace Poppler {

class LinkDestinationPrivate : public QSharedData
{
	public:
		LinkDestinationPrivate();

		LinkDestination::Kind kind; // destination type
		QString name;
		int pageNum; // page number
		double left, bottom; // position
		double right, top;
		double zoom; // zoom factor
		bool changeLeft : 1, changeTop : 1; // for destXYZ links, which position
		bool changeZoom : 1; //   components to change
};

	LinkDestinationPrivate::LinkDestinationPrivate()
	{
		// sane defaults
		kind = LinkDestination::destXYZ;
		pageNum = 0;
		left = 0;
		bottom = 0;
		right = 0;
		top = 0;
		zoom = 1;
		changeLeft = true;
		changeTop = true;
		changeZoom = false;
	}

class LinkPrivate
{
	public:
		LinkPrivate( const QRectF &area );
		virtual ~LinkPrivate();

		QRectF linkArea;
};

	LinkPrivate::LinkPrivate( const QRectF &area )
		: linkArea( area )
	{
	}

	LinkPrivate::~LinkPrivate()
	{
	}

class LinkGotoPrivate : public LinkPrivate
{
	public:
		LinkGotoPrivate( const QRectF &area, const LinkDestination &dest );

		QString extFileName;
		LinkDestination destination;
};

	LinkGotoPrivate::LinkGotoPrivate( const QRectF &area, const LinkDestination &dest )
		: LinkPrivate( area ), destination( dest )
	{
	}

class LinkExecutePrivate : public LinkPrivate
{
	public:
		LinkExecutePrivate( const QRectF &area );

		QString fileName;
		QString parameters;
};

	LinkExecutePrivate::LinkExecutePrivate( const QRectF &area )
		: LinkPrivate( area )
	{
	}

class LinkBrowsePrivate : public LinkPrivate
{
	public:
		LinkBrowsePrivate( const QRectF &area );

		QString url;
};

	LinkBrowsePrivate::LinkBrowsePrivate( const QRectF &area )
		: LinkPrivate( area )
	{
	}

class LinkActionPrivate : public LinkPrivate
{
	public:
		LinkActionPrivate( const QRectF &area );

		LinkAction::ActionType type;
};

	LinkActionPrivate::LinkActionPrivate( const QRectF &area )
		: LinkPrivate( area )
	{
	}

class LinkSoundPrivate : public LinkPrivate
{
	public:
		LinkSoundPrivate( const QRectF &area );
		~LinkSoundPrivate();

		double volume;
		bool sync : 1;
		bool repeat : 1;
		bool mix : 1;
		SoundObject *sound;
};

	LinkSoundPrivate::LinkSoundPrivate( const QRectF &area )
		: LinkPrivate( area ), sound( 0 )
	{
	}

	LinkSoundPrivate::~LinkSoundPrivate()
	{
		delete sound;
	}

class LinkJavaScriptPrivate : public LinkPrivate
{
	public:
		LinkJavaScriptPrivate( const QRectF &area );

		QString js;
};

	LinkJavaScriptPrivate::LinkJavaScriptPrivate( const QRectF &area )
		: LinkPrivate( area )
	{
	}

#if 0
class LinkMoviePrivate : public LinkPrivate
{
	public:
		LinkMoviePrivate( const QRectF &area );
};

	LinkMoviePrivate::LinkMoviePrivate( const QRectF &area )
		: LinkPrivate( area )
	{
	}
#endif

	static void cvtUserToDev(::Page *page, double xu, double yu, int *xd, int *yd) {
		double ctm[6];
		
		page->getDefaultCTM(ctm, 72.0, 72.0, 0, false, true);
		*xd = (int)(ctm[0] * xu + ctm[2] * yu + ctm[4] + 0.5);
		*yd = (int)(ctm[1] * xu + ctm[3] * yu + ctm[5] + 0.5);
	}

	LinkDestination::LinkDestination(const LinkDestinationData &data)
		: d( new LinkDestinationPrivate )
	{
		bool deleteDest = false;
		LinkDest *ld = data.ld;
		
		if ( data.namedDest && !ld && !data.externalDest )
		{
			deleteDest = true;
			ld = data.doc->doc->findDest( data.namedDest );
		}
		// in case this destination was named one, and it was not resolved
		if ( data.namedDest && !ld )
		{
			d->name = QString::fromLatin1( data.namedDest->getCString() );
		}
		
		if (!ld) return;
		
		if (ld->getKind() == ::destXYZ) d->kind = destXYZ;
		else if (ld->getKind() == ::destFit) d->kind = destFit;
		else if (ld->getKind() == ::destFitH) d->kind = destFitH;
		else if (ld->getKind() == ::destFitV) d->kind = destFitV;
		else if (ld->getKind() == ::destFitR) d->kind = destFitR;
		else if (ld->getKind() == ::destFitB) d->kind = destFitB;
		else if (ld->getKind() == ::destFitBH) d->kind = destFitBH;
		else if (ld->getKind() == ::destFitBV) d->kind = destFitBV;

		if ( !ld->isPageRef() ) d->pageNum = ld->getPageNum();
		else
		{
			Ref ref = ld->getPageRef();
			d->pageNum = data.doc->doc->findPage( ref.num, ref.gen );
		}
		double left = ld->getLeft();
		double bottom = ld->getBottom();
		double right = ld->getRight();
		double top = ld->getTop();
		d->zoom = ld->getZoom();
		d->changeLeft = ld->getChangeLeft();
		d->changeTop = ld->getChangeTop();
		d->changeZoom = ld->getChangeZoom();
		
		int leftAux = 0, topAux = 0, rightAux = 0, bottomAux = 0;
		
		::Page *page;
		if (d->pageNum > 0 &&
		    d->pageNum <= data.doc->doc->getNumPages() &&
		    (page = data.doc->doc->getPage( d->pageNum )))
		{
			cvtUserToDev( page, left, top, &leftAux, &topAux );
			cvtUserToDev( page, right, bottom, &rightAux, &bottomAux );
			
			d->left = leftAux / (double)page->getCropWidth();
			d->top = topAux / (double)page->getCropHeight();
			d->right = rightAux/ (double)page->getCropWidth();
			d->bottom = bottomAux / (double)page->getCropHeight();
		}
		else d->pageNum = 0;
		
		if (deleteDest) delete ld;
	}
	
	LinkDestination::LinkDestination(const QString &description)
		: d( new LinkDestinationPrivate )
	{
		QStringList tokens = description.split( ';' );
		d->kind = static_cast<Kind>(tokens.at(0).toInt());
		d->pageNum = tokens.at(1).toInt();
		d->left = tokens.at(2).toDouble();
		d->bottom = tokens.at(3).toDouble();
		d->right = tokens.at(4).toDouble();
		d->top = tokens.at(5).toDouble();
		d->zoom = tokens.at(6).toDouble();
		d->changeLeft = static_cast<bool>(tokens.at(7).toInt());
		d->changeTop = static_cast<bool>(tokens.at(8).toInt());
		d->changeZoom = static_cast<bool>(tokens.at(9).toInt());
	}
	
	LinkDestination::LinkDestination(const LinkDestination &other)
		: d( other.d )
	{
	}
	
	LinkDestination::~LinkDestination()
	{
	}
	
	LinkDestination::Kind LinkDestination::kind() const
	{
		return d->kind;
	}
	
	int LinkDestination::pageNumber() const
	{
		return d->pageNum;
	}
	
	double LinkDestination::left() const
	{
		return d->left;
	}
	
	double LinkDestination::bottom() const
	{
		return d->bottom;
	}
	
	double LinkDestination::right() const
	{
		return d->right;
	}
	
	double LinkDestination::top() const
	{
		return d->top;
	}
	
	double LinkDestination::zoom() const
	{
		return d->zoom;
	}
	
	bool LinkDestination::isChangeLeft() const
	{
		return d->changeLeft;
	}
	
	bool LinkDestination::isChangeTop() const
	{
		return d->changeTop;
	}
	
	bool LinkDestination::isChangeZoom() const
	{
		return d->changeZoom;
	}
	
	QString LinkDestination::toString() const
	{
		QString s = QString::number( (qint8)d->kind );
		s += ";" + QString::number( d->pageNum );
		s += ";" + QString::number( d->left );
		s += ";" + QString::number( d->bottom );
		s += ";" + QString::number( d->right );
		s += ";" + QString::number( d->top );
		s += ";" + QString::number( d->zoom );
		s += ";" + QString::number( (qint8)d->changeLeft );
		s += ";" + QString::number( (qint8)d->changeTop );
		s += ";" + QString::number( (qint8)d->changeZoom );
		return s;
	}
	
	QString LinkDestination::destinationName() const
	{
		return d->name;
	}
	
	LinkDestination& LinkDestination::operator=(const LinkDestination &other)
	{
		if ( this == &other )
			return *this;
		
		d = other.d;
		return *this;
	}
	
	
	// Link
	Link::~Link()
	{
		delete d_ptr;
	}
	
	Link::Link(const QRectF &linkArea)
		: d_ptr( new LinkPrivate( linkArea ) )
	{
	}
	
	Link::Link( LinkPrivate &dd )
		: d_ptr( &dd )
	{
	}

	Link::LinkType Link::linkType() const
	{
		return None;
	}
	
	QRectF Link::linkArea() const
	{
		Q_D( const Link );
		return d->linkArea;
	}
	
	// LinkGoto
	LinkGoto::LinkGoto( const QRectF &linkArea, QString extFileName, const LinkDestination & destination )
		: Link( *new LinkGotoPrivate( linkArea, destination ) )
	{
		Q_D( LinkGoto );
		d->extFileName = extFileName;
	}
	
	LinkGoto::~LinkGoto()
	{
	}
	
	bool LinkGoto::isExternal() const
	{
		Q_D( const LinkGoto );
		return !d->extFileName.isEmpty();
	}
	
	QString LinkGoto::fileName() const
	{
		Q_D( const LinkGoto );
		return d->extFileName;
	}
	
	LinkDestination LinkGoto::destination() const
	{
		Q_D( const LinkGoto );
		return d->destination;
	}
	
	Link::LinkType LinkGoto::linkType() const
	{
		return Goto;
	}
	
	// LinkExecute
	LinkExecute::LinkExecute( const QRectF &linkArea, const QString & file, const QString & params )
		: Link( *new LinkExecutePrivate( linkArea ) )
	{
		Q_D( LinkExecute );
		d->fileName = file;
		d->parameters = params;
	}
	
	LinkExecute::~LinkExecute()
	{
	}
	
	QString LinkExecute::fileName() const
	{
		Q_D( const LinkExecute );
		return d->fileName;
	}
	QString LinkExecute::parameters() const
	{
		Q_D( const LinkExecute );
		return d->parameters;
	}

	Link::LinkType LinkExecute::linkType() const
	{
		return Execute;
	}

	// LinkBrowse
	LinkBrowse::LinkBrowse( const QRectF &linkArea, const QString &url )
		: Link( *new LinkBrowsePrivate( linkArea ) )
	{
		Q_D( LinkBrowse );
		d->url = url;
	}
	
	LinkBrowse::~LinkBrowse()
	{
	}
	
	QString LinkBrowse::url() const
	{
		Q_D( const LinkBrowse );
		return d->url;
	}
	
	Link::LinkType LinkBrowse::linkType() const
	{
		return Browse;
	}

	// LinkAction
	LinkAction::LinkAction( const QRectF &linkArea, ActionType actionType )
		: Link( *new LinkActionPrivate( linkArea ) )
	{
		Q_D( LinkAction );
		d->type = actionType;
	}
		
	LinkAction::~LinkAction()
	{
	}
	
	LinkAction::ActionType LinkAction::actionType() const
	{
		Q_D( const LinkAction );
		return d->type;
	}

	Link::LinkType LinkAction::linkType() const
	{
		return Action;
	}

	// LinkSound
	LinkSound::LinkSound( const QRectF &linkArea, double volume, bool sync, bool repeat, bool mix, SoundObject *sound )
		: Link( *new LinkSoundPrivate( linkArea ) )
	{
		Q_D( LinkSound );
		d->volume = volume;
		d->sync = sync;
		d->repeat = repeat;
		d->mix = mix;
		d->sound = sound;
	}
	
	LinkSound::~LinkSound()
	{
	}
	
	Link::LinkType LinkSound::linkType() const
	{
		return Sound;
	}

	double LinkSound::volume() const
	{
		Q_D( const LinkSound );
		return d->volume;
	}

	bool LinkSound::synchronous() const
	{
		Q_D( const LinkSound );
		return d->sync;
	}

	bool LinkSound::repeat() const
	{
		Q_D( const LinkSound );
		return d->repeat;
	}

	bool LinkSound::mix() const
	{
		Q_D( const LinkSound );
		return d->mix;
	}

	SoundObject *LinkSound::sound() const
	{
		Q_D( const LinkSound );
		return d->sound;
	}

	// LinkJavaScript
	LinkJavaScript::LinkJavaScript( const QRectF &linkArea, const QString &js )
		: Link( *new LinkJavaScriptPrivate( linkArea ) )
	{
		Q_D( LinkJavaScript );
		d->js = js;
	}
	
	LinkJavaScript::~LinkJavaScript()
	{
	}
	
	Link::LinkType LinkJavaScript::linkType() const
	{
		return JavaScript;
	}
	
	QString LinkJavaScript::script() const
	{
		Q_D( const LinkJavaScript );
		return d->js;
	}

#if 0
	// LinkMovie
	LinkMovie::LinkMovie( const QRectF &linkArea )
		: Link( *new LinkMoviePrivate( linkArea ) )
	{
	}
	
	LinkMovie::~LinkMovie()
	{
	}
	
	Link::LinkType LinkMovie::linkType() const
	{
		return Movie;
	}
#endif

}
