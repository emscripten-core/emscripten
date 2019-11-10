// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//'use strict';

var LibraryOpenAL = {
  // ************************************************************************
  // ** INTERNALS 
  // ************************************************************************

  $AL__deps: ['$Browser'],
  $AL: {
    // ------------------------------------------------------
    // -- Constants 
    // ------------------------------------------------------

    QUEUE_INTERVAL: 25,
    QUEUE_LOOKAHEAD: 100.0 / 1000.0,

    DEVICE_NAME: 'Emscripten OpenAL',
    CAPTURE_DEVICE_NAME: 'Emscripten OpenAL capture',

    ALC_EXTENSIONS: {
      // TODO: 'ALC_EXT_EFX': true,
      'ALC_SOFT_pause_device': true,
      'ALC_SOFT_HRTF': true
    },
    AL_EXTENSIONS: {
      'AL_EXT_float32': true,
      'AL_SOFT_loop_points': true,
      'AL_SOFT_source_length': true,
      'AL_EXT_source_distance_model': true,
      'AL_SOFT_source_spatialize': true
    },

    // ------------------------------------------------------
    // -- ALC Fields
    // ------------------------------------------------------

    _alcErr: 0,
    get alcErr() {
      return this._alcErr;
    },
    set alcErr(val) {
      // Errors should not be overwritten by later errors until they are cleared by a query.
      if (this._alcErr === 0 /* ALC_NO_ERROR */ || val === 0 /* ALC_NO_ERROR */) {
        this._alcErr = val;
      }
    },

    deviceRefCounts: {},
    alcStringCache: {},
    paused: false,

    // ------------------------------------------------------
    // -- AL Fields
    // ------------------------------------------------------

    stringCache: {},
    contexts: {},
    currentCtx: null,
    buffers: {
      // The zero buffer is legal to use, so create a placeholder for it
      '0': {
        id: 0,
        refCount: 0,
        audioBuf: null,
        frequency: 0,
        bytesPerSample: 2,
        channels: 1,
        length: 0
      }
    },
    paramArray: [], // Used to prevent allocating a new array for each param call

    _nextId: 1,
    newId: function() {
      return AL.freeIds.length > 0 ? AL.freeIds.pop() : AL._nextId++;
    },
    freeIds: [],

    // ------------------------------------------------------
    // -- Mixing Logic
    // ------------------------------------------------------

    scheduleContextAudio: function(ctx) {
      // If we are animating using the requestAnimationFrame method, then the main loop does not run when in the background.
      // To give a perfect glitch-free audio stop when switching from foreground to background, we need to avoid updating
      // audio altogether when in the background, so detect that case and kill audio buffer streaming if so.
      if (Browser.mainLoop.timingMode === 1 /* EM_TIMING_RAF */ && document['visibilityState'] != 'visible') {
        return;
      }

      for (var i in ctx.sources) {
        AL.scheduleSourceAudio(ctx.sources[i]);
      }
    },

    // This function is the core scheduler that queues web-audio buffers for output.
    // src.bufQueue represents the abstract OpenAL buffer queue, which is taversed to schedule
    // corresponding web-audio buffers. These buffers are stored in src.audioQueue, which
    // represents the queue of buffers scheduled for physical playback. These two queues are
    // distinct because of the differing semantics of OpenAL and web audio. Some changes
    // to OpenAL parameters, such as pitch, may require the web audio queue to be flushed and rescheduled.
    scheduleSourceAudio: function(src, lookahead) {
      // See comment on scheduleContextAudio above.
      if (Browser.mainLoop.timingMode === 1 /*EM_TIMING_RAF*/ && document['visibilityState'] != 'visible') {
        return;
      }
      if (src.state !== 0x1012 /* AL_PLAYING */) {
        return;
      }

      var currentTime = AL.updateSourceTime(src);

      var startTime = src.bufStartTime;
      var startOffset = src.bufOffset;
      var bufCursor = src.bufsProcessed;

      // Advance past any audio that is already scheduled
      for (var i = 0; i < src.audioQueue.length; i++) {
        var audioSrc = src.audioQueue[i];
        startTime = audioSrc._startTime + audioSrc._duration;
        startOffset = 0.0;
        bufCursor += audioSrc._skipCount + 1;
      }

      if (!lookahead) {
        lookahead = AL.QUEUE_LOOKAHEAD;
      }
      var lookaheadTime = currentTime + lookahead;
      var skipCount = 0;
      while (startTime < lookaheadTime) {
        if (bufCursor >= src.bufQueue.length) {
          if (src.looping) {
            bufCursor %= src.bufQueue.length;
          } else {
            break;
          }
        }

        var buf = src.bufQueue[bufCursor % src.bufQueue.length];
        // If the buffer contains no data, skip it
        if (buf.length === 0) {
          skipCount++;
          // If we've gone through the whole queue and everything is 0 length, just give up
          if (skipCount === src.bufQueue.length) {
            break;
          }
        } else {
          var audioSrc = src.context.audioCtx.createBufferSource();
          audioSrc.buffer = buf.audioBuf;
          audioSrc.playbackRate.value = src.playbackRate;
          if (buf.audioBuf._loopStart || buf.audioBuf._loopEnd) {
            audioSrc.loopStart = buf.audioBuf._loopStart;
            audioSrc.loopEnd = buf.audioBuf._loopEnd;
          }

          var duration = 0.0;
          // If the source is a looping static buffer, use native looping for gapless playback
          if (src.type === 0x1028 /* AL_STATIC */ && src.looping) {
            duration = Number.POSITIVE_INFINITY;
            audioSrc.loop = true;
            if (buf.audioBuf._loopStart) {
              audioSrc.loopStart = buf.audioBuf._loopStart;
            }
            if (buf.audioBuf._loopEnd) {
              audioSrc.loopEnd = buf.audioBuf._loopEnd;
            }
          } else {
            duration = (buf.audioBuf.duration - startOffset) / src.playbackRate;
          }

          audioSrc._startOffset = startOffset;
          audioSrc._duration = duration;
          audioSrc._skipCount = skipCount;
          skipCount = 0;

          audioSrc.connect(src.gain);

          if (typeof(audioSrc.start) !== 'undefined') {
            // Sample the current time as late as possible to mitigate drift
            startTime = Math.max(startTime, src.context.audioCtx.currentTime);
            audioSrc.start(startTime, startOffset);
          } else if (typeof(audioSrc.noteOn) !== 'undefined') {
            startTime = Math.max(startTime, src.context.audioCtx.currentTime);
            audioSrc.noteOn(startTime);
#if OPENAL_DEBUG
            if (offset > 0.0) {
              warnOnce('The current browser does not support AudioBufferSourceNode.start(when, offset); method, so cannot play back audio with an offset '+startOffset+' secs! Audio glitches will occur!');
            }
#endif
          }
#if OPENAL_DEBUG
          else {
            warnOnce('Unable to start AudioBufferSourceNode playback! Not supported by the browser?');
          }

          console.log('scheduleSourceAudio() queuing buffer ' + buf.id + ' for source ' + src.id + ' at ' + startTime + ' (offset by ' + startOffset + ')');
#endif
          audioSrc._startTime = startTime;
          src.audioQueue.push(audioSrc);

          startTime += duration;
        }

        startOffset = 0.0;
        bufCursor++;
      }
    },

    // Advance the state of a source forward to the current time
    updateSourceTime: function(src) {
      var currentTime = src.context.audioCtx.currentTime;
      if (src.state !== 0x1012 /* AL_PLAYING */) {
        return currentTime;
      }

      // if the start time is unset, determine it based on the current offset.
      // This will be the case when a source is resumed after being paused, and
      // allows us to pretend that the source actually started playing some time
      // in the past such that it would just now have reached the stored offset.
      if (!isFinite(src.bufStartTime)) {
        src.bufStartTime = currentTime - src.bufOffset / src.playbackRate;
        src.bufOffset = 0.0;
      }

      var nextStartTime = 0.0;
      while (src.audioQueue.length) {
        var audioSrc = src.audioQueue[0];
        src.bufsProcessed += audioSrc._skipCount;
        nextStartTime = audioSrc._startTime + audioSrc._duration; // n.b. audioSrc._duration already factors in playbackRate, so no divide by src.playbackRate on it.

        if (currentTime < nextStartTime) {
          break;
        }

        src.audioQueue.shift();
        src.bufStartTime = nextStartTime;
        src.bufOffset = 0.0;
        src.bufsProcessed++;
      }

      if (src.bufsProcessed >= src.bufQueue.length && !src.looping) {
        // The source has played its entire queue and is non-looping, so just mark it as stopped.
        AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
      } else if (src.type === 0x1028 /* AL_STATIC */ && src.looping) {
        // If the source is a looping static buffer, determine the buffer offset based on the loop points
        var buf = src.bufQueue[0];
        if (buf.length === 0) {
          src.bufOffset = 0.0;
        } else {
          var delta = (currentTime - src.bufStartTime) * src.playbackRate;
          var loopStart = buf.audioBuf._loopStart || 0.0;
          var loopEnd = buf.audioBuf._loopEnd || buf.audioBuf.duration;
          if (loopEnd <= loopStart) {
            loopEnd = buf.audioBuf.duration;
          }

          if (delta < loopEnd) {
            src.bufOffset = delta;
          } else {
            src.bufOffset = loopStart + (delta - loopStart) % (loopEnd - loopStart);
          }
        }
      } else if (src.audioQueue[0]) {
        // The source is still actively playing, so we just need to calculate where we are in the current buffer
        // so it can be remembered if the source gets paused.
        src.bufOffset = (currentTime - src.audioQueue[0]._startTime) * src.playbackRate;
      } else {
        // The source hasn't finished yet, but there is no scheduled audio left for it. This can be because
        // the source has just been started/resumed, or due to an underrun caused by a long blocking operation.
        // We need to determine what state we would be in by this point in time so that when we next schedule
        // audio playback, it will be just as if no underrun occurred.

        if (src.type !== 0x1028 /* AL_STATIC */ && src.looping) {
          // if the source is a looping buffer queue, let's first calculate the queue duration, so we can
          // quickly fast forward past any full loops of the queue and only worry about the remainder.
          var srcDuration = AL.sourceDuration(src) / src.playbackRate;
          if (srcDuration > 0.0) {
            src.bufStartTime += Math.floor((currentTime - src.bufStartTime) / srcDuration) * srcDuration;
          }
        }

        // Since we've already skipped any full-queue loops if there were any, we just need to find
        // out where in the queue the remaining time puts us, which won't require stepping through the
        // entire queue more than once.
        for (var i = 0; i < src.bufQueue.length; i++) {
          if (src.bufsProcessed >= src.bufQueue.length) {
            if (src.looping) {
              src.bufsProcessed %= src.bufQueue.length;
            } else {
              AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
              break;
            }
          }

          var buf = src.bufQueue[src.bufsProcessed];
          if (buf.length > 0) {
            nextStartTime = src.bufStartTime + buf.audioBuf.duration / src.playbackRate;

            if (currentTime < nextStartTime) {
              src.bufOffset = (currentTime - src.bufStartTime) * src.playbackRate;
              break;
            }

            src.bufStartTime = nextStartTime;
          }

          src.bufOffset = 0.0;
          src.bufsProcessed++;
        }
      }

      return currentTime;
    },

    cancelPendingSourceAudio: function(src) {
      AL.updateSourceTime(src);

      for (var i = 1; i < src.audioQueue.length; i++) {
        var audioSrc = src.audioQueue[i];
        audioSrc.stop();
      }

      if (src.audioQueue.length > 1) {
        src.audioQueue.length = 1;
      }
    },

    stopSourceAudio: function(src) {
      for (var i = 0; i < src.audioQueue.length; i++) {
        src.audioQueue[i].stop();
      }
      src.audioQueue.length = 0;
    },

    setSourceState: function(src, state) {
      if (state === 0x1012 /* AL_PLAYING */) {
        if (src.state === 0x1012 /* AL_PLAYING */ || src.state == 0x1014 /* AL_STOPPED */) {
          src.bufsProcessed = 0;
          src.bufOffset = 0.0;
#if OPENAL_DEBUG
          console.log('setSourceState() resetting and playing source ' + src.id);
#endif
        } else {
#if OPENAL_DEBUG
          console.log('setSourceState() playing source ' + src.id + ' at ' + src.bufOffset);
#endif
        }

        AL.stopSourceAudio(src);

        src.state = 0x1012 /* AL_PLAYING */;
        src.bufStartTime = Number.NEGATIVE_INFINITY;
        AL.scheduleSourceAudio(src);
      } else if (state === 0x1013 /* AL_PAUSED */) {
        if (src.state === 0x1012 /* AL_PLAYING */) {
          // Store off the current offset to restore with on resume.
          AL.updateSourceTime(src);
          AL.stopSourceAudio(src);

          src.state = 0x1013 /* AL_PAUSED */;
#if OPENAL_DEBUG
          console.log('setSourceState() pausing source ' + src.id + ' at ' + src.bufOffset);
#endif
        }
      } else if (state === 0x1014 /* AL_STOPPED */) {
        if (src.state !== 0x1011 /* AL_INITIAL */) {
          src.state = 0x1014 /* AL_STOPPED */;
          src.bufsProcessed = src.bufQueue.length;
          src.bufStartTime = Number.NEGATIVE_INFINITY;
          src.bufOffset = 0.0;
          AL.stopSourceAudio(src);
#if OPENAL_DEBUG
          console.log('setSourceState() stopping source ' + src.id);
#endif
        }
      } else if (state === 0x1011 /* AL_INITIAL */) {
        if (src.state !== 0x1011 /* AL_INITIAL */) {
          src.state = 0x1011 /* AL_INITIAL */;
          src.bufsProcessed = 0;
          src.bufStartTime = Number.NEGATIVE_INFINITY;
          src.bufOffset = 0.0;
          AL.stopSourceAudio(src);
#if OPENAL_DEBUG
          console.log('setSourceState() initializing source ' + src.id);
#endif
        }
      }
    },

    initSourcePanner: function(src) {
      if (src.type === 0x1030 /* AL_UNDETERMINED */) {
        return;
      }

      // Find the first non-zero buffer in the queue to determine the proper format
      var templateBuf = AL.buffers[0];
      for (var i = 0; i < src.bufQueue.length; i++) {
        if (src.bufQueue[i].id !== 0) {
          templateBuf = src.bufQueue[i];
          break;
        }
      }
      // Create a panner if AL_SOURCE_SPATIALIZE_SOFT is set to true, or alternatively if it's set to auto and the source is mono
      if (src.spatialize === 1 /* AL_TRUE */ || (src.spatialize === 2 /* AL_AUTO_SOFT */ && templateBuf.channels === 1)) {
        if (src.panner) {
          return;
        }
        src.panner = src.context.audioCtx.createPanner();

        AL.updateSourceGlobal(src);
        AL.updateSourceSpace(src);

        src.panner.connect(src.context.gain);
        src.gain.disconnect();
        src.gain.connect(src.panner);
      } else {
        if (!src.panner) {
          return;
        }

        src.panner.disconnect();
        src.gain.disconnect();
        src.gain.connect(src.context.gain);
        src.panner = null;
      }
    },

    updateContextGlobal: function(ctx) {
      for (var i in ctx.sources) {
        AL.updateSourceGlobal(ctx.sources[i]);
      }
    },

    updateSourceGlobal: function(src) {
      var panner = src.panner;
      if (!panner) {
        return;
      }

      panner.refDistance = src.refDistance;
      panner.maxDistance = src.maxDistance;
      panner.rolloffFactor = src.rolloffFactor;

      panner.panningModel = src.context.hrtf ? 'HRTF' : 'equalpower';

      // Use the source's distance model if AL_SOURCE_DISTANCE_MODEL is enabled
      var distanceModel = src.context.sourceDistanceModel ? src.distanceModel : src.context.distanceModel;
      switch (distanceModel) {
      case 0 /* AL_NONE */:
        panner.distanceModel = 'inverse';
        panner.refDistance = 3.40282e38 /* FLT_MAX */;
        break;
      case 0xd001 /* AL_INVERSE_DISTANCE */:
      case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
        panner.distanceModel = 'inverse';
        break;
      case 0xd003 /* AL_LINEAR_DISTANCE */:
      case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
        panner.distanceModel = 'linear';
        break;
      case 0xd005 /* AL_EXPONENT_DISTANCE */:
      case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
        panner.distanceModel = 'exponential';
        break;
      }
    },

    updateListenerSpace: function(ctx) {
      var listener = ctx.audioCtx.listener;
      if (listener.positionX) {
        listener.positionX.value = ctx.listener.position[0];
        listener.positionY.value = ctx.listener.position[1];
        listener.positionZ.value = ctx.listener.position[2];
      } else {
#if OPENAL_DEBUG
        warnOnce('Listener position attributes are not present, falling back to setPosition()');
#endif
        listener.setPosition(ctx.listener.position[0], ctx.listener.position[1], ctx.listener.position[2]);
      }
      if (listener.forwardX) {
        listener.forwardX.value = ctx.listener.direction[0];
        listener.forwardY.value = ctx.listener.direction[1];
        listener.forwardZ.value = ctx.listener.direction[2];
        listener.upX.value = ctx.listener.up[0];
        listener.upY.value = ctx.listener.up[1];
        listener.upZ.value = ctx.listener.up[2];
      } else {
#if OPENAL_DEBUG
        warnOnce('Listener orientation attributes are not present, falling back to setOrientation()');
#endif
        listener.setOrientation(
          ctx.listener.direction[0], ctx.listener.direction[1], ctx.listener.direction[2],
          ctx.listener.up[0], ctx.listener.up[1], ctx.listener.up[2]);
      }

      // Update sources that are relative to the listener
      for (var i in ctx.sources) {
        AL.updateSourceSpace(ctx.sources[i]);
      }
    },

    updateSourceSpace: function(src) {
      if (!src.panner) {
        return;
      }
      var panner = src.panner;

      var posX = src.position[0];
      var posY = src.position[1];
      var posZ = src.position[2];
      var dirX = src.direction[0];
      var dirY = src.direction[1];
      var dirZ = src.direction[2];

      var listener = src.context.listener;
      var lPosX = listener.position[0];
      var lPosY = listener.position[1];
      var lPosZ = listener.position[2];

      // WebAudio does spatialization in world-space coordinates, meaning both the buffer sources and
      // the listener position are in the same absolute coordinate system relative to a fixed origin.
      // By default, OpenAL works this way as well, but it also provides a "listener relative" mode, where
      // a buffer source's coordinate are interpreted not in absolute world space, but as being relative
      // to the listener object itself, so as the listener moves the source appears to move with it
      // with no update required. Since web audio does not support this mode, we must transform the source
      // coordinates from listener-relative space to absolute world space.
      //
      // We do this via affine transformation matrices applied to the source position and source direction.
      // A change-of-basis converts from listener-space displacements to world-space displacements,
      // which must be done for both the source position and direction. Lastly, the source position must be
      // added to the listener position to get the final source position, since the source position represents
      // a displacement from the listener.
      if (src.relative) {
        // Negate the listener direction since forward is -Z.
        var lBackX = -listener.direction[0];
        var lBackY = -listener.direction[1];
        var lBackZ = -listener.direction[2];
        var lUpX = listener.up[0];
        var lUpY = listener.up[1];
        var lUpZ = listener.up[2];

        function inverseMagnitude(x, y, z) {
          var length = Math.sqrt(x * x + y * y + z * z);

          if (length < Number.EPSILON) {
            return 0.0;
          }

          return 1.0 / length;
        }

        // Normalize the Back vector
        var invMag = inverseMagnitude(lBackX, lBackY, lBackZ);
        lBackX *= invMag;
        lBackY *= invMag;
        lBackZ *= invMag;

        // ...and the Up vector
        var invMag = inverseMagnitude(lUpX, lUpY, lUpZ);
        lUpX *= invMag;
        lUpY *= invMag;
        lUpZ *= invMag;

        // Calculate the Right vector as the cross product of the Up and Back vectors
        var lRightX = (lUpY * lBackZ - lUpZ * lBackY);
        var lRightY = (lUpZ * lBackX - lUpX * lBackZ);
        var lRightZ = (lUpX * lBackY - lUpY * lBackX);

        // Back and Up might not be exactly perpendicular, so the cross product also needs normalization
        var invMag = inverseMagnitude(lRightX, lRightY, lRightZ);
        lRightX *= invMag;
        lRightY *= invMag;
        lRightZ *= invMag;

        // Recompute Up from the now orthonormal Right and Back vectors so we have a fully orthonormal basis
        var lUpX = (lBackY * lRightZ - lBackZ * lRightY);
        var lUpY = (lBackZ * lRightX - lBackX * lRightZ);
        var lUpZ = (lBackX * lRightY - lBackY * lRightX);

        var oldX = dirX;
        var oldY = dirY;
        var oldZ = dirZ;

        // Use our 3 vectors to apply a change-of-basis matrix to the source direction
        dirX = oldX * lRightX + oldY * lUpX + oldZ * lBackX;
        dirY = oldX * lRightY + oldY * lUpY + oldZ * lBackY;
        dirZ = oldX * lRightZ + oldY * lUpZ + oldZ * lBackZ;

        var oldX = posX;
        var oldY = posY;
        var oldZ = posZ;

        // ...and to the source position
        posX = oldX * lRightX + oldY * lUpX + oldZ * lBackX;
        posY = oldX * lRightY + oldY * lUpY + oldZ * lBackY;
        posZ = oldX * lRightZ + oldY * lUpZ + oldZ * lBackZ;

        // The change-of-basis corrects the orientation, but the origin is still the listener.
        // Translate the source position by the listener position to finish.
        posX += lPosX;
        posY += lPosY;
        posZ += lPosZ;
      }

      if (panner.positionX) {
        panner.positionX.value = posX;
        panner.positionY.value = posY;
        panner.positionZ.value = posZ;
      } else {
#if OPENAL_DEBUG
        warnOnce('Panner position attributes are not present, falling back to setPosition()');
#endif
        panner.setPosition(posX, posY, posZ);
      }
      if (panner.orientationX) {
        panner.orientationX.value = dirX;
        panner.orientationY.value = dirY;
        panner.orientationZ.value = dirZ;
      } else {
#if OPENAL_DEBUG
        warnOnce('Panner orientation attributes are not present, falling back to setOrientation()');
#endif
        panner.setOrientation(dirX, dirY, dirZ);
      }

      var oldShift = src.dopplerShift;
      var velX = src.velocity[0];
      var velY = src.velocity[1];
      var velZ = src.velocity[2];
      var lVelX = listener.velocity[0];
      var lVelY = listener.velocity[1];
      var lVelZ = listener.velocity[2];
      if (posX === lPosX && posY === lPosY && posZ === lPosZ
        || velX === lVelX && velY === lVelY && velZ === lVelZ)
      {
        src.dopplerShift = 1.0;
      } else {
        // Doppler algorithm from 1.1 spec
        var speedOfSound = src.context.speedOfSound;
        var dopplerFactor = src.context.dopplerFactor;

        var slX = lPosX - posX;
        var slY = lPosY - posY;
        var slZ = lPosZ - posZ;

        var magSl = Math.sqrt(slX * slX + slY * slY + slZ * slZ);
        var vls = (slX * lVelX + slY * lVelY + slZ * lVelZ) / magSl;
        var vss = (slX * velX + slY * velY + slZ * velZ) / magSl;

        vls = Math.min(vls, speedOfSound / dopplerFactor);
        vss = Math.min(vss, speedOfSound / dopplerFactor);

        src.dopplerShift = (speedOfSound - dopplerFactor * vls) / (speedOfSound - dopplerFactor * vss);
      }
      if (src.dopplerShift !== oldShift) {
        AL.updateSourceRate(src);
      }
    },

    updateSourceRate: function(src) {
      if (src.state === 0x1012 /* AL_PLAYING */) {
        // clear scheduled buffers
        AL.cancelPendingSourceAudio(src);

        var audioSrc = src.audioQueue[0];
        if (!audioSrc) {
          return; // It is possible that AL.scheduleContextAudio() has not yet fed the next buffer, if so, skip.
        }

        var duration;
        if (src.type === 0x1028 /* AL_STATIC */ && src.looping) {
          duration = Number.POSITIVE_INFINITY;
        } else {
          // audioSrc._duration is expressed after factoring in playbackRate, so when changing playback rate, need
          // to recompute/rescale the rate to the new playback speed.
          duration = (audioSrc.buffer.duration - audioSrc._startOffset) / src.playbackRate;
        }

        audioSrc._duration = duration;
        audioSrc.playbackRate.value = src.playbackRate;

        // reschedule buffers with the new playbackRate
        AL.scheduleSourceAudio(src);
      }
    },

    sourceDuration: function(src) {
      var length = 0.0;
      for (var i = 0; i < src.bufQueue.length; i++) {
        var audioBuf = src.bufQueue[i].audioBuf;
        length += audioBuf ? audioBuf.duration : 0.0;
      }
      return length;
    },

    sourceTell: function(src) {
      AL.updateSourceTime(src);

      var offset = 0.0;
      for (var i = 0; i < src.bufsProcessed; i++) {
        offset += src.bufQueue[i].audioBuf.duration;
      }
      offset += src.bufOffset;

      return offset;
    },

    sourceSeek: function(src, offset) {
      var playing = src.state == 0x1012 /* AL_PLAYING */;
      if (playing) {
        AL.setSourceState(src, 0x1011 /* AL_INITIAL */);
      }

      src.bufsProcessed = 0;
      while (offset > src.bufQueue[src.bufsProcessed].audioBuf.duration) {
        offset -= src.bufQueue[src.bufsProcessed].audiobuf.duration;
        src.bufsProcessed++;
      }

      src.bufOffset = offset;
      if (playing) {
        AL.setSourceState(src, 0x1012 /* AL_PLAYING */);
      }
    },

    // ------------------------------------------------------
    // -- Accessor Helpers
    // ------------------------------------------------------

    getGlobalParam: function(funcname, param) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return null;
      }

      switch (param) {
      case 0xC000 /* AL_DOPPLER_FACTOR */:
        return AL.currentCtx.dopplerFactor;
      case 0xC003 /* AL_SPEED_OF_SOUND */:
        return AL.currentCtx.speedOfSound;
      case 0xD000 /* AL_DISTANCE_MODEL */:
        return AL.currentCtx.distanceModel;
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return null;
      }
    },

    setGlobalParam: function(funcname, param, value) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return;
      }

      switch (param) {
      case 0xC000 /* AL_DOPPLER_FACTOR */:
        if (!Number.isFinite(value) || value < 0.0) { // Strictly negative values are disallowed
#if OPENAL_DEBUG
          console.error(funcname + '() value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        AL.currentCtx.dopplerFactor = value;
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case 0xC003 /* AL_SPEED_OF_SOUND */:
        if (!Number.isFinite(value) || value <= 0.0) { // Negative or zero values are disallowed
#if OPENAL_DEBUG
          console.error(funcname + '() value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        AL.currentCtx.speedOfSound = value;
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case 0xD000 /* AL_DISTANCE_MODEL */:
        switch (value) {
        case 0 /* AL_NONE */:
        case 0xd001 /* AL_INVERSE_DISTANCE */:
        case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
        case 0xd003 /* AL_LINEAR_DISTANCE */:
        case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
        case 0xd005 /* AL_EXPONENT_DISTANCE */:
        case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
          AL.currentCtx.distanceModel = value;
          AL.updateContextGlobal(AL.currentCtx);
          break;
        default:
#if OPENAL_DEBUG
          console.error(funcname + '() value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        break;
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return;
      }
    },

    getListenerParam: function(funcname, param) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return null;
      }

      switch (param) {
      case 0x1004 /* AL_POSITION */:
        return AL.currentCtx.listener.position;
      case 0x1006 /* AL_VELOCITY */:
        return AL.currentCtx.listener.velocity;
      case 0x100F /* AL_ORIENTATION */:
        return AL.currentCtx.listener.direction.concat(AL.currentCtx.listener.up);
      case 0x100A /* AL_GAIN */:
        return AL.currentCtx.gain.gain.value;
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return null;
      }
    },

    setListenerParam: function(funcname, param, value) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return;
      }
      if (value === null) {
#if OPENAL_DEBUG
        console.error(funcname + '(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return;
      }

      var listener = AL.currentCtx.listener;
      switch (param) {
      case 0x1004 /* AL_POSITION */:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_POSITION value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        listener.position[0] = value[0];
        listener.position[1] = value[1];
        listener.position[2] = value[2];
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case 0x1006 /* AL_VELOCITY */:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_VELOCITY value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        listener.velocity[0] = value[0];
        listener.velocity[1] = value[1];
        listener.velocity[2] = value[2];
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case 0x100A /* AL_GAIN */:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_GAIN value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        AL.currentCtx.gain.gain.value = value;
        break;
      case 0x100F /* AL_ORIENTATION */:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])
          || !Number.isFinite(value[3]) || !Number.isFinite(value[4]) || !Number.isFinite(value[5])
        ) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_ORIENTATION value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        listener.direction[0] = value[0];
        listener.direction[1] = value[1];
        listener.direction[2] = value[2];
        listener.up[0] = value[3];
        listener.up[1] = value[4];
        listener.up[2] = value[5];
        AL.updateListenerSpace(AL.currentCtx);
        break;
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return;
      }
    },

    getBufferParam: function(funcname, bufferId, param) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return;
      }
      var buf = AL.buffers[bufferId];
      if (!buf || bufferId === 0) {
#if OPENAL_DEBUG
        console.error(funcname + '() called with an invalid buffer');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }

      switch (param) {
      case 0x2001 /* AL_FREQUENCY */:
        return buf.frequency;
      case 0x2002 /* AL_BITS */:
        return buf.bytesPerSample * 8;
      case 0x2003 /* AL_CHANNELS */:
        return buf.channels;
      case 0x2004 /* AL_SIZE */:
        return buf.length * buf.bytesPerSample * buf.channels;
      case 0x2015 /* AL_LOOP_POINTS_SOFT */:
        if (buf.length === 0) {
          return [0, 0];
        } else {
          return [
            (buf.audioBuf._loopStart || 0.0) * buf.frequency,
            (buf.audioBuf._loopEnd || buf.length) * buf.frequency
          ];
        }
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return null;
      }
    },

    setBufferParam: function(funcname, bufferId, param, value) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return;
      }
      var buf = AL.buffers[bufferId];
      if (!buf || bufferId === 0) {
#if OPENAL_DEBUG
        console.error(funcname + '() called with an invalid buffer');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
      if (value === null) {
#if OPENAL_DEBUG
        console.error(funcname + '(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return;
      }

      switch (param) {
      case 0x2004 /* AL_SIZE */:
        if (value !== 0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_SIZE value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        // Per the spec, setting AL_SIZE to 0 is a legal NOP.
        break;
      case 0x2015 /* AL_LOOP_POINTS_SOFT */:
        if (value[0] < 0 || value[0] > buf.length || value[1] < 0 || value[1] > buf.Length || value[0] >= value[1]) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_LOOP_POINTS_SOFT value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        if (buf.refCount > 0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_LOOP_POINTS_SOFT set on bound buffer');
#endif
          AL.currentCtx.err = 0xA004 /* AL_INVALID_OPERATION */;
          return;
        }

        if (buf.audioBuf) {
          buf.audioBuf._loopStart = value[0] / buf.frequency;
          buf.audioBuf._loopEnd = value[1] / buf.frequency;
        }
        break;
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return;
      }
    },

    getSourceParam: function(funcname, sourceId, param) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return null;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
#if OPENAL_DEBUG
        console.error(funcname + '() called with an invalid source');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return null;
      }

      switch (param) {
      case 0x202 /* AL_SOURCE_RELATIVE */:
        return src.relative;
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
        return src.coneInnerAngle;
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
        return src.coneOuterAngle;
      case 0x1003 /* AL_PITCH */:
        return src.pitch;
      case 0x1004 /* AL_POSITION */:
        return src.position;
      case 0x1005 /* AL_DIRECTION */:
        return src.direction;
      case 0x1006 /* AL_VELOCITY */:
        return src.velocity;
      case 0x1007 /* AL_LOOPING */:
        return src.looping;
      case 0x1009 /* AL_BUFFER */:
        if (src.type === 0x1028 /* AL_STATIC */) {
          return src.bufQueue[0].id;
        } else {
          return 0;
        }
      case 0x100A /* AL_GAIN */:
        return src.gain.gain.value;
       case 0x100D /* AL_MIN_GAIN */:
        return src.minGain;
      case 0x100E /* AL_MAX_GAIN */:
        return src.maxGain;
      case 0x1010 /* AL_SOURCE_STATE */:
        return src.state;
      case 0x1015 /* AL_BUFFERS_QUEUED */:
        if (src.bufQueue.length === 1 && src.bufQueue[0].id === 0) {
          return 0;
        } else {
          return src.bufQueue.length;
        }
      case 0x1016 /* AL_BUFFERS_PROCESSED */:
        if ((src.bufQueue.length === 1 && src.bufQueue[0].id === 0) || src.looping) {
          return 0;
        } else {
          return src.bufsProcessed;
        }
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
        return src.refDistance;
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
        return src.rolloffFactor;
      case 0x1022 /* AL_CONE_OUTER_GAIN */:
        return src.coneOuterGain;
      case 0x1023 /* AL_MAX_DISTANCE */:
        return src.maxDistance;
      case 0x1024 /* AL_SEC_OFFSET */:
        return AL.sourceTell(src);
      case 0x1025 /* AL_SAMPLE_OFFSET */:
        var offset = AL.sourceTell(src);
        if (offset > 0.0) {
          offset *= src.bufQueue[0].frequency;
        }
        return offset;
      case 0x1026 /* AL_BYTE_OFFSET */:
        var offset = AL.sourceTell(src);
        if (offset > 0.0) {
          offset *= src.bufQueue[0].frequency * src.bufQueue[0].bytesPerSample;
        }
        return offset;
      case 0x1027 /* AL_SOURCE_TYPE */:
        return src.type;
      case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
        return src.spatialize;
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */: 
        var length = 0;
        var bytesPerFrame = 0;
        for (var i = 0; i < src.bufQueue.length; i++) {
          length += src.bufQueue[i].length;
          if (src.bufQueue[i].id !== 0) {
            bytesPerFrame = src.bufQueue[i].bytesPerSample * src.bufQueue[i].channels;
          }
        }
        return length * bytesPerFrame;
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
        var length = 0;
        for (var i = 0; i < src.bufQueue.length; i++) {
          length += src.bufQueue[i].length;
        }
        return length;
      case 0x200B /* AL_SEC_LENGTH_SOFT */:
        return AL.sourceDuration(src);
      case 0xD000 /* AL_DISTANCE_MODEL */:
        return src.distanceModel;
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return null;
      }
    },

    setSourceParam: function(funcname, sourceId, param, value) {
      if (!AL.currentCtx) {
#if OPENAL_DEBUG
        console.error(funcname + '() called without a valid context');
#endif
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
#if OPENAL_DEBUG
        console.error('alSourcef() called with an invalid source');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
      if (value === null) {
#if OPENAL_DEBUG
        console.error(funcname + '(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return;
      }

      switch (param) {
      case 0x202 /* AL_SOURCE_RELATIVE */:
        if (value === 1 /* AL_TRUE */) {
          src.relative = true;
          AL.updateSourceSpace(src);
        } else if (value === 0 /* AL_FALSE */) {
          src.relative = false;
          AL.updateSourceSpace(src);
        } else {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_SOURCE_RELATIVE value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        break;
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
        if (!Number.isFinite(value)) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_CONE_INNER_ANGLE value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        src.coneInnerAngle = value;
        if (src.panner) {
          src.panner.coneInnerAngle = value % 360.0;
        }
        break;
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
        if (!Number.isFinite(value)) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_CONE_OUTER_ANGLE value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        src.coneOuterAngle = value;
        if (src.panner) {
          src.panner.coneOuterAngle = value % 360.0;
        }
        break;
      case 0x1003 /* AL_PITCH */:
        if (!Number.isFinite(value) || value <= 0.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_PITCH value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        if (src.pitch === value) {
          break;
        }

        src.pitch = value;
        AL.updateSourceRate(src);
        break;
      case 0x1004 /* AL_POSITION */:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_POSITION value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        src.position[0] = value[0];
        src.position[1] = value[1];
        src.position[2] = value[2];
        AL.updateSourceSpace(src);
        break;
      case 0x1005 /* AL_DIRECTION */:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_DIRECTION value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        src.direction[0] = value[0];
        src.direction[1] = value[1];
        src.direction[2] = value[2];
        AL.updateSourceSpace(src);
        break;
      case 0x1006 /* AL_VELOCITY */:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_VELOCITY value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        src.velocity[0] = value[0];
        src.velocity[1] = value[1];
        src.velocity[2] = value[2];
        AL.updateSourceSpace(src);
        break;
      case 0x1007 /* AL_LOOPING */:
        if (value === 1 /* AL_TRUE */) {
          src.looping = true;
          AL.updateSourceTime(src);
          if (src.type === 0x1028 /* AL_STATIC */ && src.audioQueue.length > 0) {
            var audioSrc  = src.audioQueue[0];
            audioSrc.loop = true;
            audioSrc._duration = Number.POSITIVE_INFINITY;
          }
        } else if (value === 0 /* AL_FALSE */) {
          src.looping = false;
          var currentTime = AL.updateSourceTime(src);
          if (src.type === 0x1028 /* AL_STATIC */ && src.audioQueue.length > 0) {
            var audioSrc  = src.audioQueue[0];
            audioSrc.loop = false;
            audioSrc._duration = src.bufQueue[0].audioBuf.duration / src.playbackRate;
            audioSrc._startTime = currentTime - src.bufOffset / src.playbackRate;
          }
        } else {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_LOOPING value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        break;
      case 0x1009 /* AL_BUFFER */:
        if (src.state === 0x1012 /* AL_PLAYING */ || src.state === 0x1013 /* AL_PAUSED */) {
#if OPENAL_DEBUG
          console.error(funcname + '(AL_BUFFER) called while source is playing or paused');
#endif
          AL.currentCtx.err = 0xA004 /* AL_INVALID_OPERATION */;
          return;
        }

        if (value === 0) {
          for (var i in src.bufQueue) {
            src.bufQueue[i].refCount--;
          }
          src.bufQueue.length = 1;
          src.bufQueue[0] = AL.buffers[0];

          src.bufsProcessed = 0;
          src.type = 0x1030 /* AL_UNDETERMINED */;
        } else {
          var buf = AL.buffers[value];
          if (!buf) {
#if OPENAL_DEBUG
            console.error('alSourcei(AL_BUFFER) called with an invalid buffer');
#endif
            AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
            return;
          }

          for (var i in src.bufQueue) {
            src.bufQueue[i].refCount--;
          }
          src.bufQueue.length = 0;

          buf.refCount++;
          src.bufQueue = [buf];
          src.bufsProcessed = 0;
          src.type = 0x1028 /* AL_STATIC */;
        }

        AL.initSourcePanner(src);
        AL.scheduleSourceAudio(src);
        break;
      case 0x100A /* AL_GAIN */:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_GAIN value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        src.gain.gain.value = value;
        break;
      case 0x100D /* AL_MIN_GAIN */:
        if (!Number.isFinite(value) || value < 0.0 || value > Math.min(src.maxGain, 1.0)) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_MIN_GAIN value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
#if OPENAL_DEBUG
        warnOnce('AL_MIN_GAIN is not currently supported');
#endif
        src.minGain = value;
        break;
      case 0x100E /* AL_MAX_GAIN */:
        if (!Number.isFinite(value) || value < Math.max(0.0, src.minGain) || value > 1.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_MAX_GAIN value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
#if OPENAL_DEBUG
        warnOnce('AL_MAX_GAIN is not currently supported');
#endif
        src.maxGain = value;
        break;
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_REFERENCE_DISTANCE value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        src.refDistance = value;
        if (src.panner) {
          src.panner.refDistance = value;
        }
        break;
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_ROLLOFF_FACTOR value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        src.rolloffFactor = value;
        if (src.panner) {
          src.panner.rolloffFactor = value;
        }
        break;
      case 0x1022 /* AL_CONE_OUTER_GAIN */:
        if (!Number.isFinite(value) || value < 0.0 || value > 1.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_CORE_OUTER_GAIN value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        src.coneOuterGain = value;
        if (src.panner) {
          src.panner.coneOuterGain = value;
        }
        break;
      case 0x1023 /* AL_MAX_DISTANCE */:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_MAX_DISTANCE value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        src.maxDistance = value;
        if (src.panner) {
          src.panner.maxDistance = value;
        }
        break;
      case 0x1024 /* AL_SEC_OFFSET */:
        if (value < 0.0 || value > AL.sourceDuration(src)) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_SEC_OFFSET value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        AL.sourceSeek(src, value);
        break;
      case 0x1025 /* AL_SAMPLE_OFFSET */:
        var srcLen = AL.sourceDuration(src);
        if (srcLen > 0.0) {
          var frequency;
          for (var bufId in src.bufQueue) {
            if (bufId !== 0) {
              frequency = src.bufQueue[bufId].frequency;
              break;
            }
          }
          value /= frequency;
        }
        if (value < 0.0 || value > srcLen) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_SAMPLE_OFFSET value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        AL.sourceSeek(src, value);
        break;
      case 0x1026 /* AL_BYTE_OFFSET */:
        var srcLen = AL.sourceDuration(src);
        if (srcLen > 0.0) {
          var bytesPerSec;
          for (var bufId in src.bufQueue) {
            if (bufId !== 0) {
              var buf = src.bufQueue[bufId];
              bytesPerSec = buf.frequency * buf.bytesPerSample * buf.channels;
              break;
            }
          }
          value /= bytesPerSec;
        }
        if (value < 0.0 || value > srcLen) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_BYTE_OFFSET value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        AL.sourceSeek(src, value);
        break;
      case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
        if (value !== 0 /* AL_FALSE */ && value !== 1 /* AL_TRUE */ && value !== 2 /* AL_AUTO_SOFT */) {
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_SOURCE_SPATIALIZE_SOFT value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }

        src.spatialize = value;
        AL.initSourcePanner(src);
        break;
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */: 
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
      case 0x200B /* AL_SEC_LENGTH_SOFT */:
#if OPENAL_DEBUG
        console.error(funcname + '() param AL_*_LENGTH_SOFT is read only');
#endif
        AL.currentCtx.err = 0xA004 /* AL_INVALID_OPERATION */;
        break;
      case 0xD000 /* AL_DISTANCE_MODEL */:
        switch (value) {
        case 0 /* AL_NONE */:
        case 0xd001 /* AL_INVERSE_DISTANCE */:
        case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
        case 0xd003 /* AL_LINEAR_DISTANCE */:
        case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
        case 0xd005 /* AL_EXPONENT_DISTANCE */:
        case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
          src.distanceModel = value;
          if (AL.currentCtx.sourceDistanceModel) {
            AL.updateContextGlobal(AL.currentCtx);
          }
          break;
        default:
#if OPENAL_DEBUG
          console.error(funcname + '() param AL_DISTANCE_MODEL value ' + value + ' is out of range');
#endif
          AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
          return;
        }
        break;
      default:
#if OPENAL_DEBUG
        console.error(funcname + '() param 0x' + param.toString(16) + ' is unknown or not implemented');
#endif
        AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
        return;
      }
    },

    // -------------------------------------------------------
    // -- Capture
    // -------------------------------------------------------

    // A map of 'capture device contexts'.
    captures: {},

    sharedCaptureAudioCtx: null,

    // Helper which:
    // - Asserts that deviceId is both non-NULL AND a known device ID;
    // - Returns a reference to it, or null if not found.
    // - Sets alcErr accordingly.
    // Treat NULL and <invalid> separately because careless
    // people might assume that most alcCapture functions
    // accept NULL as a 'use the default' device.
    requireValidCaptureDevice: function(deviceId, funcname) {
      if (deviceId === 0) {
#if OPENAL_DEBUG
        console.error(funcname+'() on a NULL device is an error');
#endif
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return null;
      }
      var c = AL.captures[deviceId];
      if (!c) {
#if OPENAL_DEBUG
        console.error(funcname+'() on an invalid device');
#endif
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return null;
      }
      var err = c.mediaStreamError;
      if (err) {
#if OPENAL_DEBUG
        switch(err.name) {
        case 'PermissionDeniedError':
          console.error(funcname+'() but the user denied access to the device');
          break;
        case 'NotFoundError':
          console.error(funcname+'() but no capture device was found');
          break;
        default:
          console.error(funcname+'() but a MediaStreamError was encountered: ' + err);
          break;
        }
#endif
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return null;
      }
      return c;
    }

  },

  // ***************************************************************************
  // ** ALC API 
  // ***************************************************************************

  // -------------------------------------------------------
  // -- ALC Capture
  // -------------------------------------------------------

  // bufferSize is actually 'number of sample frames', so was renamed
  // bufferFrameCapacity here for clarity.
  alcCaptureOpenDevice__proxy: 'sync',
  alcCaptureOpenDevice__sig: 'iiiii',
  alcCaptureOpenDevice: function(pDeviceName, requestedSampleRate, format, bufferFrameCapacity) {

    var resolvedDeviceName = AL.CAPTURE_DEVICE_NAME;

    // NULL is a valid device name here (resolves to default);
    if (pDeviceName !== 0) {
      resolvedDeviceName = Pointer_stringify(pDeviceName);
      if (resolvedDeviceName !== AL.CAPTURE_DEVICE_NAME) {
#if OPENAL_DEBUG
        console.error('alcCaptureOpenDevice() with invalid device name \''+resolvedDeviceName+'\'');
#endif
        // ALC_OUT_OF_MEMORY
        // From the programmer's guide, ALC_OUT_OF_MEMORY's meaning is
        // overloaded here, to mean:
        // 'The specified device is invalid, or can not capture audio.'
        // This may be misleading to API users, but well...
        AL.alcErr = 0xA005 /* ALC_OUT_OF_MEMORY */;
        return 0;
      }
    }

    // Otherwise it's probably okay (though useless) for bufferFrameCapacity to be zero.
    if (bufferFrameCapacity < 0) { // ALCsizei is signed int
#if OPENAL_DEBUG
      console.error('alcCaptureOpenDevice() with negative bufferSize');
#endif
      AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
      return 0;
    }

    navigator.getUserMedia = navigator.getUserMedia 
      || navigator.webkitGetUserMedia 
      || navigator.mozGetUserMedia 
      || navigator.msGetUserMedia;
    var has_getUserMedia = navigator.getUserMedia 
      || (navigator.mediaDevices 
      &&  navigator.mediaDevices.getUserMedia);

    if (!has_getUserMedia) {
#if OPENAL_DEBUG
      console.error('alcCaptureOpenDevice() cannot capture audio, because your browser lacks a `getUserMedia()` implementation');
#endif
      // See previously mentioned rationale for ALC_OUT_OF_MEMORY
      AL.alcErr = 0xA005 /* ALC_OUT_OF_MEMORY */;
      return 0;
    }

    var AudioContext = window.AudioContext || window.webkitAudioContext;

    if (!AL.sharedCaptureAudioCtx) {
      try {
        AL.sharedCaptureAudioCtx = new AudioContext();
      } catch(e) {
#if OPENAL_DEBUG
        console.error('alcCaptureOpenDevice() could not create the shared capture AudioContext: ' + e);
#endif
        // See previously mentioned rationale for ALC_OUT_OF_MEMORY
        AL.alcErr = 0xA005 /* ALC_OUT_OF_MEMORY */;
        return 0;
      }
    }

    var outputChannelCount;

    switch (format) {
    case 0x10010: /* AL_FORMAT_MONO_FLOAT32 */
    case 0x1101:  /* AL_FORMAT_MONO16 */
    case 0x1100:  /* AL_FORMAT_MONO8 */
      outputChannelCount = 1;
      break;
    case 0x10011: /* AL_FORMAT_STEREO_FLOAT32 */
    case 0x1103:  /* AL_FORMAT_STEREO16 */
    case 0x1102:  /* AL_FORMAT_STEREO8 */
      outputChannelCount = 2;
      break;
    default:
#if OPENAL_DEBUG
      console.error('alcCaptureOpenDevice() with unsupported format ' + format);
#endif
      AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
      return 0;
    }

    function newF32Array(cap) { return new Float32Array(cap);}
    function newI16Array(cap) { return new Int16Array(cap);  }
    function newU8Array(cap)  { return new Uint8Array(cap);  }

    var requestedSampleType;
    var newSampleArray;

    switch (format) {
    case 0x10010: /* AL_FORMAT_MONO_FLOAT32 */
    case 0x10011: /* AL_FORMAT_STEREO_FLOAT32 */
      requestedSampleType = 'f32';
      newSampleArray = newF32Array;
      break;
    case 0x1101:  /* AL_FORMAT_MONO16 */
    case 0x1103:  /* AL_FORMAT_STEREO16 */
      requestedSampleType = 'i16';
      newSampleArray = newI16Array;
      break;
    case 0x1100:  /* AL_FORMAT_MONO8 */
    case 0x1102:  /* AL_FORMAT_STEREO8 */
      requestedSampleType = 'u8';
      newSampleArray = newU8Array;
      break;
    }

    var buffers = [];
    try {
      for (var chan=0; chan < outputChannelCount; ++chan) {
        buffers[chan] = newSampleArray(bufferFrameCapacity);
      }
    } catch(e) {
#if OPENAL_DEBUG
      console.error('alcCaptureOpenDevice() failed to allocate internal buffers (is bufferSize low enough?): ' + e);
#endif
      AL.alcErr = 0xA005 /* ALC_OUT_OF_MEMORY */;
      return 0;
    }


    // What we'll place into the `AL.captures` array in the end,
    // declared here for closures to access it
    var newCapture = {
      audioCtx: AL.sharedCaptureAudioCtx,
      deviceName: resolvedDeviceName,
      requestedSampleRate: requestedSampleRate,
      requestedSampleType: requestedSampleType,
      outputChannelCount: outputChannelCount,
      inputChannelCount: null, // Not known until the getUserMedia() promise resolves
      mediaStreamError: null, // Used by other functions to return early and report an error.
      mediaStreamSourceNode: null,
      // Either one, or none of the below two, is active.
      mergerNode: null,
      splitterNode: null,
      scriptProcessorNode: null,
      isCapturing: false,
      buffers: buffers,
      get bufferFrameCapacity() {
        return buffers[0].length;
      },
      capturePlayhead: 0, // current write position, in sample frames
      capturedFrameCount: 0
    };

    // Preparing for getUserMedia()

    var onError = function(mediaStreamError) {
      newCapture.mediaStreamError = mediaStreamError;
#if OPENAL_DEBUG
      console.error('navigator.getUserMedia() errored with: ' + mediaStreamError);
#endif
    };
    var onSuccess = function(mediaStream) {
      newCapture.mediaStreamSourceNode = newCapture.audioCtx.createMediaStreamSource(mediaStream);

      var inputChannelCount = 1;
      switch(newCapture.mediaStreamSourceNode.channelCountMode) {
      case 'max':
        inputChannelCount = outputChannelCount;
        break;
      case 'clamped-max':
        inputChannelCount = Math.min(outputChannelCount, newCapture.mediaStreamSourceNode.channelCount);
        break;
      case 'explicit':
        inputChannelCount = newCapture.mediaStreamSourceNode.channelCount;
        break;
      }

      newCapture.inputChannelCount = inputChannelCount;

#if OPENAL_DEBUG
      if (inputChannelCount > 2 || outputChannelCount > 2) {
        console.warn('The number of input or output channels is too high, capture might not work as expected!');
      }
#endif

      // Have to pick a size from 256, 512, 1024, 2048, 4096, 8192, 16384.
      // One can also set it to zero, which leaves the decision up to the impl.
      // An extension could allow specifying this value.
      var processorFrameCount = 512;

      newCapture.scriptProcessorNode = newCapture.audioCtx.createScriptProcessor(
        processorFrameCount, inputChannelCount, outputChannelCount
      );

      if (inputChannelCount > outputChannelCount) {
        newCapture.mergerNode = newCapture.audioCtx.createChannelMerger(inputChannelCount);
        newCapture.mediaStreamSourceNode.connect(newCapture.mergerNode);
        newCapture.mergerNode.connect(newCapture.scriptProcessorNode);
      } else if (inputChannelCount < outputChannelCount) {
        newCapture.splitterNode = newCapture.audioCtx.createChannelSplitter(outputChannelCount);
        newCapture.mediaStreamSourceNode.connect(newCapture.splitterNode);
        newCapture.splitterNode.connect(newCapture.scriptProcessorNode);
      } else {
        newCapture.mediaStreamSourceNode.connect(newCapture.scriptProcessorNode);
      }

      newCapture.scriptProcessorNode.connect(newCapture.audioCtx.destination);

      newCapture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {

        if (!newCapture.isCapturing) {
          return;
        }

        var c = newCapture;
        var srcBuf = audioProcessingEvent.inputBuffer;

        // Actually just copy srcBuf's channel data into
        // c.buffers, optimizing for each case.
        switch (format) {
        case 0x10010: /* AL_FORMAT_MONO_FLOAT32 */
          var channel0 = srcBuf.getChannelData(0);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i];
          }
          break;
        case 0x10011: /* AL_FORMAT_STEREO_FLOAT32 */
          var channel0 = srcBuf.getChannelData(0);
          var channel1 = srcBuf.getChannelData(1);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i];
            c.buffers[1][wi] = channel1[i];
          }
          break;
        case 0x1101:  /* AL_FORMAT_MONO16 */
          var channel0 = srcBuf.getChannelData(0);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i] * 32767;
          }
          break;
        case 0x1103:  /* AL_FORMAT_STEREO16 */
          var channel0 = srcBuf.getChannelData(0);
          var channel1 = srcBuf.getChannelData(1);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i] * 32767;
            c.buffers[1][wi] = channel1[i] * 32767;
          }
          break;
        case 0x1100:  /* AL_FORMAT_MONO8 */
          var channel0 = srcBuf.getChannelData(0);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = (channel0[i] + 1.0) * 127;
          }
          break;
        case 0x1102:  /* AL_FORMAT_STEREO8 */
          var channel0 = srcBuf.getChannelData(0);
          var channel1 = srcBuf.getChannelData(1);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = (channel0[i] + 1.0) * 127;
            c.buffers[1][wi] = (channel1[i] + 1.0) * 127;
          }
          break;
        }

        c.capturePlayhead += srcBuf.length;
        c.capturePlayhead %= c.bufferFrameCapacity;
        c.capturedFrameCount += srcBuf.length;
        c.capturedFrameCount = Math.min(c.capturedFrameCount, c.bufferFrameCapacity);
      };
    };

    // The latest way to call getUserMedia()
    if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
      navigator.mediaDevices
           .getUserMedia({audio: true})
           .then(onSuccess)
           .catch(onError);
    } else { // The usual (now deprecated) way
      navigator.getUserMedia({audio: true}, onSuccess, onError);
    }

    var id = AL.newId();
    AL.captures[id] = newCapture;
    return id;
  },

  alcCaptureCloseDevice__proxy: 'sync',
  alcCaptureCloseDevice__sig: 'ii',
  alcCaptureCloseDevice: function(deviceId) {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureCloseDevice');
    if (!c) return false;

    delete AL.captures[deviceId];
    AL.freeIds.push(deviceId);

    // This clean-up might be unnecessary (paranoid) ?

    // May happen if user hasn't decided to grant or deny input
    if (c.mediaStreamSourceNode) c.mediaStreamSourceNode.disconnect();
    if (c.mergerNode) c.mergerNode.disconnect();
    if (c.splitterNode) c.splitterNode.disconnect();
    // May happen if user hasn't decided to grant or deny input
    if (c.scriptProcessorNode) c.scriptProcessorNode.disconnect();

    delete c.buffers;

    c.capturedFrameCount = 0;
    c.isCapturing = false;

    return true;
  },

  alcCaptureStart__proxy: 'sync',
  alcCaptureStart__sig: 'vi',
  alcCaptureStart: function(deviceId) {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureStart');
    if (!c) return;

    if (c.isCapturing) {
#if OPENAL_DEBUG
      console.warn('Redundant call to alcCaptureStart()');
#endif
      // NOTE: Spec says (emphasis mine):
      //     The amount of audio samples available after **restarting** a 
      //     stopped capture device is reset to zero.
      // So redundant calls to alcCaptureStart() must have no effect.
      return;
    }
    c.isCapturing = true;
    c.capturedFrameCount = 0;
    c.capturePlayhead = 0;
  },

  alcCaptureStop__proxy: 'sync',
  alcCaptureStop__sig: 'vi',
  alcCaptureStop: function(deviceId) {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureStop');
    if (!c) return;

#if OPENAL_DEBUG
    if (!c.isCapturing) {
      console.warn('Redundant call to alcCaptureStop()');
    }
#endif
    c.isCapturing = false;
  },

  // The OpenAL spec hints that implementations are allowed to 
  // 'defer resampling and other conversions' up until this point.
  //
  // The last parameter is actually 'number of sample frames', so was
  // renamed accordingly here
  alcCaptureSamples__proxy: 'sync',
  alcCaptureSamples__sig: 'viii',
  alcCaptureSamples: function(deviceId, pFrames, requestedFrameCount) {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureSamples');
    if (!c) return;

    // ALCsizei is actually 32-bit signed int, so could be negative
    // Also, spec says :
    //   Requesting more sample frames than are currently available is 
    //   an error.
    if (requestedFrameCount < 0
    ||  requestedFrameCount > c.capturedFrameCount) 
    {
  // if OPENAL_DEBUG
      console.error('alcCaptureSamples() with invalid bufferSize');
  // endif
      AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
      return;
    }
    
    function setF32Sample(i, sample) {
      {{{ makeSetValue('pFrames', '4*i', 'sample', 'float') }}};
    }
    function setI16Sample(i, sample) {
      {{{ makeSetValue('pFrames', '2*i', 'sample', 'i16') }}};
    }
    function setU8Sample(i, sample) {
      {{{ makeSetValue('pFrames', 'i', 'sample', 'i8') }}};
    }

    var setSample;

    switch(c.requestedSampleType) {
    case 'f32': setSample = setF32Sample; break;
    case 'i16': setSample = setI16Sample; break;
    case 'u8' : setSample = setU8Sample ; break;
    default: 
#if OPENAL_DEBUG
      console.error('Internal error: Unknown sample type \''+c.requestedSampleType+'\'');
#endif
      return;
    }

    var dstfreq = c.requestedSampleRate;
    var srcfreq = c.audioCtx.sampleRate;

    if (srcfreq == dstfreq) {
      for (var i = 0, frame_i = 0; frame_i < requestedFrameCount; ++frame_i) {
        for (var chan = 0; chan < c.buffers.length; ++chan, ++i) {
          var src_i = (frame_i + c.capturePlayhead) % c.capturedFrameCount;
          setSample(i, c.buffers[chan][src_i]);
        }
      }
    } else {
      // Perform linear resampling.

      // There is room for improvement - right now we're fine with linear resampling.
      // We don't use OfflineAudioContexts for this: See the discussion at
      // https://github.com/jpernst/emscripten/issues/2#issuecomment-312729735
      // if you're curious about why.

      function lerp(from, to, progress) {
        return (1 - progress) * from + progress * to;
      }

      for (var i = 0, frame_i = 0; frame_i < requestedFrameCount; ++frame_i) {

        var t = frame_i / dstfreq; // Most exact time for the current output sample
        var src_i = (Math.floor(t*srcfreq) + c.capturePlayhead) % c.capturedFrameCount;
        var src_next_i = (src_i+1) % c.capturedFrameCount;
        var between = t*srcfreq - src_i; //(t - src_i/srcfreq) / ((src_i+1)/srcfreq - src_i/srcfreq);

        for (var chan = 0; chan < c.buffers.length; ++chan, ++i) {
          var cb = c.buffers[chan];
          var sample = lerp(cb[src_i], cb[src_next_i], between);
          setSample(i, sample);
        }
      }
    }

    // Spec doesn't say if alcCaptureSamples() must zero the number
    // of available captured sample-frames, but not only would it
    // be insane not to do, OpenAL-Soft happens to do that as well.
    c.capturedFrameCount = 0;
  },


  // -------------------------------------------------------
  // -- ALC Resources
  // -------------------------------------------------------

  alcOpenDevice__proxy: 'sync',
  alcOpenDevice__sig: 'ii',
  alcOpenDevice: function(pDeviceName) {
    if (pDeviceName) {
      var name = Pointer_stringify(pDeviceName);
      if (name !== AL.DEVICE_NAME) {
        return 0;
      }
    }

    if (typeof(AudioContext) !== 'undefined' || typeof(webkitAudioContext) !== 'undefined') {
      var deviceId = AL.newId();
      AL.deviceRefCounts[deviceId] = 0;
      return deviceId;
    } else {
      return 0;
    }
  },

  alcCloseDevice__proxy: 'sync',
  alcCloseDevice__sig: 'ii',
  alcCloseDevice: function(deviceId) {
    if (!deviceId in AL.deviceRefCounts || AL.deviceRefCounts[deviceId] > 0) {
      return 0 /* ALC_FALSE */;
    }

    delete AL.deviceRefCounts[deviceId];
    AL.freeIds.push(deviceId);
    return 1 /* ALC_TRUE */;
  },

  alcCreateContext__proxy: 'sync',
  alcCreateContext__sig: 'iii',
  alcCreateContext: function(deviceId, pAttrList) {
    if (!deviceId in AL.deviceRefCounts) {
#if OPENAL_DEBUG
      console.log('alcCreateContext() called with an invalid device');
#endif
      AL.alcErr = 0xA001; /* ALC_INVALID_DEVICE */
      return 0;
    }

    var options = null;
    var attrs = [];
    var hrtf = null;
    pAttrList >>= 2;
    if (pAttrList) {
      var attr = 0;
      var val = 0;
      while (true) {
        attr = HEAP32[pAttrList++];
        attrs.push(attr);
        if (attr === 0) {
          break;
        }
        val = HEAP32[pAttrList++];
        attrs.push(val);

        switch (attr) {
        case 0x1007 /* ALC_FREQUENCY */:
          if (!options) {
            options = {};
          }

          options.sampleRate = val;
          break;
        case 0x1010 /* ALC_MONO_SOURCES */: // fallthrough
        case 0x1011 /* ALC_STEREO_SOURCES */:
          // Do nothing; these hints are satisfied by default
          break
        case 0x1992 /* ALC_HRTF_SOFT */:
          switch (val) {
            case 0 /* ALC_FALSE */:
              hrtf = false;
              break;
            case 1 /* ALC_TRUE */:
              hrtf = true;
              break;
            case 2 /* ALC_DONT_CARE_SOFT */:
              break;
            default:
#if OPENAL_DEBUG
              console.log('Unsupported ALC_HRTF_SOFT mode ' + val);
#endif
              AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
              return 0;
          }
          break;
        case 0x1996 /* ALC_HRTF_ID_SOFT */:
          if (val !== 0) {
#if OPENAL_DEBUG
            console.log('Invalid ALC_HRTF_ID_SOFT index ' + val);
#endif
            AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
            return 0;
          }
          break;
        default:
#if OPENAL_DEBUG
          console.log('Unsupported context attribute 0x' + attr.toString(16));
#endif
          AL.alcErr = 0xA004; /* ALC_INVALID_VALUE */
          return 0;
        }
      }
    }

    var AudioContext = window.AudioContext || window.webkitAudioContext;
    var ac = null;
    try {
      // Only try to pass options if there are any, for compat with browsers that don't support this
      if (options) {
        ac = new AudioContext(options);
      } else {
        ac = new AudioContext();
      }
    } catch (e) {
      if (e.name === 'NotSupportedError') {
#if OPENAL_DEBUG
        console.log('Invalid or unsupported options');
#endif
        AL.alcErr = 0xA004; /* ALC_INVALID_VALUE */
      } else {
        AL.alcErr = 0xA001; /* ALC_INVALID_DEVICE */
      }

      return 0;
    }

    // Old Web Audio API (e.g. Safari 6.0.5) had an inconsistently named createGainNode function.
    if (typeof(ac.createGain) === 'undefined') {
      ac.createGain = ac.createGainNode;
    }

    var gain = ac.createGain();
    gain.connect(ac.destination);
    var ctx = {
      deviceId: deviceId,
      id: AL.newId(),
      attrs: attrs,
      audioCtx: ac,
      listener: {
    	  position: [0.0, 0.0, 0.0],
    	  velocity: [0.0, 0.0, 0.0],
    	  direction: [0.0, 0.0, 0.0],
    	  up: [0.0, 0.0, 0.0]
      },
      sources: [],
      interval: setInterval(function() { AL.scheduleContextAudio(ctx); }, AL.QUEUE_INTERVAL),
      gain: gain,
      distanceModel: 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */,
      speedOfSound: 343.3,
      dopplerFactor: 1.0,
      sourceDistanceModel: false,
      hrtf: hrtf || false,

      _err: 0,
      get err() {
        return this._err;
      },
      set err(val) {
        // Errors should not be overwritten by later errors until they are cleared by a query.
        if (this._err === 0 /* AL_NO_ERROR */ || val === 0 /* AL_NO_ERROR */) {
          this._err = val;
        }
      }
    };
    AL.deviceRefCounts[deviceId]++;
    AL.contexts[ctx.id] = ctx;

    if (hrtf !== null) {
      // Apply hrtf attrib to all contexts for this device
      for (var ctxId in AL.contexts) {
        var c = AL.contexts[ctxId];
        if (c.deviceId === deviceId) {
          c.hrtf = hrtf;
          AL.updateContextGlobal(c);
        }
      }
    }

    return ctx.id;
  },

  alcDestroyContext__proxy: 'sync',
  alcDestroyContext__sig: 'vi',
  alcDestroyContext: function(contextId) {
    var ctx = AL.contexts[contextId];
    if (AL.currentCtx === ctx) {
#if OPENAL_DEBUG
      console.log('alcDestroyContext() called with an invalid context');
#endif
      AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
      return;
    }

    // Stop playback, etc
    if (AL.contexts[contextId].interval) {
      clearInterval(AL.contexts[contextId].interval);
    }
    AL.deviceRefCounts[ctx.deviceId]--;
    delete AL.contexts[contextId];
    AL.freeIds.push(contextId);
  },

  // -------------------------------------------------------
  // -- ALC State
  // -------------------------------------------------------

  alcGetError__proxy: 'sync',
  alcGetError__sig: 'ii',
  alcGetError: function(deviceId) {
    var err = AL.alcErr;
    AL.alcErr = 0 /* ALC_NO_ERROR */;
    return err;
  },

  alcGetCurrentContext__proxy: 'sync',
  alcGetCurrentContext__sig: 'i',
  alcGetCurrentContext: function() {
    if (AL.currentCtx !== null) {
      return AL.currentCtx.id;
    } else {
      return 0;
    }
  },

  alcMakeContextCurrent__proxy: 'sync',
  alcMakeContextCurrent__sig: 'ii',
  alcMakeContextCurrent: function(contextId) {
    if (contextId === 0) {
      AL.currentCtx = null;
      return 0;
    } else {
      AL.currentCtx = AL.contexts[contextId];
      return 1;
    }
  },

  alcGetContextsDevice__proxy: 'sync',
  alcGetContextsDevice__sig: 'ii',
  alcGetContextsDevice: function(contextId) {
    if (contextId in AL.contexts) {
      return AL.contexts[contextId].deviceId;
    } else {
      return 0;
    }
  },

  // The spec is vague about what these are actually supposed to do, and NOP is a reasonable implementation
  alcProcessContext: function(contextId) {},
  alcSuspendContext: function(contextId) {},

  alcIsExtensionPresent__proxy: 'sync',
  alcIsExtensionPresent__sig: 'iii',
  alcIsExtensionPresent: function(deviceId, pExtName) {
    name = Pointer_stringify(pExtName);

    return AL.ALC_EXTENSIONS[name] ? 1 : 0;
  },

  alcGetProcAddress__deps: ['emscripten_GetAlcProcAddress'],
  alcGetProcAddress__proxy: 'sync',
  alcGetProcAddress__sig: 'iii',
  alcGetProcAddress: function(deviceId, pProcName) {
    if (!pProcName) {
#if OPENAL_DEBUG
      console.error('alcGetProcAddress() called with null name pointer');
#endif
      AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
      return 0; /* ALC_NONE */
    }
    return _emscripten_GetAlcProcAddress(pProcName);
  },

  alcGetEnumValue__proxy: 'sync',
  alcGetEnumValue__sig: 'iii',
  alcGetEnumValue: function(deviceId, pEnumName) {
    // Spec says :
    // Using a NULL handle is legal, but only the
    // tokens defined by the AL core are guaranteed.
    if (deviceId !== 0 && !deviceId in AL.deviceRefCounts) {
#if OPENAL_DEBUG
      console.error('alcGetEnumValue() called with an invalid device');
#endif
      // ALC_INVALID_DEVICE is not listed as a possible error state for
      // this function, sadly.
      return 0 /* ALC_NONE */;
    } else if (!pEnumName) {
      AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
      return 0; /* ALC_NONE */
    }
    name = Pointer_stringify(pEnumName);
    // See alGetEnumValue(), but basically behave the same as OpenAL-Soft
    switch(name) {
    case 'ALC_NO_ERROR': return 0;
    case 'ALC_INVALID_DEVICE': return 0xA001;
    case 'ALC_INVALID_CONTEXT': return 0xA002;
    case 'ALC_INVALID_ENUM': return 0xA003;
    case 'ALC_INVALID_VALUE': return 0xA004;
    case 'ALC_OUT_OF_MEMORY': return 0xA005;
    case 'ALC_MAJOR_VERSION': return 0x1000;
    case 'ALC_MINOR_VERSION': return 0x1001;
    case 'ALC_ATTRIBUTES_SIZE': return 0x1002;
    case 'ALC_ALL_ATTRIBUTES': return 0x1003;
    case 'ALC_DEFAULT_DEVICE_SPECIFIER': return 0x1004;
    case 'ALC_DEVICE_SPECIFIER': return 0x1005;
    case 'ALC_EXTENSIONS': return 0x1006;
    case 'ALC_FREQUENCY': return 0x1007;
    case 'ALC_REFRESH': return 0x1008;
    case 'ALC_SYNC': return 0x1009;
    case 'ALC_MONO_SOURCES': return 0x1010;
    case 'ALC_STEREO_SOURCES': return 0x1011;
    case 'ALC_CAPTURE_DEVICE_SPECIFIER': return 0x310;
    case 'ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER': return 0x311;
    case 'ALC_CAPTURE_SAMPLES': return 0x312;

    /* Extensions */
    case 'ALC_HRTF_SOFT': return 0x1992;
    case 'ALC_HRTF_ID_SOFT': return 0x1996;
    case 'ALC_DONT_CARE_SOFT': return 0x0002;
    case 'ALC_HRTF_STATUS_SOFT': return 0x1993;
    case 'ALC_NUM_HRTF_SPECIFIERS_SOFT': return 0x1994;
    case 'ALC_HRTF_SPECIFIER_SOFT': return 0x1995;
    case 'ALC_HRTF_DISABLED_SOFT': return 0x0000;
    case 'ALC_HRTF_ENABLED_SOFT': return 0x0001;
    case 'ALC_HRTF_DENIED_SOFT': return 0x0002;
    case 'ALC_HRTF_REQUIRED_SOFT': return 0x0003;
    case 'ALC_HRTF_HEADPHONES_DETECTED_SOFT': return 0x0004;
    case 'ALC_HRTF_UNSUPPORTED_FORMAT_SOFT': return 0x0005;

    default:
#if OPENAL_DEBUG
      console.error('No value for `' + pEnumName + '` is known by alcGetEnumValue()');
#endif
      AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
      return 0 /* AL_NONE */;
    }
  },

  alcGetString__proxy: 'sync',
  alcGetString__sig: 'iii',
  alcGetString: function(deviceId, param) {
    if (AL.alcStringCache[param]) {
      return AL.alcStringCache[param];
    }

    var ret;
    switch (param) {
    case 0 /* ALC_NO_ERROR */:
      ret = 'No Error';
      break;
    case 0xA001 /* ALC_INVALID_DEVICE */:
      ret = 'Invalid Device';
      break;
    case 0xA002 /* ALC_INVALID_CONTEXT */:
      ret = 'Invalid Context';
      break;
    case 0xA003 /* ALC_INVALID_ENUM */:
      ret = 'Invalid Enum';
      break;
    case 0xA004 /* ALC_INVALID_VALUE */:
      ret = 'Invalid Value';
      break;
    case 0xA005 /* ALC_OUT_OF_MEMORY */:
      ret = 'Out of Memory';
      break;
    case 0x1004 /* ALC_DEFAULT_DEVICE_SPECIFIER */:
      if (typeof(AudioContext) !== 'undefined' ||
          typeof(webkitAudioContext) !== 'undefined') {
        ret = AL.DEVICE_NAME;
      } else {
        return 0;
      }
      break;
    case 0x1005 /* ALC_DEVICE_SPECIFIER */:
      if (typeof(AudioContext) !== 'undefined' ||
          typeof(webkitAudioContext) !== 'undefined') {
        ret = AL.DEVICE_NAME.concat('\0');
      } else {
        ret = '\0';
      }
      break;
    case 0x311 /* ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER */:
      ret = AL.CAPTURE_DEVICE_NAME;
      break;
    case 0x310 /* ALC_CAPTURE_DEVICE_SPECIFIER */:
      if (deviceId === 0) 
        ret = AL.CAPTURE_DEVICE_NAME.concat('\0');
        else {
        var c = AL.requireValidCaptureDevice(deviceId, 'alcGetString');
        if (!c) {
          return 0;
        }
        ret = c.deviceName;
      }  
      break;
    case 0x1006 /* ALC_EXTENSIONS */:
      if (!deviceId) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return 0;
      }

      ret = '';
      for (var ext in AL.ALC_EXTENSIONS) {
        ret = ret.concat(ext);
        ret = ret.concat(' ');
      }
      ret = ret.trim();
      break;
    default:
      AL.alcErr = 0xA003 /* ALC_INVALID_ENUM */;
      return 0;
    }

    ret = allocate(intArrayFromString(ret), 'i8', ALLOC_NORMAL);
    AL.alcStringCache[param] = ret;
    return ret;
  },

  alcGetIntegerv__proxy: 'sync',
  alcGetIntegerv__sig: 'viiii',
  alcGetIntegerv: function(deviceId, param, size, pValues) {
    if (size === 0 || !pValues) {
      // Ignore the query, per the spec
      return;
    }

    switch(param) {
    case 0x1000 /* ALC_MAJOR_VERSION */:
      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
      break;
    case 0x1001 /* ALC_MINOR_VERSION */:
      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
      break;
    case 0x1002 /* ALC_ATTRIBUTES_SIZE */:
      if (!deviceId in AL.deviceRefCounts) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
        return;
      }

      {{{ makeSetValue('pValues', '0', 'AL.currentCtx.attrs.length', 'i32') }}};
      break;
    case 0x1003 /* ALC_ALL_ATTRIBUTES */:
      if (!deviceId in AL.deviceRefCounts) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
        return;
      }

      for (var i = 0; i < AL.currentCtx.attrs.length; i++) {
        {{{ makeSetValue('pValues', 'i*4', 'AL.currentCtx.attrs[i]', 'i32') }}};
      }
      break;
    case 0x1007 /* ALC_FREQUENCY */:
      if (!deviceId in AL.deviceRefCounts) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
        return;
      }

      {{{ makeSetValue('pValues', '0', 'AL.currentCtx.audioCtx.sampleRate', 'i32') }}};
      break;
    case 0x1010 /* ALC_MONO_SOURCES */:
    case 0x1011 /* ALC_STEREO_SOURCES */:
      if (!deviceId in AL.deviceRefCounts) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
        return;
      }

      {{{ makeSetValue('pValues', '0', '0x7FFFFFFF', 'i32') }}};
      break;
    case 0x1992 /* ALC_HRTF_SOFT */:
    case 0x1993 /* ALC_HRTF_STATUS_SOFT */:
      if (!deviceId in AL.deviceRefCounts) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return;
      }

      var hrtfStatus = 0 /* ALC_HRTF_DISABLED_SOFT */;
      for (var ctxId in AL.contexts) {
        var ctx = AL.contexts[ctxId];
        if (ctx.deviceId === deviceId) {
          hrtfStatus = ctx.hrtf ? 1 /* ALC_HRTF_ENABLED_SOFT */ : 0 /* ALC_HRTF_DISABLED_SOFT */;
        }
      }
      {{{ makeSetValue('pValues', '0', 'hrtfStatus', 'i32') }}};
      break;
    case 0x1994 /* ALC_NUM_HRTF_SPECIFIERS_SOFT */:
      if (!deviceId in AL.deviceRefCounts) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return;
      }
      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
      break;
    case 0x20003 /* ALC_MAX_AUXILIARY_SENDS */:
      if (!deviceId in AL.deviceRefCounts) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
        return;
      }

      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
    case 0x312 /* ALC_CAPTURE_SAMPLES */:
      var c = AL.requireValidCaptureDevice(deviceId, 'alcGetIntegerv');
      if (!c) {
        return;
      }
      var n = c.capturedFrameCount;
      var dstfreq = c.requestedSampleRate;
      var srcfreq = c.audioCtx.sampleRate;
      var nsamples = Math.floor(n * (dstfreq/srcfreq));
      {{{ makeSetValue('pValues', '0', 'nsamples', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.log('alcGetIntegerv() with param 0x' + param.toString(16) + ' not implemented yet');
#endif
      AL.alcErr = 0xA003 /* ALC_INVALID_ENUM */;
      return;
    }
  },

  emscripten_alcDevicePauseSOFT__proxy: 'sync',
  emscripten_alcDevicePauseSOFT__sig: 'vi',
  emscripten_alcDevicePauseSOFT: function(deviceId) {
    if (!deviceId in AL.deviceRefCounts) {
#if OPENAL_DEBUG
      console.log('alcDevicePauseSOFT() called with an invalid device');
#endif
      AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
      return;
    }

    if (AL.paused) {
      return;
    }
    AL.paused = true;

    for (ctxId in AL.contexts) {
      var ctx = AL.contexts[ctxId];
      if (ctx.deviceId !== deviceId) {
        continue;
      }

      ctx.audioCtx.suspend();
      clearInterval(ctx.interval);
      ctx.interval = null;
    }
  },

  emscripten_alcDeviceResumeSOFT__proxy: 'sync',
  emscripten_alcDeviceResumeSOFT__sig: 'vi',
  emscripten_alcDeviceResumeSOFT: function(deviceId) {
    if (!deviceId in AL.deviceRefCounts) {
#if OPENAL_DEBUG
      console.log('alcDeviceResumeSOFT() called with an invalid device');
#endif
      AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
      return;
    }

    if (!AL.paused) {
      return;
    }
    AL.paused = false;

    for (ctxId in AL.contexts) {
      var ctx = AL.contexts[ctxId];
      if (ctx.deviceId !== deviceId) {
        continue;
      }

      ctx.interval = setInterval(function() { AL.scheduleContextAudio(ctx); }, AL.QUEUE_INTERVAL);
      ctx.audioCtx.resume();
    }
  },

  emscripten_alcGetStringiSOFT__proxy: 'sync',
  emscripten_alcGetStringiSOFT__sig: 'iiii',
  emscripten_alcGetStringiSOFT__deps: ['alcGetString'],
  emscripten_alcGetStringiSOFT: function(deviceId, param, index) {
    if (!deviceId in AL.deviceRefCounts) {
#if OPENAL_DEBUG
      console.log('alcGetStringiSOFT() called with an invalid device');
#endif
      AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
      return 0;
    }

    if (AL.alcStringCache[param]) {
      return AL.alcStringCache[param];
    }

    var ret;
    switch (param) {
    case 0x1995 /* ALC_HRTF_SPECIFIER_SOFT */:
      if (index === 0) {
        ret = 'Web Audio HRTF';
      } else {
#if OPENAL_DEBUG
        console.log('alcGetStringiSOFT() with param ALC_HRTF_SPECIFIER_SOFT index ' + index + ' is out of range');
#endif
        AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
        return 0;
      }
    default:
      if (index === 0) {
        return _alcGetString(deviceId, param);
      } else {
#if OPENAL_DEBUG
        console.log('alcGetStringiSOFT() with param 0x' + param.toString(16) + ' not implemented yet');
#endif
        AL.alcErr = 0xA003 /* ALC_INVALID_ENUM */;
        return 0;
      }
    }

    ret = allocate(intArrayFromString(ret), 'i8', ALLOC_NORMAL);
    AL.alcStringCache[param] = ret;
    return ret;
  },

  emscripten_alcResetDeviceSOFT__proxy: 'sync',
  emscripten_alcResetDeviceSOFT__sig: 'iii',
  emscripten_alcResetDeviceSOFT: function(deviceId, pAttrList) {
    if (!deviceId in AL.deviceRefCounts) {
#if OPENAL_DEBUG
      console.log('alcResetDeviceSOFT() called with an invalid device');
#endif
      AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
      return 0 /* ALC_FALSE */;
    }

    var hrtf = null;
    pAttrList >>= 2;
    if (pAttrList) {
      var attr = 0;
      var val = 0;
      while (true) {
        attr = HEAP32[pAttrList++];
        if (attr === 0) {
          break;
        }
        val = HEAP32[pAttrList++];

        switch (attr) {
        case 0x1992 /* ALC_HRTF_SOFT */:
          if (val === 1 /* ALC_TRUE */) {
            hrtf = true;
          } else if (val === 0 /* ALC_FALSE */) {
            hrtf = false;
          }
          break;
        }
      }
    }

    if (hrtf !== null) {
      // Apply hrtf attrib to all contexts for this device
      for (var ctxId in AL.contexts) {
        var ctx = AL.contexts[ctxId];
        if (ctx.deviceId === deviceId) {
          ctx.hrtf = hrtf;
          AL.updateContextGlobal(ctx);
        }
      }
    }

    return 1 /* ALC_TRUE */;
  },

  // ***************************************************************************
  // ** AL API 
  // ***************************************************************************

  // -------------------------------------------------------
  // -- AL Resources
  // -------------------------------------------------------

  alGenBuffers__proxy: 'sync',
  alGenBuffers__sig: 'vii',
  alGenBuffers: function(count, pBufferIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alGenBuffers() called without a valid context');
#endif
      return;
    }

    for (var i = 0; i < count; ++i) {
      var buf = {
        deviceId: AL.currentCtx.deviceId,
        id: AL.newId(),
        refCount: 0,
        audioBuf: null,
        frequency: 0,
        bytesPerSample: 2,
        channels: 1,
        length: 0,
      };
      AL.deviceRefCounts[buf.deviceId]++;
      AL.buffers[buf.id] = buf;
      {{{ makeSetValue('pBufferIds', 'i*4', 'buf.id', 'i32') }}};
    }
  },

  alDeleteBuffers__proxy: 'sync',
  alDeleteBuffers__sig: 'vii',
  alDeleteBuffers: function(count, pBufferIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alDeleteBuffers() called without a valid context');
#endif
      return;
    }

    for (var i = 0; i < count; ++i) {
      var bufId = {{{ makeGetValue('pBufferIds', 'i*4', 'i32') }}};
      /// Deleting the zero buffer is a legal NOP, so ignore it
      if (bufId === 0) {
        continue;
      }

      // Make sure the buffer index is valid.
      if (!AL.buffers[bufId]) {
#if OPENAL_DEBUG
        console.error('alDeleteBuffers() called with an invalid buffer');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }

      // Make sure the buffer is no longer in use.
      if (AL.buffers[bufId].refCount) {
#if OPENAL_DEBUG
        console.error('alDeleteBuffers() called with a used buffer');
#endif
        AL.currentCtx.err = 0xA004 /* AL_INVALID_OPERATION */;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      var bufId = {{{ makeGetValue('pBufferIds', 'i*4', 'i32') }}};
      if (bufId === 0) {
        continue;
      }

      AL.deviceRefCounts[AL.buffers[bufId].deviceId]--;
      delete AL.buffers[bufId];
      AL.freeIds.push(bufId);
    }
  },

  alGenSources__proxy: 'sync',
  alGenSources__sig: 'vii',
  alGenSources: function(count, pSourceIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alGenSources() called without a valid context');
#endif
      return;
    }
    for (var i = 0; i < count; ++i) {
      var gain = AL.currentCtx.audioCtx.createGain();
      gain.connect(AL.currentCtx.gain);
      var src = {
        context: AL.currentCtx,
        id: AL.newId(),
        type: 0x1030 /* AL_UNDETERMINED */,
        state: 0x1011 /* AL_INITIAL */,
        bufQueue: [AL.buffers[0]],
        audioQueue: [],
        looping: false,
        pitch: 1.0,
        dopplerShift: 1.0,
        gain: gain,
        minGain: 0.0,
        maxGain: 1.0,
        panner: null,
        bufsProcessed: 0,
        bufStartTime: Number.NEGATIVE_INFINITY,
        bufOffset: 0.0,
        relative: false,
        refDistance: 1.0,
        maxDistance: 3.40282e38 /* FLT_MAX */,
        rolloffFactor: 1.0,
        position: [0.0, 0.0, 0.0],
        velocity: [0.0, 0.0, 0.0],
        direction: [0.0, 0.0, 0.0],
        coneOuterGain: 0.0,
        coneInnerAngle: 360.0,
        coneOuterAngle: 360.0,
        distanceModel: 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */,
        spatialize: 2 /* AL_AUTO_SOFT */,

        get playbackRate() {
          return this.pitch * this.dopplerShift;
        }
      };
      AL.currentCtx.sources[src.id] = src;
      {{{ makeSetValue('pSourceIds', 'i*4', 'src.id', 'i32') }}};
    }
  },

  alDeleteSources__deps: ['alSourcei'],
  alDeleteSources__proxy: 'sync',
  alDeleteSources__sig: 'vii',
  alDeleteSources: function(count, pSourceIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alDeleteSources() called without a valid context');
#endif
      return;
    }

    for (var i = 0; i < count; ++i) {
      var srcId = {{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}};
      if (!AL.currentCtx.sources[srcId]) {
#if OPENAL_DEBUG
        console.error('alDeleteSources() called with an invalid source');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      var srcId = {{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}};
      AL.setSourceState(AL.currentCtx.sources[srcId], 0x1014 /* AL_STOPPED */);
      _alSourcei(srcId, 0x1009 /* AL_BUFFER */, 0);
      delete AL.currentCtx.sources[srcId];
      AL.freeIds.push(srcId);
    }
  },

  // -------------------------------------------------------
  // --- AL Context State
  // -------------------------------------------------------

  alGetError__proxy: 'sync',
  alGetError__sig: 'i',
  alGetError: function() {
    if (!AL.currentCtx) {
      return 0xA004 /* AL_INVALID_OPERATION */;
    } else {
      // Reset error on get.
      var err = AL.currentCtx.err;
      AL.currentCtx.err = 0 /* AL_NO_ERROR */;
      return err;
    }
  },

  alIsExtensionPresent__proxy: 'sync',
  alIsExtensionPresent__sig: 'ii',
  alIsExtensionPresent: function(pExtName) {
    name = Pointer_stringify(pExtName);

    return AL.AL_EXTENSIONS[name] ? 1 : 0;
  },

  alGetProcAddress__deps: ['emscripten_GetAlProcAddress'],
  alGetProcAddress__proxy: 'sync',
  alGetProcAddress__sig: 'vi',
  alGetProcAddress: function(pProcName) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alGetProcAddress() called without a valid context');
