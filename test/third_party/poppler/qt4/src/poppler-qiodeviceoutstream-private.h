/* poppler-qiodevicestream-private.h: Qt4 interface to poppler
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

#ifndef POPPLER_QIODEVICESTREAM_PRIVATE_H
#define POPPLER_QIODEVICESTREAM_PRIVATE_H

#include "Object.h"
#include "Stream.h"

class QIODevice;

namespace Poppler {

class QIODeviceOutStream : public OutStream
{
  public:
    QIODeviceOutStream(QIODevice* device);
    virtual ~QIODeviceOutStream();

    virtual void close();
    virtual int getPos();
    virtual void put(char c);
    virtual void printf(const char *format, ...);

  private:
    QIODevice *m_device;
};

}

#endif
