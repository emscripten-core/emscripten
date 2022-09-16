/* poppler-sound.cc: qt interface to poppler
 * Copyright (C) 2008, 2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2010, Carlos Garcia Campos <carlosgc@gnome.org>
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

#include "poppler-qt4.h"

#include "Object.h"
#include "Annot.h"
#include "Movie.h"

namespace Poppler
{

class MovieData
{
public:
	MovieData()
	  : m_movieObj( 0 )
	{
	}

	~MovieData()
	{
		delete m_movieObj;
	}

	Movie *m_movieObj;
	QSize m_size;
	int m_rotation;
	MovieObject::PlayMode m_playMode : 3;
	bool m_showControls : 1;
};

MovieObject::MovieObject( AnnotMovie *ann )
{
	m_movieData = new MovieData();
	m_movieData->m_movieObj = ann->getMovie()->copy();

	MovieActivationParameters *mp = m_movieData->m_movieObj->getActivationParameters();
	int width, height;
	m_movieData->m_movieObj->getFloatingWindowSize(&width, &height);
	m_movieData->m_size = QSize(width, height);
	m_movieData->m_rotation = m_movieData->m_movieObj->getRotationAngle();
	m_movieData->m_showControls = mp->showControls;
	m_movieData->m_playMode = (MovieObject::PlayMode)mp->repeatMode;
}

MovieObject::~MovieObject()
{
	delete m_movieData;
}

QString MovieObject::url() const
{
	GooString * goo = m_movieData->m_movieObj->getFileName();
	return goo ? QString( goo->getCString() ) : QString();
}

QSize MovieObject::size() const
{
	return m_movieData->m_size;
}

int MovieObject::rotation() const
{
	return m_movieData->m_rotation;
}

bool MovieObject::showControls() const
{
	return m_movieData->m_showControls;
}

MovieObject::PlayMode MovieObject::playMode() const
{
	return m_movieData->m_playMode;
}

}