#endif
      return;
    }
    if (!pProcName) {
#if OPENAL_DEBUG
      console.error('alcGetProcAddress() called with null name pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return 0; /* ALC_NONE */
    }
    return _emscripten_GetAlProcAddress(pProcName);
  },

  alGetEnumValue__proxy: 'sync',
  alGetEnumValue__sig: 'ii',
  alGetEnumValue: function(pEnumName) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alGetEnumValue() called without a valid context');
#endif
      return 0;
    }

    if (!pEnumName) {
#if OPENAL_DEBUG
      console.error('alGetEnumValue() called with null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return 0 /* AL_NONE */;
    }
    name = Pointer_stringify(pEnumName);

    switch(name) {
    // Spec doesn't clearly state that alGetEnumValue() is required to
    // support _only_ extension tokens.
    // We should probably follow OpenAL-Soft's example and support all
    // of the names we know.
    // See http://repo.or.cz/openal-soft.git/blob/HEAD:/Alc/ALc.c
    case 'AL_BITS': return 0x2002;
    case 'AL_BUFFER': return 0x1009;
    case 'AL_BUFFERS_PROCESSED': return 0x1016;
    case 'AL_BUFFERS_QUEUED': return 0x1015;
    case 'AL_BYTE_OFFSET': return 0x1026;
    case 'AL_CHANNELS': return 0x2003;
    case 'AL_CONE_INNER_ANGLE': return 0x1001;
    case 'AL_CONE_OUTER_ANGLE': return 0x1002;
    case 'AL_CONE_OUTER_GAIN': return 0x1022;
    case 'AL_DIRECTION': return 0x1005;
    case 'AL_DISTANCE_MODEL': return 0xD000;
    case 'AL_DOPPLER_FACTOR': return 0xC000;
    case 'AL_DOPPLER_VELOCITY': return 0xC001;
    case 'AL_EXPONENT_DISTANCE': return 0xD005;
    case 'AL_EXPONENT_DISTANCE_CLAMPED': return 0xD006;
    case 'AL_EXTENSIONS': return 0xB004;
    case 'AL_FORMAT_MONO16': return 0x1101;
    case 'AL_FORMAT_MONO8': return 0x1100;
    case 'AL_FORMAT_STEREO16': return 0x1103;
    case 'AL_FORMAT_STEREO8': return 0x1102;
    case 'AL_FREQUENCY': return 0x2001;
    case 'AL_GAIN': return 0x100A;
    case 'AL_INITIAL': return 0x1011;
    case 'AL_INVALID': return -1;
    case 'AL_ILLEGAL_ENUM': // fallthrough
    case 'AL_INVALID_ENUM': return 0xA002;
    case 'AL_INVALID_NAME': return 0xA001;
    case 'AL_ILLEGAL_COMMAND': // fallthrough
    case 'AL_INVALID_OPERATION': return 0xA004;
    case 'AL_INVALID_VALUE': return 0xA003;
    case 'AL_INVERSE_DISTANCE': return 0xD001;
    case 'AL_INVERSE_DISTANCE_CLAMPED': return 0xD002;
    case 'AL_LINEAR_DISTANCE': return 0xD003;
    case 'AL_LINEAR_DISTANCE_CLAMPED': return 0xD004;
    case 'AL_LOOPING': return 0x1007;
    case 'AL_MAX_DISTANCE': return 0x1023;
    case 'AL_MAX_GAIN': return 0x100E;
    case 'AL_MIN_GAIN': return 0x100D;
    case 'AL_NONE': return 0;
    case 'AL_NO_ERROR': return 0;
    case 'AL_ORIENTATION': return 0x100F;
    case 'AL_OUT_OF_MEMORY': return 0xA005;
    case 'AL_PAUSED': return 0x1013;
    case 'AL_PENDING': return 0x2011;
    case 'AL_PITCH': return 0x1003;
    case 'AL_PLAYING': return 0x1012;
    case 'AL_POSITION': return 0x1004;
    case 'AL_PROCESSED': return 0x2012;
    case 'AL_REFERENCE_DISTANCE': return 0x1020;
    case 'AL_RENDERER': return 0xB003;
    case 'AL_ROLLOFF_FACTOR': return 0x1021;
    case 'AL_SAMPLE_OFFSET': return 0x1025;
    case 'AL_SEC_OFFSET': return 0x1024;
    case 'AL_SIZE': return 0x2004;
    case 'AL_SOURCE_RELATIVE': return 0x202;
    case 'AL_SOURCE_STATE': return 0x1010;
    case 'AL_SOURCE_TYPE': return 0x1027;
    case 'AL_SPEED_OF_SOUND': return 0xC003;
    case 'AL_STATIC': return 0x1028;
    case 'AL_STOPPED': return 0x1014;
    case 'AL_STREAMING': return 0x1029;
    case 'AL_UNDETERMINED': return 0x1030;
    case 'AL_UNUSED': return 0x2010;
    case 'AL_VELOCITY': return 0x1006;
    case 'AL_VENDOR': return 0xB001;
    case 'AL_VERSION': return 0xB002;

    /* Extensions */
    case 'AL_AUTO_SOFT': return 0x0002;
    case 'AL_SOURCE_DISTANCE_MODEL': return 0x200;
    case 'AL_SOURCE_SPATIALIZE_SOFT': return 0x1214;
    case 'AL_LOOP_POINTS_SOFT': return 0x2015;
    case 'AL_BYTE_LENGTH_SOFT': return 0x2009;
    case 'AL_SAMPLE_LENGTH_SOFT': return 0x200A;
    case 'AL_SEC_LENGTH_SOFT': return 0x200B;
    case 'AL_FORMAT_MONO_FLOAT32': return 0x10010;
    case 'AL_FORMAT_STEREO_FLOAT32': return 0x10011;

    default:
#if OPENAL_DEBUG
      console.error('No value for `' + name + '` is known by alGetEnumValue()');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return 0;
    }
  },

  alGetString__proxy: 'sync',
  alGetString__sig: 'ii',
  alGetString: function(param) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alGetString() called without a valid context');
