/* Sound.h - an object that holds the sound structure
 * Copyright (C) 2006-2007, Pino Toscano <pino@kde.org>
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

#ifndef Sound_H
#define Sound_H

class GooString;
class Object;
class Stream;

//------------------------------------------------------------------------

enum SoundKind {
  soundEmbedded,		// embedded sound
  soundExternal			// external sound
};

enum SoundEncoding {
  soundRaw,			// raw encoding
  soundSigned,			// twos-complement values
  soundMuLaw,			// mu-law-encoded samples
  soundALaw			// A-law-encoded samples
};

class Sound
{
public:
  // Try to parse the Object s
  static Sound *parseSound(Object *s);

  // Destructor
  ~Sound();

  Object *getObject() { return streamObj; }
  Stream *getStream();

  SoundKind getSoundKind() { return kind; }
  GooString *getFileName() { return fileName; }
  double getSamplingRate() { return samplingRate; }
  int getChannels() { return channels; }
  int getBitsPerSample() { return bitsPerSample; }
  SoundEncoding getEncoding() { return encoding; }

  Sound *copy();

private:
  // Create a sound. The Object obj is ensured to be a Stream with a Dict
  Sound(Object *obj, bool readAttrs = true);

  Object *streamObj;
  SoundKind kind;
  GooString *fileName;
  double samplingRate;
  int channels;
  int bitsPerSample;
  SoundEncoding encoding;
};

#endif
