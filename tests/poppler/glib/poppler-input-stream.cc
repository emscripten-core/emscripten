/* poppler-input-stream.cc: glib interface to poppler
 *
 * Copyright (C) 2012 Carlos Garcia Campos <carlosgc@gnome.org>
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

#include "config.h"
#include "poppler-input-stream.h"

PopplerInputStream::PopplerInputStream(GInputStream *inputStreamA, GCancellable *cancellableA,
                                       Goffset startA, GBool limitedA, Goffset lengthA, Object *dictA)
  : BaseStream(dictA, lengthA)
{
  inputStream = (GInputStream *)g_object_ref(inputStreamA);
  cancellable = cancellableA ? (GCancellable *)g_object_ref(cancellableA) : NULL;
  start = startA;
  limited = limitedA;
  length = lengthA;
  bufPtr = bufEnd = buf;
  bufPos = start;
}

PopplerInputStream::~PopplerInputStream()
{
  close();
  g_object_unref(inputStream);
  if (cancellable)
    g_object_unref(cancellable);
}

BaseStream *PopplerInputStream::copy() {
  return new PopplerInputStream(inputStream, cancellable, start, limited, length, &dict);
}

Stream *PopplerInputStream::makeSubStream(Goffset startA, GBool limitedA,
                                          Goffset lengthA, Object *dictA)
{
  return new PopplerInputStream(inputStream, cancellable, startA, limitedA, lengthA, dictA);
}

void PopplerInputStream::reset()
{
  GSeekable *seekable = G_SEEKABLE(inputStream);

  savePos = (Guint)g_seekable_tell(seekable);
  g_seekable_seek(seekable, start, G_SEEK_SET, cancellable, NULL);
  saved = gTrue;
  bufPtr = bufEnd = buf;
  bufPos = start;
}

void PopplerInputStream::close()
{
  if (!saved)
    return;
  g_seekable_seek(G_SEEKABLE(inputStream), savePos, G_SEEK_SET, cancellable, NULL);
  saved = gFalse;
}

void PopplerInputStream::setPos(Goffset pos, int dir)
{
  Guint size;
  GSeekable *seekable = G_SEEKABLE(inputStream);

  if (dir >= 0) {
    g_seekable_seek(seekable, pos, G_SEEK_SET, cancellable, NULL);
  } else {
    g_seekable_seek(seekable, 0, G_SEEK_END, cancellable, NULL);
    size = (Guint)g_seekable_tell(seekable);

    if (pos > size)
      pos = size;

    g_seekable_seek(seekable, -(goffset)pos, G_SEEK_END, cancellable, NULL);
    bufPos = (Guint)g_seekable_tell(seekable);
  }
  bufPtr = bufEnd = buf;
}

void PopplerInputStream::moveStart(Goffset delta)
{
  start += delta;
  bufPtr = bufEnd = buf;
  bufPos = start;
}

GBool PopplerInputStream::fillBuf()
{
  int n;

  bufPos += bufEnd - buf;
  bufPtr = bufEnd = buf;
  if (limited && bufPos >= start + length) {
    return gFalse;
  }

  if (limited && bufPos + inputStreamBufSize > start + length) {
    n = start + length - bufPos;
  } else {
    n = inputStreamBufSize;
  }

  n = g_input_stream_read(inputStream, buf, n, cancellable, NULL);
  bufEnd = buf + n;
  if (bufPtr >= bufEnd) {
    return gFalse;
  }

  return gTrue;
}

int PopplerInputStream::getChars(int nChars, Guchar *buffer)
{
  int n, m;

  n = 0;
  while (n < nChars) {
    if (bufPtr >= bufEnd) {
      if (!fillBuf()) {
        break;
      }
    }
    m = (int)(bufEnd - bufPtr);
    if (m > nChars - n) {
      m = nChars - n;
    }
    memcpy(buffer + n, bufPtr, m);
    bufPtr += m;
    n += m;
  }
  return n;
}