#endif
      return 0;
    }

    if (AL.stringCache[param]) {
      return AL.stringCache[param];
    }

    var ret;
    switch (param) {
    case 0 /* AL_NO_ERROR */:
      ret = 'No Error';
      break;
    case 0xA001 /* AL_INVALID_NAME */:
      ret = 'Invalid Name';
      break;
    case 0xA002 /* AL_INVALID_ENUM */:
      ret = 'Invalid Enum';
      break;
    case 0xA003 /* AL_INVALID_VALUE */:
      ret = 'Invalid Value';
      break;
    case 0xA004 /* AL_INVALID_OPERATION */:
      ret = 'Invalid Operation';
      break;
    case 0xA005 /* AL_OUT_OF_MEMORY */:
      ret = 'Out of Memory';
      break;
    case 0xB001 /* AL_VENDOR */:
      ret = 'Emscripten';
      break;
    case 0xB002 /* AL_VERSION */:
      ret = '1.1';
      break;
    case 0xB003 /* AL_RENDERER */:
      ret = 'WebAudio';
      break;
    case 0xB004 /* AL_EXTENSIONS */:
      ret = '';
      for (ext in AL.AL_EXTENSIONS) {
        ret = ret.concat(ext);
        ret = ret.concat(' ');
      }
      ret = ret.trim();
      break;
    default:
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return 0;
    }

    ret = allocate(intArrayFromString(ret), 'i8', ALLOC_NORMAL);
    AL.stringCache[param] = ret;
    return ret;
  },

  alEnable__proxy: 'sync',
  alEnable__sig: 'vi',
  alEnable: function(param) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alEnable() called without a valid context');
