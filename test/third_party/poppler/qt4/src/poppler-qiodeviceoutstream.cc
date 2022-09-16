/* poppler-qiodevicestream.cc: Qt4 interface to poppler
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
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

#include "poppler-qiodeviceoutstream-private.h"

#include <QtCore/QIODevice>

#include <stdio.h>

#define QIODeviceOutStreamBufSize 8192

namespace Poppler {

QIODeviceOutStream::QIODeviceOutStream(QIODevice* device)
  : m_device(device)
{
}

QIODeviceOutStream::~QIODeviceOutStream()
{
}

void QIODeviceOutStream::close()
{
}

int QIODeviceOutStream::getPos()
{
  return (int)m_device->pos();
}

void QIODeviceOutStream::put(char c)
{
  m_device->putChar(c);
}

void QIODeviceOutStream::printf(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  char buf[QIODeviceOutStreamBufSize];
  size_t bufsize = 0;
  bufsize = qvsnprintf(buf, QIODeviceOutStreamBufSize - 1, format, ap);
  va_end(ap);
  m_device->write(buf, bufsize);
}

}