#endif
      return;
    }
    switch (param) {
    case 'AL_SOURCE_DISTANCE_MODEL':
      AL.currentCtx.sourceDistanceModel = true;
      AL.updateContextGlobal(AL.currentCtx);
      break;
    default:
#if OPENAL_DEBUG
      console.error('alEnable() with param 0x' + param.toString(16) + ' not implemented yet');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alDisable__proxy: 'sync',
  alDisable__sig: 'vi',
  alDisable: function(param) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alDisable() called without a valid context');
#endif
      return;
    }
    switch (pname) {
    case 'AL_SOURCE_DISTANCE_MODEL':
      AL.currentCtx.sourceDistanceModel = false;
      AL.updateContextGlobal(AL.currentCtx);
      break;
    default:
#if OPENAL_DEBUG
      console.error('alDisable() with param 0x' + param.toString(16) + ' not implemented yet');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alIsEnabled__proxy: 'sync',
  alIsEnabled__sig: 'ii',
  alIsEnabled: function(param) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alIsEnabled() called without a valid context');
#endif
      return 0;
    }
    switch (pname) {
    case 'AL_SOURCE_DISTANCE_MODEL':
      return AL.currentCtx.sourceDistanceModel ? 0 /* AL_FALSE */ : 1 /* AL_TRUE */;
    default:
#if OPENAL_DEBUG
      console.error('alIsEnabled() with param 0x' + param.toString(16) + ' not implemented yet');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return 0;
    }
  },

  alGetDouble__proxy: 'sync',
  alGetDouble__sig: 'di',
  alGetDouble: function(param) {
    var val = AL.getGlobalParam('alGetDouble', param);
    if (val === null) {
      return 0.0;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      return val;
    default:
#if OPENAL_DEBUG
      console.error('alGetDouble(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return 0.0;
    }
  },

  alGetDoublev__proxy: 'sync',
  alGetDoublev__sig: 'vii',
  alGetDoublev: function(param, pValues) {
    var val = AL.getGlobalParam('alGetDoublev', param);
    // Silently ignore null destinations, as per the spec for global state functions
    if (val === null || !pValues) {
      return;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      {{{ makeSetValue('pValues', '0', 'val', 'double') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetDoublev(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetFloat__proxy: 'sync',
  alGetFloat__sig: 'fi',
  alGetFloat: function(param) {
    var val = AL.getGlobalParam('alGetFloat', param);
    if (val === null) {
      return 0.0;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      return val;
    default:
#if OPENAL_DEBUG
      console.error('alGetFloat(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      return 0.0;
    }
  },

  alGetFloatv__proxy: 'sync',
  alGetFloatv__sig: 'vii',
  alGetFloatv: function(param, pValues) {
    var val = AL.getGlobalParam('alGetFloatv', param);
    // Silently ignore null destinations, as per the spec for global state functions
    if (val === null || !pValues) {
      return;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      {{{ makeSetValue('pValues', '0', 'val', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetFloatv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetInteger__proxy: 'sync',
  alGetInteger__sig: 'ii',
  alGetInteger: function(param) {
    var val = AL.getGlobalParam('alGetInteger', param);
    if (val === null) {
      return 0;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      return val;
    default:
#if OPENAL_DEBUG
      console.error('alGetInteger(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return 0;
    }
  },

  alGetIntegerv__proxy: 'sync',
  alGetIntegerv__sig: 'vii',
  alGetIntegerv: function(param, pValues) {
    var val = AL.getGlobalParam('alGetIntegerv', param);
    // Silently ignore null destinations, as per the spec for global state functions
    if (val === null || !pValues) {
      return;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      {{{ makeSetValue('pValues', '0', 'val', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetIntegerv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetBoolean__proxy: 'sync',
  alGetBoolean__sig: 'ii',
  alGetBoolean: function(param) {
    var val = AL.getGlobalParam('alGetBoolean', param);
    if (val === null) {
      return 0 /* AL_FALSE */;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      return val !== 0 ? 1 /* AL_TRUE */ : 0 /* AL_FALSE */;
    default:
#if OPENAL_DEBUG
      console.error('alGetBoolean(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return 0 /* AL_FALSE */;
    }
  },

  alGetBooleanv__proxy: 'sync',
  alGetBooleanv__sig: 'vii',
  alGetBooleanv: function(param, pValues) {
    var val = AL.getGlobalParam('alGetBooleanv', param);
    // Silently ignore null destinations, as per the spec for global state functions
    if (val === null || !pValues) {
      return;
    }

    switch (param) {
    case 0xC000 /* AL_DOPPLER_FACTOR */:
    case 0xC003 /* AL_SPEED_OF_SOUND */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      {{{ makeSetValue('pValues', '0', 'val', 'i8') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetBooleanv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alDistanceModel__proxy: 'sync',
  alDistanceModel__sig: 'vi',
  alDistanceModel: function(model) {
    AL.setGlobalParam('alDistanceModel', 0xD000 /* AL_DISTANCE_MODEL */, model);
  },

  alSpeedOfSound__proxy: 'sync',
  alSpeedOfSound__sig: 'vi',
  alSpeedOfSound: function(value) {
    AL.setGlobalParam('alSpeedOfSound', 0xC003 /* AL_SPEED_OF_SOUND */, value);
  },

  alDopplerFactor__proxy: 'sync',
  alDopplerFactor__sig: 'vi',
  alDopplerFactor: function(value) {
    AL.setGlobalParam('alDopplerFactor', 0xC000 /* AL_DOPPLER_FACTOR */, value);
  },

  // http://openal.996291.n3.nabble.com/alSpeedOfSound-or-alDopperVelocity-tp1960.html
  // alDopplerVelocity() sets a multiplier for the speed of sound.
  // It's deprecated since it's equivalent to directly calling
  // alSpeedOfSound() with an appropriately premultiplied value.
  alDopplerVelocity__proxy: 'sync',
  alDopplerVelocity__sig: 'vi',
  alDopplerVelocity: function(value) {
    warnOnce('alDopplerVelocity() is deprecated, and only kept for compatibility with OpenAL 1.0. Use alSpeedOfSound() instead.');
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alDopplerVelocity() called without a valid context');
#endif
      return;
    }
    if (value <= 0) { // Negative or zero values are disallowed
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }
  },

  // -------------------------------------------------------
  // -- AL Listener State
  // -------------------------------------------------------

  alGetListenerf__proxy: 'sync',
  alGetListenerf__sig: 'vii',
  alGetListenerf: function(param, pValue) {
    var val = AL.getListenerParam('alGetListenerf', param);
    if (val === null) {
      return;
    }
    if (!pValue) {
#if OPENAL_DEBUG
      console.error('alGetListenerf() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x100A /* AL_GAIN */:
      {{{ makeSetValue('pValue', '0', 'val', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetListenerf(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetListener3f__proxy: 'sync',
  alGetListener3f__sig: 'viiii',
  alGetListener3f: function(param, pValue0, pValue1, pValue2) {
    var val = AL.getListenerParam('alGetListener3f', param);
    if (val === null) {
      return;
    }
    if (!pValue0 || !pValue1 || !pValue2) {
#if OPENAL_DEBUG
      console.error('alGetListener3f() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetListener3f(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetListenerfv__proxy: 'sync',
  alGetListenerfv__sig: 'vii',
  alGetListenerfv: function(param, pValues) {
    var val = AL.getListenerParam('alGetListenerfv', param);
    if (val === null) {
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alGetListenerfv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'float') }}};
      break;
    case 0x100F /* AL_ORIENTATION */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'float') }}};
      {{{ makeSetValue('pValues', '12', 'val[3]', 'float') }}};
      {{{ makeSetValue('pValues', '16', 'val[4]', 'float') }}};
      {{{ makeSetValue('pValues', '20', 'val[5]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetListenerfv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetListeneri__proxy: 'sync',
  alGetListeneri__sig: 'vii',
  alGetListeneri: function(param, pValue) {
    var val = AL.getListenerParam('alGetListeneri', param);
    if (val === null) {
      return;
    }
    if (!pValue) {
#if OPENAL_DEBUG
      console.error('alGetListeneri() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

#if OPENAL_DEBUG
    console.error('alGetListeneri(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
    AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
  },

  alGetListener3i__proxy: 'sync',
  alGetListener3i__sig: 'viiii',
  alGetListener3i: function(param, pValue0, pValue1, pValue2) {
    var val = AL.getListenerParam('alGetListener3i', param);
    if (val === null) {
      return;
    }
    if (!pValue0 || !pValue1 || !pValue2) {
#if OPENAL_DEBUG
      console.error('alGetListener3i() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetListener3i(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetListeneriv__proxy: 'sync',
  alGetListeneriv__sig: 'vii',
  alGetListeneriv: function(param, pValues) {
    var val = AL.getListenerParam('alGetListeneriv', param);
    if (val === null) {
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alGetListeneriv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'i32') }}};
      break;
    case 0x100F /* AL_ORIENTATION */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'i32') }}};
      {{{ makeSetValue('pValues', '12', 'val[3]', 'i32') }}};
      {{{ makeSetValue('pValues', '16', 'val[4]', 'i32') }}};
      {{{ makeSetValue('pValues', '20', 'val[5]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetListeneriv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alListenerf__proxy: 'sync',
  alListenerf__sig: 'vif',
  alListenerf: function(param, value) {
    switch (param) {
    case 0x100A /* AL_GAIN */:
      AL.setListenerParam('alListenerf', param, value);
      break;
    default:
      AL.setListenerParam('alListenerf', param, null);
      break;
    }
  },

  alListener3f__proxy: 'sync',
  alListener3f__sig: 'vifff',
  alListener3f: function(param, value0, value1, value2) {
    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setListenerParam('alListener3f', param, AL.paramArray);
      break;
    default:
      AL.setListenerParam('alListener3f', param, null);
      break;
    }
  },

  alListenerfv__proxy: 'sync',
  alListenerfv__sig: 'vii',
  alListenerfv: function(param, pValues) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alListenerfv() called without a valid context');
#endif
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alListenerfv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'float') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'float') }}};
      AL.setListenerParam('alListenerfv', param, AL.paramArray);
      break;
    case 0x100F /* AL_ORIENTATION */:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'float') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'float') }}};
      AL.paramArray[3] = {{{ makeGetValue('pValues', '12', 'float') }}};
      AL.paramArray[4] = {{{ makeGetValue('pValues', '16', 'float') }}};
      AL.paramArray[5] = {{{ makeGetValue('pValues', '20', 'float') }}};
      AL.setListenerParam('alListenerfv', param, AL.paramArray);
      break;
    default:
      AL.setListenerParam('alListenerfv', param, null);
      break;
    }
  },

  alListeneri__proxy: 'sync',
  alListeneri__sig: 'vii',
  alListeneri: function(param, value) {
    AL.setListenerParam('alListeneri', param, null);
  },

  alListener3i__proxy: 'sync',
  alListener3i__sig: 'viiii',
  alListener3i: function(param, value0, value1, value2) {
    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setListenerParam('alListener3i', param, AL.paramArray);
      break;
    default:
      AL.setListenerParam('alListener3i', param, null);
      break;
    }
  },

  alListeneriv__proxy: 'sync',
  alListeneriv__sig: 'vii',
  alListeneriv: function(param, pValues) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alListeneriv() called without a valid context');
#endif
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alListeneriv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'i32') }}};
      AL.setListenerParam('alListeneriv', param, AL.paramArray);
      break;
    case 0x100F /* AL_ORIENTATION */:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'i32') }}};
      AL.paramArray[3] = {{{ makeGetValue('pValues', '12', 'i32') }}};
      AL.paramArray[4] = {{{ makeGetValue('pValues', '16', 'i32') }}};
      AL.paramArray[5] = {{{ makeGetValue('pValues', '20', 'i32') }}};
      AL.setListenerParam('alListeneriv', param, AL.paramArray);
      break;
    default:
      AL.setListenerParam('alListeneriv', param, null);
      break;
    }
  },

  // -------------------------------------------------------
  // -- AL Buffer State
  // -------------------------------------------------------

  alIsBuffer__proxy: 'sync',
  alIsBuffer__sig: 'ii',
  alIsBuffer: function(bufferId) {
    if (!AL.currentCtx) {
      return false;
    }
    if (bufferId > AL.buffers.length) {
      return false;
    }

    if (!AL.buffers[bufferId]) {
      return false;
    } else {
      return true;
    }
  },

  alBufferData__proxy: 'sync',
  alBufferData__sig: 'viiiii',
  alBufferData: function(bufferId, format, pData, size, freq) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alBufferData() called without a valid context');
#endif
      return;
    }
    var buf = AL.buffers[bufferId];
    if (!buf) {
#if OPENAL_DEBUG
      console.error('alBufferData() called with an invalid buffer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }
    if (freq <= 0) {
#if OPENAL_DEBUG
      console.error('alBufferData() called with an invalid frequency');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    var audioBuf = null;
    try {
      switch (format) {
      case 0x1100 /* AL_FORMAT_MONO8 */:
        if (size > 0) {
          audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size, freq);
          var channel0 = audioBuf.getChannelData(0);
          for (var i = 0; i < size; ++i) {
            channel0[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
          }
        }
        buf.bytesPerSample = 1;
        buf.channels = 1;
        buf.length = size;
        break;
      case 0x1101 /* AL_FORMAT_MONO16 */:
        if (size > 0) {
          audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size >> 1, freq);
          var channel0 = audioBuf.getChannelData(0);
          pData >>= 1;
          for (var i = 0; i < size >> 1; ++i) {
            channel0[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
          }
        }
        buf.bytesPerSample = 2;
        buf.channels = 1;
        buf.length = size >> 1;
        break;
      case 0x1102 /* AL_FORMAT_STEREO8 */:
        if (size > 0) {
          audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 1, freq);
          var channel0 = audioBuf.getChannelData(0);
          var channel1 = audioBuf.getChannelData(1);
          for (var i = 0; i < size >> 1; ++i) {
            channel0[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
            channel1[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
          }
        }
        buf.bytesPerSample = 1;
        buf.channels = 2;
        buf.length = size >> 1;
        break;
      case 0x1103 /* AL_FORMAT_STEREO16 */:
        if (size > 0) {
          audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 2, freq);
          var channel0 = audioBuf.getChannelData(0);
          var channel1 = audioBuf.getChannelData(1);
          pData >>= 1;
          for (var i = 0; i < size >> 2; ++i) {
            channel0[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
            channel1[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
          }
        }
        buf.bytesPerSample = 2;
        buf.channels = 2;
        buf.length = size >> 2;
        break;
      case 0x10010 /* AL_FORMAT_MONO_FLOAT32 */:
        if (size > 0) {
          audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size >> 2, freq);
          var channel0 = audioBuf.getChannelData(0);
          pData >>= 2;
          for (var i = 0; i < size >> 2; ++i) {
            channel0[i] = HEAPF32[pData++];
          }
        }
        buf.bytesPerSample = 4;
        buf.channels = 1;
        buf.length = size >> 2;
        break;
      case 0x10011 /* AL_FORMAT_STEREO_FLOAT32 */:
        if (size > 0) {
          audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 3, freq);
          var channel0 = audioBuf.getChannelData(0);
          var channel1 = audioBuf.getChannelData(1);
          pData >>= 2;
          for (var i = 0; i < size >> 3; ++i) {
            channel0[i] = HEAPF32[pData++];
            channel1[i] = HEAPF32[pData++];
          }
        }
        buf.bytesPerSample = 4;
        buf.channels = 2;
        buf.length = size >> 3;
        break;
      default:
#if OPENAL_DEBUG
        console.error('alBufferData() called with invalid format ' + format);
#endif
        AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
        return;
      }
      buf.frequency = freq;
      buf.audioBuf = audioBuf;
    } catch (e) {
#if OPENAL_DEBUG
      console.error('alBufferData() upload failed with an exception ' + e);
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }
  },

  alGetBufferf__proxy: 'sync',
  alGetBufferf__sig: 'viii',
  alGetBufferf: function(bufferId, param, pValue) {
    var val = AL.getBufferParam('alGetBufferf', bufferId, param);
    if (val === null) {
      return;
    }
    if (!pValue) {
#if OPENAL_DEBUG
      console.error('alGetBufferf() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

#if OPENAL_DEBUG
    console.error('alGetBufferf(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
    AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
  },

  alGetBuffer3f__proxy: 'sync',
  alGetBuffer3f__sig: 'viiiii',
  alGetBuffer3f: function(bufferId, param, pValue0, pValue1, pValue2) {
    var val = AL.getBufferParam('alGetBuffer3f', bufferId, param);
    if (val === null) {
      return;
    }
    if (!pValue0 || !pValue1 || !pValue2) {
#if OPENAL_DEBUG
      console.error('alGetBuffer3f() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

#if OPENAL_DEBUG
    console.error('alGetBuffer3f(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
    AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
  },

  alGetBufferfv__proxy: 'sync',
  alGetBufferfv__sig: 'viii',
  alGetBufferfv: function(bufferId, param, pValues) {
    var val = AL.getBufferParam('alGetBufferfv', bufferId, param);
    if (val === null) {
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alGetBufferfv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

#if OPENAL_DEBUG
    console.error('alGetBufferfv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
    AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
  },

  alGetBufferi__proxy: 'sync',
  alGetBufferi__sig: 'viii',
  alGetBufferi: function(bufferId, param, pValue) {
    var val = AL.getBufferParam('alGetBufferi', bufferId, param);
    if (val === null) {
      return;
    }
    if (!pValue) {
#if OPENAL_DEBUG
      console.error('alGetBufferi() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x2001 /* AL_FREQUENCY */:
    case 0x2002 /* AL_BITS */:
    case 0x2003 /* AL_CHANNELS */:
    case 0x2004 /* AL_SIZE */:
      {{{ makeSetValue('pValue', '0', 'val', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetBufferi(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetBuffer3i__proxy: 'sync',
  alGetBuffer3i__sig: 'viiiii',
  alGetBuffer3i: function(bufferId, param, pValue0, pValue1, pValue2) {
    var val = AL.getBufferParam('alGetBuffer3i', bufferId, param);
    if (val === null) {
      return;
    }
    if (!pValue0 || !pValue1 || !pValue2) {
#if OPENAL_DEBUG
      console.error('alGetBuffer3i() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

#if OPENAL_DEBUG
    console.error('alGetBuffer3i(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
    AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
  },

  alGetBufferiv__proxy: 'sync',
  alGetBufferiv__sig: 'viii',
  alGetBufferiv: function(bufferId, param, pValues) {
    var val = AL.getBufferParam('alGetBufferiv', bufferId, param);
    if (val === null) {
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alGetBufferiv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x2001 /* AL_FREQUENCY */:
    case 0x2002 /* AL_BITS */:
    case 0x2003 /* AL_CHANNELS */:
    case 0x2004 /* AL_SIZE */:
      {{{ makeSetValue('pValues', '0', 'val', 'i32') }}};
      break;
    case 0x2015 /* AL_LOOP_POINTS_SOFT */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetBufferiv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  // All of the remaining alBuffer* setters and getters are only of interest
  // to extensions which need them. Core OpenAL alone defines no valid
  // property for these.

  alBufferf__proxy: 'sync',
  alBufferf__sig: 'viif',
  alBufferf: function(bufferId, param, value) {
    AL.setBufferParam('alBufferf', bufferId, param, null);
  },

  alBuffer3f__proxy: 'sync',
  alBuffer3f__sig: 'viifff',
  alBuffer3f: function(bufferId, param, value0, value1, value2) {
    AL.setBufferParam('alBuffer3f', bufferId, param, null);
  },

  alBufferfv__proxy: 'sync',
  alBufferfv__sig: 'viii',
  alBufferfv: function(bufferId, param, pValues) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alBufferfv() called without a valid context');
#endif
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alBufferfv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    AL.setBufferParam('alBufferfv', bufferId, param, null);
  },

  alBufferi__proxy: 'sync',
  alBufferi__sig: 'viii',
  alBufferi: function(bufferId, param, value) {
    AL.setBufferParam('alBufferi', bufferId, param, null);
  },

  alBuffer3i__proxy: 'sync',
  alBuffer3i__sig: 'viiiii',
  alBuffer3i: function(bufferId, param, value0, value1, value2) {
    AL.setBufferParam('alBuffer3i', bufferId, param, null);
  },

  alBufferiv__proxy: 'sync',
  alBufferiv__sig: 'viii',
  alBufferiv: function(bufferId, param, pValues) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alBufferiv() called without a valid context');
#endif
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alBufferiv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x2015 /* AL_LOOP_POINTS_SOFT */:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.setBufferParam('alBufferiv', bufferId, param, AL.paramArray);
      break;
    default:
      AL.setBufferParam('alBufferiv', bufferId, param, null);
      break;
    }
  },

  // -------------------------------------------------------
  // -- AL Source State
  // -------------------------------------------------------

  alIsSource__proxy: 'sync',
  alIsSource__sig: 'ii',
  alIsSource: function(sourceId) {
    if (!AL.currentCtx) {
      return false;
    }

    if (!AL.currentCtx.sources[sourceId]) {
      return false;
    } else {
      return true;
    }
  },

  alSourceQueueBuffers__proxy: 'sync',
  alSourceQueueBuffers__sig: 'viii',
  alSourceQueueBuffers: function(sourceId, count, pBufferIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourceQueueBuffers() called without a valid context');
#endif
      return;
    }
    var src = AL.currentCtx.sources[sourceId];
    if (!src) {
#if OPENAL_DEBUG
      console.error('alSourceQueueBuffers() called with an invalid source');
#endif
      AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    if (src.type === 0x1028 /* AL_STATIC */) {
#if OPENAL_DEBUG
      console.error('alSourceQueueBuffers() called while a static buffer is bound');
#endif
      AL.currentCtx.err = 0xA004 /* AL_INVALID_OPERATION */;
      return;
    }

    if (count === 0) {
      return;
    }

    // Find the first non-zero buffer in the queue to determine the proper format
    var templateBuf = AL.buffers[0];
    for (var i = 0; i < src.bufQueue.length; i++) {
      if (src.bufQueue[i].id !== 0) {
        templateBuf = src.bufQueue[i];
        break;
      }
    }

    for (var i = 0; i < count; ++i) {
      var bufId = {{{ makeGetValue('pBufferIds', 'i*4', 'i32') }}};
      var buf = AL.buffers[bufId];
      if (!buf) {
#if OPENAL_DEBUG
        console.error('alSourceQueueBuffers() called with an invalid buffer');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }

      // Check that the added buffer has the correct format. If the template is the zero buffer, any format is valid.
      if (templateBuf.id !== 0 && (
        buf.frequency !== templateBuf.frequency
        || buf.bytesPerSample !== templateBuf.bytesPerSample
        || buf.channels !== templateBuf.channels)
      ) {
#if OPENAL_DEBUG
        console.error('alSourceQueueBuffers() called with a buffer of different format');
#endif
        AL.currentCtx.err = 0xA004 /* AL_INVALID_OPERATION */;
      }
    }

    // If the only buffer in the queue is the zero buffer, clear the queue before we add anything.
    if (src.bufQueue.length === 1 && src.bufQueue[0].id === 0) {
      src.bufQueue.length = 0;
    }

    src.type = 0x1029 /* AL_STREAMING */;
    for (var i = 0; i < count; ++i) {
      var bufId = {{{ makeGetValue('pBufferIds', 'i*4', 'i32') }}};
      var buf = AL.buffers[bufId];
      buf.refCount++;
      src.bufQueue.push(buf);
    }

    // if the source is looping, cancel the schedule so we can reschedule the loop order
    if (src.looping) {
      AL.cancelPendingSourceAudio(src);
    }

    AL.initSourcePanner(src);
    AL.scheduleSourceAudio(src);
  },

  alSourceUnqueueBuffers__proxy: 'sync',
  alSourceUnqueueBuffers__sig: 'viii',
  alSourceUnqueueBuffers: function(sourceId, count, pBufferIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourceUnqueueBuffers() called without a valid context');
#endif
      return;
    }
    var src = AL.currentCtx.sources[sourceId];
    if (!src) {
#if OPENAL_DEBUG
      console.error('alSourceUnqueueBuffers() called with an invalid source');
#endif
      AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    if (count > (src.bufQueue.length === 1 && src.bufQueue[0].id === 0 ? 0 : src.bufsProcessed)) {
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    if (count === 0) {
      return;
    }

    for (var i = 0; i < count; i++) {
      var buf = src.bufQueue.shift();
      buf.refCount--;
      // Write the buffers index out to the return list.
      {{{ makeSetValue('pBufferIds', 'i*4', 'buf.id', 'i32') }}};
      src.bufsProcessed--;
    }

    /// If the queue is empty, put the zero buffer back in
    if (src.bufQueue.length === 0) {
      src.bufQueue.push(AL.buffers[0]);
    }

    AL.initSourcePanner(src);
    AL.scheduleSourceAudio(src);
  },

  alSourcePlay__proxy: 'sync',
  alSourcePlay__sig: 'vi',
  alSourcePlay: function(sourceId) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourcePlay() called without a valid context');
#endif
      return;
    }
    var src = AL.currentCtx.sources[sourceId];
    if (!src) {
#if OPENAL_DEBUG
      console.error('alSourcePlay() called with an invalid source');
#endif
      AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1012 /* AL_PLAYING */);
  },

  alSourcePlayv__proxy: 'sync',
  alSourcePlayv__sig: 'vii',
  alSourcePlayv: function(count, pSourceIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourcePlayv() called without a valid context');
#endif
      return;
    }
    if (!pSourceIds) {
#if OPENAL_DEBUG
      console.error('alSourcePlayv() called with null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
    }
    for (var i = 0; i < count; ++i) {
      if (!AL.currentCtx.sources[{{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}]) {
#if OPENAL_DEBUG
        console.error('alSourcePlayv() called with an invalid source');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      AL.setSourceState({{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}, 0x1012 /* AL_PLAYING */);
    }
  },

  alSourceStop__proxy: 'sync',
  alSourceStop__sig: 'vi',
  alSourceStop: function(sourceId) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourceStop() called without a valid context');
#endif
      return;
    }
    var src = AL.currentCtx.sources[sourceId];
    if (!src) {
#if OPENAL_DEBUG
      console.error('alSourceStop() called with an invalid source');
#endif
      AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
  },

  alSourceStopv__proxy: 'sync',
  alSourceStopv__sig: 'vii',
  alSourceStopv: function(count, pSourceIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourceStopv() called without a valid context');
#endif
      return;
    }
    if (!pSourceIds) {
#if OPENAL_DEBUG
      console.error('alSourceStopv() called with null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
    }
    for (var i = 0; i < count; ++i) {
      if (!AL.currentCtx.sources[{{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}]) {
#if OPENAL_DEBUG
        console.error('alSourceStopv() called with an invalid source');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      AL.setSourceState({{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}, 0x1014 /* AL_STOPPED */);
    }
  },

  alSourceRewind__proxy: 'sync',
  alSourceRewind__sig: 'vi',
  alSourceRewind: function(sourceId) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourceRewind() called without a valid context');
#endif
      return;
    }
    var src = AL.currentCtx.sources[sourceId];
    if (!src) {
#if OPENAL_DEBUG
      console.error('alSourceRewind() called with an invalid source');
#endif
      AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    // Stop the source first to clear the source queue
    AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
    // Now set the state of AL_INITIAL according to the specification
    AL.setSourceState(src, 0x1011 /* AL_INITIAL */);
  },

  alSourceRewindv__proxy: 'sync',
  alSourceRewindv__sig: 'vii',
  alSourceRewindv: function(count, pSourceIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourceRewindv() called without a valid context');
#endif
      return;
    }
    if (!pSourceIds) {
#if OPENAL_DEBUG
      console.error('alSourceRewindv() called with null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
    }
    for (var i = 0; i < count; ++i) {
      if (!AL.currentCtx.sources[{{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}]) {
#if OPENAL_DEBUG
        console.error('alSourceRewindv() called with an invalid source');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      AL.setSourceState({{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}, 0x1011 /* AL_INITIAL */);
    }
  },

  alSourcePause__proxy: 'sync',
  alSourcePause__sig: 'vi',
  alSourcePause: function(sourceId) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourcePause() called without a valid context');
#endif
      return;
    }
    var src = AL.currentCtx.sources[sourceId];
    if (!src) {
#if OPENAL_DEBUG
      console.error('alSourcePause() called with an invalid source');
#endif
      AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1013 /* AL_PAUSED */);
  },

  alSourcePausev__proxy: 'sync',
  alSourcePausev__sig: 'vii',
  alSourcePausev: function(count, pSourceIds) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourcePausev() called without a valid context');
#endif
      return;
    }
    if (!pSourceIds) {
#if OPENAL_DEBUG
      console.error('alSourcePausev() called with null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
    }
    for (var i = 0; i < count; ++i) {
      if (!AL.currentCtx.sources[{{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}]) {
#if OPENAL_DEBUG
        console.error('alSourcePausev() called with an invalid source');
#endif
        AL.currentCtx.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      AL.setSourceState({{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}, 0x1013 /* AL_PAUSED */);
    }
  },

  alGetSourcef__proxy: 'sync',
  alGetSourcef__sig: 'viii',
  alGetSourcef: function(sourceId, param, pValue) {
    var val = AL.getSourceParam('alGetSourcef', sourceId, param);
    if (val === null) {
      return;
    }
    if (!pValue) {
#if OPENAL_DEBUG
      console.error('alGetSourcef() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1003 /* AL_PITCH */:
    case 0x100A /* AL_GAIN */:
    case 0x100D /* AL_MIN_GAIN */:
    case 0x100E /* AL_MAX_GAIN */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1022 /* AL_CONE_OUTER_GAIN */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      {{{ makeSetValue('pValue', '0', 'val', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetSourcef(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetSource3f__proxy: 'sync',
  alGetSource3f__sig: 'viiiii',
  alGetSource3f: function(source, param, pValue0, pValue1, pValue2) {
    var val = AL.getSourceParam('alGetSource3f', sourceId, param);
    if (val === null) {
      return;
    }
    if (!pValue0 || !pValue1 || !pValue2) {
#if OPENAL_DEBUG
      console.error('alGetSource3f() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetSource3f(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetSourcefv__proxy: 'sync',
  alGetSourcefv__sig: 'viii',
  alGetSourcefv: function(sourceId, param, pValues) {
    var val = AL.getSourceParam('alGetSourcefv', sourceId, param);
    if (val === null) {
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alGetSourcefv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1003 /* AL_PITCH */:
    case 0x100A /* AL_GAIN */:
    case 0x100D /* AL_MIN_GAIN */:
    case 0x100E /* AL_MAX_GAIN */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1022 /* AL_CONE_OUTER_GAIN */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'float') }}};
      break;
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetSourcefv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetSourcei__proxy: 'sync',
  alGetSourcei__sig: 'viii',
  alGetSourcei: function(sourceId, param, pValue) {
    var val = AL.getSourceParam('alGetSourcei', sourceId, param);
    if (val === null) {
      return;
    }
    if (!pValue) {
#if OPENAL_DEBUG
      console.error('alGetSourcei() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x202 /* AL_SOURCE_RELATIVE */:
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1007 /* AL_LOOPING */:
    case 0x1009 /* AL_BUFFER */:
    case 0x1010 /* AL_SOURCE_STATE */:
    case 0x1015 /* AL_BUFFERS_QUEUED */:
    case 0x1016 /* AL_BUFFERS_PROCESSED */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x1027 /* AL_SOURCE_TYPE */:
    case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */: 
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      {{{ makeSetValue('pValue', '0', 'val', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetSourcei(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetSource3i__proxy: 'sync',
  alGetSource3i__sig: 'viiiii',
  alGetSource3i: function(source, param, pValue0, pValue1, pValue2) {
    var val = AL.getSourceParam('alGetSource3i', sourceId, param);
    if (val === null) {
      return;
    }
    if (!pValue0 || !pValue1 || !pValue2) {
#if OPENAL_DEBUG
      console.error('alGetSource3i() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetSource3i(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alGetSourceiv__proxy: 'sync',
  alGetSourceiv__sig: 'viii',
  alGetSourceiv: function(sourceId, param, pValues) {
    var val = AL.getSourceParam('alGetSourceiv', sourceId, param);
    if (val === null) {
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alGetSourceiv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x202 /* AL_SOURCE_RELATIVE */:
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1007 /* AL_LOOPING */:
    case 0x1009 /* AL_BUFFER */:
    case 0x1010 /* AL_SOURCE_STATE */:
    case 0x1015 /* AL_BUFFERS_QUEUED */:
    case 0x1016 /* AL_BUFFERS_PROCESSED */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x1027 /* AL_SOURCE_TYPE */:
    case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */: 
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      {{{ makeSetValue('pValues', '0', 'val', 'i32') }}};
      break;
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      console.error('alGetSourceiv(): param 0x' + param.toString(16) + ' has wrong signature');
#endif
      AL.currentCtx.err = 0xA002 /* AL_INVALID_ENUM */;
      return;
    }
  },

  alSourcef__proxy: 'sync',
  alSourcef__sig: 'viif',
  alSourcef: function(sourceId, param, value) {
    switch (param) {
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1003 /* AL_PITCH */:
    case 0x100A /* AL_GAIN */:
    case 0x100D /* AL_MIN_GAIN */:
    case 0x100E /* AL_MAX_GAIN */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1022 /* AL_CONE_OUTER_GAIN */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      AL.setSourceParam('alSourcef', sourceId, param, value);
      break;
    default:
      AL.setSourceParam('alSourcef', sourceId, param, null);
      break;
    }
  },

  alSource3f__proxy: 'sync',
  alSource3f__sig: 'viifff',
  alSource3f: function(sourceId, param, value0, value1, value2) {
    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setSourceParam('alSource3f', sourceId, param, AL.paramArray);
      break;
    default:
      AL.setSourceParam('alSource3f', sourceId, param, null);
      break;
    }
  },

  alSourcefv__proxy: 'sync',
  alSourcefv__sig: 'viii',
  alSourcefv: function(sourceId, param, pValues) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourcefv() called without a valid context');
#endif
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alSourcefv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1003 /* AL_PITCH */:
    case 0x100A /* AL_GAIN */:
    case 0x100D /* AL_MIN_GAIN */:
    case 0x100E /* AL_MAX_GAIN */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1022 /* AL_CONE_OUTER_GAIN */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      var val = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.setSourceParam('alSourcefv', sourceId, param, val);
      break;
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'float') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'float') }}};
      AL.setSourceParam('alSourcefv', sourceId, param, AL.paramArray);
      break;
    default:
      AL.setSourceParam('alSourcefv', sourceId, param, null);
      break;
    }
  },

  alSourcei__proxy: 'sync',
  alSourcei__sig: 'viii',
  alSourcei: function(sourceId, param, value) {
    switch (param) {
    case 0x202 /* AL_SOURCE_RELATIVE */:
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1007 /* AL_LOOPING */:
    case 0x1009 /* AL_BUFFER */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */: 
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      AL.setSourceParam('alSourcei', sourceId, param, value);
      break;
    default:
      AL.setSourceParam('alSourcei', sourceId, param, null);
      break;
    }
  },

  alSource3i__proxy: 'sync',
  alSource3i__sig: 'viiiii',
  alSource3i: function(sourceId, param, value0, value1, value2) {
    switch (param) {
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setSourceParam('alSource3i', sourceId, param, AL.paramArray);
      break;
    default:
      AL.setSourceParam('alSource3i', sourceId, param, null);
      break;
    }
  },

  alSourceiv__proxy: 'sync',
  alSourceiv__sig: 'viii',
  alSourceiv: function(source, param, pValues) {
    if (!AL.currentCtx) {
#if OPENAL_DEBUG
      console.error('alSourceiv() called without a valid context');
#endif
      return;
    }
    if (!pValues) {
#if OPENAL_DEBUG
      console.error('alSourceiv() called with a null pointer');
#endif
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch (param) {
    case 0x202 /* AL_SOURCE_RELATIVE */:
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1007 /* AL_LOOPING */:
    case 0x1009 /* AL_BUFFER */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
    case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */: 
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case 0xD000 /* AL_DISTANCE_MODEL */:
      var val = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.setSourceParam('alSourceiv', sourceId, param, val);
      break;
    case 0x1004 /* AL_POSITION */:
    case 0x1005 /* AL_DIRECTION */:
    case 0x1006 /* AL_VELOCITY */:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'i32') }}};
      AL.setSourceParam('alSourceiv', sourceId, param, AL.paramArray);
      break;
    default:
      AL.setSourceParam('alSourceiv', sourceId, param, null);
      break;
    }
  }
};

autoAddDeps(LibraryOpenAL, '$AL');
mergeInto(LibraryManager.library, LibraryOpenAL);

