/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//'use strict';

var LibraryOpenAL = {
  ...defineInternalConsts({
    $AL_QUEUE_INTERVAL: 25,
    $AL_QUEUE_LOOKAHEAD: 100.0 / 1000.0,
    $AL_DEVICE_NAME: 'Emscripten OpenAL',
    $AL_CAPTURE_DEVICE_NAME: 'Emscripten OpenAL capture',
  }),

  // ************************************************************************
  // ** INTERNALS
  // ************************************************************************

  $AL: {
    // ------------------------------------------------------
    // -- Constants
    // ------------------------------------------------------

    ALC_EXTENSIONS: {
      // TODO: 'ALC_EXT_EFX': true,
      'ALC_EXT_capture': true,
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

    _alcErr: cDefs.ALC_NO_ERROR,
    get alcErr() {
      return this._alcErr;
    },
    set alcErr(val) {
      // Errors should not be overwritten by later errors until they are cleared by a query.
      if (this._alcErr === ALC_NO_ERROR || val === ALC_NO_ERROR) {
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
    contexts: new Map(),
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
    newId: () => AL.freeIds.length > 0 ? AL.freeIds.pop() : AL._nextId++,
    freeIds: [],

    // ------------------------------------------------------
    // -- Mixing Logic
    // ------------------------------------------------------

    scheduleContextAudio: (ctx) => {
      // If we are animating using the requestAnimationFrame method, then the main loop does not run when in the background.
      // To give a perfect glitch-free audio stop when switching from foreground to background, we need to avoid updating
      // audio altogether when in the background, so detect that case and kill audio buffer streaming if so.
      if (MainLoop.timingMode === EM_TIMING_RAF && document['visibilityState'] != 'visible') {
        return;
      }

      for (var src of ctx.sources.values()) {
        AL.scheduleSourceAudio(src);
      }
    },

    // This function is the core scheduler that queues web-audio buffers for output.
    // src.bufQueue represents the abstract OpenAL buffer queue, which is traversed to schedule
    // corresponding web-audio buffers. These buffers are stored in src.audioQueue, which
    // represents the queue of buffers scheduled for physical playback. These two queues are
    // distinct because of the differing semantics of OpenAL and web audio. Some changes
    // to OpenAL parameters, such as pitch, may require the web audio queue to be flushed and rescheduled.
    scheduleSourceAudio: (src, lookahead = undefined) => {
      // See comment on scheduleContextAudio above.
      if (MainLoop.timingMode === EM_TIMING_RAF && document['visibilityState'] != 'visible') {
        return;
      }
      if (src.state !== AL_PLAYING) {
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
        lookahead = AL_QUEUE_LOOKAHEAD;
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
        if (!buf.length) {
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
          if (src.type === AL_STATIC && src.looping) {
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

          if (typeof audioSrc.start !== 'undefined') {
            // Sample the current time as late as possible to mitigate drift
            startTime = Math.max(startTime, src.context.audioCtx.currentTime);
            audioSrc.start(startTime, startOffset);
          } else if (typeof audioSrc.noteOn !== 'undefined') {
            startTime = Math.max(startTime, src.context.audioCtx.currentTime);
            audioSrc.noteOn(startTime);
#if OPENAL_DEBUG
            if (startOffset > 0.0) {
              warnOnce(`The current browser does not support AudioBufferSourceNode.start(when, offset); method, so cannot play back audio with an offset ${startOffset} secs! Audio glitches will occur!`);
            }
#endif
          }
#if OPENAL_DEBUG
          else {
            warnOnce('Unable to start AudioBufferSourceNode playback! Not supported by the browser?');
          }

          dbg(`scheduleSourceAudio() queuing buffer ${buf.id} for source ${src.id} at ${startTime} (offset by ${startOffset})`);
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
    updateSourceTime: (src) => {
      var currentTime = src.context.audioCtx.currentTime;
      if (src.state !== AL_PLAYING) {
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
        AL.setSourceState(src, AL_STOPPED);
      } else if (src.type === AL_STATIC && src.looping) {
        // If the source is a looping static buffer, determine the buffer offset based on the loop points
        var buf = src.bufQueue[0];
        if (!buf.length) {
          src.bufOffset = 0.0;
        } else {
          var delta = (currentTime - src.bufStartTime) * src.playbackRate;
          var loopStart = buf.audioBuf._loopStart ?? 0.0;
          var loopEnd = buf.audioBuf._loopEnd ?? buf.audioBuf.duration;
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

        if (src.type !== AL_STATIC && src.looping) {
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
              AL.setSourceState(src, AL_STOPPED);
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

    cancelPendingSourceAudio: (src) => {
      AL.updateSourceTime(src);

      for (var i = 1; i < src.audioQueue.length; i++) {
        var audioSrc = src.audioQueue[i];
        audioSrc.stop();
      }

      if (src.audioQueue.length > 1) {
        src.audioQueue.length = 1;
      }
    },

    stopSourceAudio: (src) => {
      for (var audioSrc of src.audioQueue) {
        audioSrc.stop();
      }
      src.audioQueue.length = 0;
    },

    setSourceState: (src, state) => {
      if (state === AL_PLAYING) {
        if (src.state === AL_PLAYING || src.state == AL_STOPPED) {
          src.bufsProcessed = 0;
          src.bufOffset = 0.0;
#if OPENAL_DEBUG
          dbg(`setSourceState() resetting and playing source ${src.id}`);
#endif
        } else {
#if OPENAL_DEBUG
          dbg(`setSourceState() playing source ${src.id} at ${src.bufOffset}`);
#endif
        }

        AL.stopSourceAudio(src);

        src.state = AL_PLAYING;
        src.bufStartTime = Number.NEGATIVE_INFINITY;
        AL.scheduleSourceAudio(src);
      } else if (state === AL_PAUSED) {
        if (src.state === AL_PLAYING) {
          // Store off the current offset to restore with on resume.
          AL.updateSourceTime(src);
          AL.stopSourceAudio(src);

          src.state = AL_PAUSED;
#if OPENAL_DEBUG
          dbg(`setSourceState() pausing source ${src.id} at ${src.bufOffset}`);
#endif
        }
      } else if (state === AL_STOPPED) {
        if (src.state !== AL_INITIAL) {
          src.state = AL_STOPPED;
          src.bufsProcessed = src.bufQueue.length;
          src.bufStartTime = Number.NEGATIVE_INFINITY;
          src.bufOffset = 0.0;
          AL.stopSourceAudio(src);
#if OPENAL_DEBUG
          dbg(`setSourceState() stopping source ${src.id}`);
#endif
        }
      } else if (state === AL_INITIAL) {
        if (src.state !== AL_INITIAL) {
          src.state = AL_INITIAL;
          src.bufsProcessed = 0;
          src.bufStartTime = Number.NEGATIVE_INFINITY;
          src.bufOffset = 0.0;
          AL.stopSourceAudio(src);
#if OPENAL_DEBUG
          dbg(`setSourceState() initializing source ${src.id}`);
#endif
        }
      }
    },

    setSourceStatev: (count, pSourceIds, state, funcname) => {
      if (!AL.requireValidSources(count, pSourceIds, funcname)) {
        return;
      }
      for (var i = 0; i < count; ++i) {
        var srcId = {{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}};
        AL.setSourceState(AL.currentCtx.sources.get(srcId), state);
      }
    },

    initSourcePanner: (src) => {
      if (src.type === AL_UNDETERMINED) {
        return;
      }

      // Find the first non-zero buffer in the queue to determine the proper format
      var templateBuf = AL.buffers[0];
      for (var buf of src.bufQueue) {
        if (buf.id) {
          templateBuf = buf;
          break;
        }
      }
      // Create a panner if AL_SOURCE_SPATIALIZE_SOFT is set to true, or alternatively if it's set to auto and the source is mono
      if (src.spatialize === AL_TRUE || (src.spatialize === AL_AUTO_SOFT && templateBuf.channels === 1)) {
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

    updateContextGlobal: (ctx) => {
      for (var src of ctx.sources.values()) {
        AL.updateSourceGlobal(src);
      }
    },

    updateSourceGlobal: (src) => {
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
      case AL_NONE:
        panner.distanceModel = 'inverse';
        panner.refDistance = FLT_MAX;
        break;
      case AL_INVERSE_DISTANCE:
      case AL_INVERSE_DISTANCE_CLAMPED:
        panner.distanceModel = 'inverse';
        break;
      case AL_LINEAR_DISTANCE:
      case AL_LINEAR_DISTANCE_CLAMPED:
        panner.distanceModel = 'linear';
        break;
      case AL_EXPONENT_DISTANCE:
      case AL_EXPONENT_DISTANCE_CLAMPED:
        panner.distanceModel = 'exponential';
        break;
      }
    },

    updateListenerSpace: (ctx) => {
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
      for (var src of ctx.sources.values()) {
        AL.updateSourceSpace(src);
      }
    },

    updateSourceSpace: (src) => {
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
      // a buffer source's coordinates are interpreted not in absolute world space, but as being relative
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

        var inverseMagnitude = (x, y, z) => {
          var length = Math.sqrt(x * x + y * y + z * z);

          if (length < Number.EPSILON) {
            return 0.0;
          }

          return 1.0 / length;
        };

        // Normalize the Back vector
        var invMag = inverseMagnitude(lBackX, lBackY, lBackZ);
        lBackX *= invMag;
        lBackY *= invMag;
        lBackZ *= invMag;

        // ...and the Up vector
        invMag = inverseMagnitude(lUpX, lUpY, lUpZ);
        lUpX *= invMag;
        lUpY *= invMag;
        lUpZ *= invMag;

        // Calculate the Right vector as the cross product of the Up and Back vectors
        var lRightX = (lUpY * lBackZ - lUpZ * lBackY);
        var lRightY = (lUpZ * lBackX - lUpX * lBackZ);
        var lRightZ = (lUpX * lBackY - lUpY * lBackX);

        // Back and Up might not be exactly perpendicular, so the cross product also needs normalization
        invMag = inverseMagnitude(lRightX, lRightY, lRightZ);
        lRightX *= invMag;
        lRightY *= invMag;
        lRightZ *= invMag;

        // Recompute Up from the now orthonormal Right and Back vectors so we have a fully orthonormal basis
        lUpX = (lBackY * lRightZ - lBackZ * lRightY);
        lUpY = (lBackZ * lRightX - lBackX * lRightZ);
        lUpZ = (lBackX * lRightY - lBackY * lRightX);

        var oldX = dirX;
        var oldY = dirY;
        var oldZ = dirZ;

        // Use our 3 vectors to apply a change-of-basis matrix to the source direction
        dirX = oldX * lRightX + oldY * lUpX + oldZ * lBackX;
        dirY = oldX * lRightY + oldY * lUpY + oldZ * lBackY;
        dirZ = oldX * lRightZ + oldY * lUpZ + oldZ * lBackZ;

        oldX = posX;
        oldY = posY;
        oldZ = posZ;

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
        // Assigning to panner.positionX/Y/Z unnecessarily seems to cause performance issues
        // See https://github.com/emscripten-core/emscripten/issues/15847

        if (posX != panner.positionX.value) panner.positionX.value = posX;
        if (posY != panner.positionY.value) panner.positionY.value = posY;
        if (posZ != panner.positionZ.value) panner.positionZ.value = posZ;
      } else {
#if OPENAL_DEBUG
        warnOnce('Panner position attributes are not present, falling back to setPosition()');
#endif
        panner.setPosition(posX, posY, posZ);
      }
      if (panner.orientationX) {
        // Assigning to panner.orientation/Y/Z unnecessarily seems to cause performance issues
        // See https://github.com/emscripten-core/emscripten/issues/15847

        if (dirX != panner.orientationX.value) panner.orientationX.value = dirX;
        if (dirY != panner.orientationY.value) panner.orientationY.value = dirY;
        if (dirZ != panner.orientationZ.value) panner.orientationZ.value = dirZ;
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

    updateSourceRate: (src) => {
      if (src.state === AL_PLAYING) {
        // clear scheduled buffers
        AL.cancelPendingSourceAudio(src);

        var audioSrc = src.audioQueue[0];
        if (!audioSrc) {
          return; // It is possible that AL.scheduleContextAudio() has not yet fed the next buffer, if so, skip.
        }

        var duration;
        if (src.type === AL_STATIC && src.looping) {
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

    sourceDuration: (src) => {
      var length = 0.0;
      for (var buf of src.bufQueue) {
        length += buf.audioBuf?.duration ?? 0.0;
      }
      return length;
    },

    sourceTell: (src) => {
      AL.updateSourceTime(src);

      var offset = 0.0;
      for (var i = 0; i < src.bufsProcessed; i++) {
        offset += src.bufQueue[i].audioBuf?.duration ?? 0.0;
      }
      offset += src.bufOffset;

      return offset;
    },

    sourceSeek: (src, offset) => {
      var playing = src.state == AL_PLAYING;
      if (playing) {
        AL.setSourceState(src, AL_INITIAL);
      }

      src.bufsProcessed = 0;
      for (var buf of src.bufQueue) {
        var duration = buf.audioBuf?.duration ?? 0.0;
        if (offset < duration) {
          break;
        }
        offset -= duration;
        src.bufsProcessed++;
      }
      // 'offset' is now the intra-buffer offset within the target buffer.
      src.bufOffset = offset;

      if (playing) {
        AL.setSourceState(src, AL_PLAYING);
      }
    },

    requireValidContext: (funcname) => {
#if OPENAL_DEBUG
      if (!AL.currentCtx) {
        dbg(`${funcname}() called without a valid context`);
      }
#endif
      return AL.currentCtx;
    },

    requireValidDevice: (deviceId, funcname) => {
      if (!(deviceId in AL.deviceRefCounts)) {
#if OPENAL_DEBUG
        dbg(`${funcname}() called with an invalid device`);
#endif
        AL.alcErr = ALC_INVALID_DEVICE;
        return false;
      }
      return true;
    },

    requireValidPointer: (ptr, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return false;
      }
      if (!ptr) {
#if OPENAL_DEBUG
        dbg(`${funcname}() called with a null pointer`);
#endif
        AL.currentCtx.err = AL_INVALID_VALUE;
        return false;
      }
      return true;
    },

    requireValidSource: (sourceId, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return null;
      }
      var src = AL.currentCtx.sources.get(sourceId);
      if (!src) {
#if OPENAL_DEBUG
        dbg(`${funcname}() called with an invalid source`);
#endif
        AL.currentCtx.err = AL_INVALID_NAME;
        return null;
      }
      return src;
    },

    requireValidSources: (count, pSourceIds, funcname) => {
      if (!AL.requireValidContext(funcname) || (count > 0 && !AL.requireValidPointer(pSourceIds, funcname))) {
        return false;
      }
      for (var i = 0; i < count; ++i) {
        if (!AL.requireValidSource({{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}}, funcname)) {
          return false;
        }
      }
      return true;
    },

    // ------------------------------------------------------
    // -- Accessor Helpers
    // ------------------------------------------------------

    getGlobalParam: (param, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return null;
      }

      switch (param) {
      case AL_DOPPLER_FACTOR:
        return AL.currentCtx.dopplerFactor;
      case AL_SPEED_OF_SOUND:
        return AL.currentCtx.speedOfSound;
      case AL_DISTANCE_MODEL:
        return AL.currentCtx.distanceModel;
      default:
#if OPENAL_DEBUG
        dbg(`${funcname}() param ${ptrToString(param)} is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return null;
      }
    },

    setGlobalParam: (param, value, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return;
      }

      switch (param) {
      case AL_DOPPLER_FACTOR:
        if (!Number.isFinite(value) || value < 0.0) { // Strictly negative values are disallowed
#if OPENAL_DEBUG
          dbg(`${funcname}() value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        AL.currentCtx.dopplerFactor = value;
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case AL_SPEED_OF_SOUND:
        if (!Number.isFinite(value) || value <= 0.0) { // Negative or zero values are disallowed
#if OPENAL_DEBUG
          dbg(`${funcname}() value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        AL.currentCtx.speedOfSound = value;
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case AL_DISTANCE_MODEL:
        switch (value) {
        case AL_NONE:
        case AL_INVERSE_DISTANCE:
        case AL_INVERSE_DISTANCE_CLAMPED:
        case AL_LINEAR_DISTANCE:
        case AL_LINEAR_DISTANCE_CLAMPED:
        case AL_EXPONENT_DISTANCE:
        case AL_EXPONENT_DISTANCE_CLAMPED:
          AL.currentCtx.distanceModel = value;
          AL.updateContextGlobal(AL.currentCtx);
          break;
        default:
#if OPENAL_DEBUG
          dbg(`${funcname}() value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        break;
      default:
#if OPENAL_DEBUG
        dbg(`${funcname}() param ${ptrToString(param)} is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return;
      }
    },

    getListenerParam: (param, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return null;
      }

      switch (param) {
      case AL_POSITION:
        return AL.currentCtx.listener.position;
      case AL_VELOCITY:
        return AL.currentCtx.listener.velocity;
      case AL_ORIENTATION:
        return AL.currentCtx.listener.direction.concat(AL.currentCtx.listener.up);
      case AL_GAIN:
        return AL.currentCtx.gain.gain.value;
      default:
#if OPENAL_DEBUG
        dbg(`${funcname}() param ${ptrToString(param)} is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return null;
      }
    },

    setListenerParam: (param, value, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return;
      }
      if (value === null) {
#if OPENAL_DEBUG
        dbg(`${funcname}(): param ${ptrToString(param)} has wrong signature`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return;
      }

      var listener = AL.currentCtx.listener;
      switch (param) {
      case AL_POSITION:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_POSITION value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        listener.position[0] = value[0];
        listener.position[1] = value[1];
        listener.position[2] = value[2];
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case AL_VELOCITY:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_VELOCITY value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        listener.velocity[0] = value[0];
        listener.velocity[1] = value[1];
        listener.velocity[2] = value[2];
        AL.updateListenerSpace(AL.currentCtx);
        break;
      case AL_GAIN:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_GAIN value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        AL.currentCtx.gain.gain.value = value;
        break;
      case AL_ORIENTATION:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])
          || !Number.isFinite(value[3]) || !Number.isFinite(value[4]) || !Number.isFinite(value[5])
        ) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_ORIENTATION value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
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
        dbg(`${funcname}() param ${ptrToString(param)} is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return;
      }
    },

    getBufferParam: (bufferId, param, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return null;
      }
      var buf = AL.buffers[bufferId];
      if (!buf || !bufferId) {
#if OPENAL_DEBUG
        dbg(`${funcname}() called with an invalid buffer`);
#endif
        AL.currentCtx.err = AL_INVALID_NAME;
        return null;
      }

      switch (param) {
      case AL_FREQUENCY:
        return buf.frequency;
      case AL_BITS:
        return buf.bytesPerSample * 8;
      case AL_CHANNELS:
        return buf.channels;
      case AL_SIZE:
        return buf.length * buf.bytesPerSample * buf.channels;
      case AL_LOOP_POINTS_SOFT:
        if (!buf.length) {
          return [0, 0];
        }
        return [
          (buf.audioBuf._loopStart ?? 0.0) * buf.frequency,
          buf.audioBuf._loopEnd ? buf.audioBuf._loopEnd * buf.frequency : buf.length
        ];
      default:
#if OPENAL_DEBUG
        dbg(`${funcname}() param ${ptrToString(param)} is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return null;
      }
    },

    setBufferParam: (bufferId, param, value, funcname) => {
      if (!AL.requireValidContext(funcname)) {
        return;
      }
      var buf = AL.buffers[bufferId];
      if (!buf || !bufferId) {
#if OPENAL_DEBUG
        dbg(`${funcname}() called with an invalid buffer`);
#endif
        AL.currentCtx.err = AL_INVALID_NAME;
        return;
      }
      if (value === null) {
#if OPENAL_DEBUG
        dbg(`${funcname}(): param ${ptrToString(param)} has wrong signature`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return;
      }

      switch (param) {
      case AL_SIZE:
        if (value) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_SIZE value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        // Per the spec, setting AL_SIZE to 0 is a legal NOP.
        break;
      case AL_LOOP_POINTS_SOFT:
        if (value[0] < 0 || value[0] > buf.length || value[1] < 0 || value[1] > buf.length || value[0] >= value[1]) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_LOOP_POINTS_SOFT value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        if (buf.refCount > 0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_LOOP_POINTS_SOFT set on bound buffer`);
#endif
          AL.currentCtx.err = AL_INVALID_OPERATION;
          return;
        }

        if (buf.audioBuf) {
          buf.audioBuf._loopStart = value[0] / buf.frequency;
          buf.audioBuf._loopEnd = value[1] / buf.frequency;
        }
        break;
      default:
#if OPENAL_DEBUG
        dbg(`${funcname}() param ${ptrToString(param)}' is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return;
      }
    },

    getSourceParam: (sourceId, param, funcname) => {
      var src = AL.requireValidSource(sourceId, funcname);
      if (!src) {
        return null;
      }

      switch (param) {
      case AL_SOURCE_RELATIVE:
        return src.relative;
      case AL_CONE_INNER_ANGLE:
        return src.coneInnerAngle;
      case AL_CONE_OUTER_ANGLE:
        return src.coneOuterAngle;
      case AL_PITCH:
        return src.pitch;
      case AL_POSITION:
        return src.position;
      case AL_DIRECTION:
        return src.direction;
      case AL_VELOCITY:
        return src.velocity;
      case AL_LOOPING:
        return src.looping;
      case AL_BUFFER:
        if (src.type === AL_STATIC) {
          return src.bufQueue[0].id;
        }
        return 0;
      case AL_GAIN:
        return src.gain.gain.value;
      case AL_MIN_GAIN:
        return src.minGain;
      case AL_MAX_GAIN:
        return src.maxGain;
      case AL_SOURCE_STATE:
        return src.state;
      case AL_BUFFERS_QUEUED:
        if (src.bufQueue.length === 1 && !src.bufQueue[0].id) {
          return 0;
        }
        return src.bufQueue.length;
      case AL_BUFFERS_PROCESSED:
        if ((src.bufQueue.length === 1 && !src.bufQueue[0].id) || src.looping) {
          return 0;
        }
        return src.bufsProcessed;
      case AL_REFERENCE_DISTANCE:
        return src.refDistance;
      case AL_ROLLOFF_FACTOR:
        return src.rolloffFactor;
      case AL_CONE_OUTER_GAIN:
        return src.coneOuterGain;
      case AL_MAX_DISTANCE:
        return src.maxDistance;
      case AL_SEC_OFFSET:
        return AL.sourceTell(src);
      case AL_SAMPLE_OFFSET:
        var offset = AL.sourceTell(src);
        if (offset > 0.0) {
          offset *= src.bufQueue[0].frequency;
        }
        return offset;
      case AL_BYTE_OFFSET:
        var offset = AL.sourceTell(src);
        if (offset > 0.0) {
          offset *= src.bufQueue[0].frequency * src.bufQueue[0].bytesPerSample;
        }
        return offset;
      case AL_SOURCE_TYPE:
        return src.type;
      case AL_SOURCE_SPATIALIZE_SOFT:
        return src.spatialize;
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
        var length = 0;
        var bytesPerFrame = 0;
        for (var buf of src.bufQueue) {
          length += buf.length;
          if (buf.id) {
            bytesPerFrame = buf.bytesPerSample * buf.channels;
          }
        }
        return length * bytesPerFrame;
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
        var length = 0;
        for (var buf of src.bufQueue) {
          length += buf.length;
        }
        return length;
      case 0x200B /* AL_SEC_LENGTH_SOFT */:
        return AL.sourceDuration(src);
      case AL_DISTANCE_MODEL:
        return src.distanceModel;
      default:
#if OPENAL_DEBUG
        dbg(`${funcname}() param ${ptrToString(param)}' is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return null;
      }
    },

    setSourceParam: (sourceId, param, value, funcname) => {
      var src = AL.requireValidSource(sourceId, funcname);
      if (!src) {
        return;
      }
      if (value === null) {
#if OPENAL_DEBUG
        dbg(`${funcname}(): param ${ptrToString(param)}' has wrong signature`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return;
      }

      switch (param) {
      case AL_SOURCE_RELATIVE:
        if (value === AL_TRUE) {
          src.relative = true;
          AL.updateSourceSpace(src);
        } else if (value === AL_FALSE) {
          src.relative = false;
          AL.updateSourceSpace(src);
        } else {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_SOURCE_RELATIVE value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        break;
      case AL_CONE_INNER_ANGLE:
        if (!Number.isFinite(value)) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_CONE_INNER_ANGLE value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        src.coneInnerAngle = value;
        if (src.panner) {
          src.panner.coneInnerAngle = value % 360.0;
        }
        break;
      case AL_CONE_OUTER_ANGLE:
        if (!Number.isFinite(value)) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_CONE_OUTER_ANGLE value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        src.coneOuterAngle = value;
        if (src.panner) {
          src.panner.coneOuterAngle = value % 360.0;
        }
        break;
      case AL_PITCH:
        if (!Number.isFinite(value) || value <= 0.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_PITCH value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        if (src.pitch === value) {
          break;
        }

        src.pitch = value;
        AL.updateSourceRate(src);
        break;
      case AL_POSITION:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_POSITION value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        src.position[0] = value[0];
        src.position[1] = value[1];
        src.position[2] = value[2];
        AL.updateSourceSpace(src);
        break;
      case AL_DIRECTION:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_DIRECTION value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        src.direction[0] = value[0];
        src.direction[1] = value[1];
        src.direction[2] = value[2];
        AL.updateSourceSpace(src);
        break;
      case AL_VELOCITY:
        if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_VELOCITY value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        src.velocity[0] = value[0];
        src.velocity[1] = value[1];
        src.velocity[2] = value[2];
        AL.updateSourceSpace(src);
        break;
      case AL_LOOPING:
        if (value === AL_TRUE) {
          src.looping = true;
          AL.updateSourceTime(src);
          if (src.type === AL_STATIC && src.audioQueue.length > 0) {
            var audioSrc  = src.audioQueue[0];
            audioSrc.loop = true;
            audioSrc._duration = Number.POSITIVE_INFINITY;
          }
        } else if (value === AL_FALSE) {
          src.looping = false;
          var currentTime = AL.updateSourceTime(src);
          if (src.type === AL_STATIC && src.audioQueue.length > 0) {
            var audioSrc  = src.audioQueue[0];
            audioSrc.loop = false;
            audioSrc._duration = src.bufQueue[0].audioBuf.duration / src.playbackRate;
            audioSrc._startTime = currentTime - src.bufOffset / src.playbackRate;
          }
        } else {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_LOOPING value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        break;
      case AL_BUFFER:
        if (src.state === AL_PLAYING || src.state === AL_PAUSED) {
#if OPENAL_DEBUG
          dbg(`${funcname}(AL_BUFFER) called while source is playing or paused`);
#endif
          AL.currentCtx.err = AL_INVALID_OPERATION;
          return;
        }

        var buf = AL.buffers[value];
        if (!buf) {
#if OPENAL_DEBUG
          dbg('alSourcei(AL_BUFFER) called with an invalid buffer');
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        for (var oldBuf of src.bufQueue) {
          oldBuf.refCount--;
        }

        src.bufQueue = [buf];
        src.bufsProcessed = 0;

        if (!value) {
          src.type = AL_UNDETERMINED;
        } else {
          buf.refCount++;
          src.type = AL_STATIC;
        }

        AL.initSourcePanner(src);
        AL.scheduleSourceAudio(src);
        break;
      case AL_GAIN:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_GAIN value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        src.gain.gain.value = value;
        break;
      case AL_MIN_GAIN:
        if (!Number.isFinite(value) || value < 0.0 || value > Math.min(src.maxGain, 1.0)) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_MIN_GAIN value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
#if OPENAL_DEBUG
        warnOnce('AL_MIN_GAIN is not currently supported');
#endif
        src.minGain = value;
        break;
      case AL_MAX_GAIN:
        if (!Number.isFinite(value) || value < Math.max(0.0, src.minGain) || value > 1.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_MAX_GAIN value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
#if OPENAL_DEBUG
        warnOnce('AL_MAX_GAIN is not currently supported');
#endif
        src.maxGain = value;
        break;
      case AL_REFERENCE_DISTANCE:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_REFERENCE_DISTANCE value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        src.refDistance = value;
        if (src.panner) {
          src.panner.refDistance = value;
        }
        break;
      case AL_ROLLOFF_FACTOR:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_ROLLOFF_FACTOR value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        src.rolloffFactor = value;
        if (src.panner) {
          src.panner.rolloffFactor = value;
        }
        break;
      case AL_CONE_OUTER_GAIN:
        if (!Number.isFinite(value) || value < 0.0 || value > 1.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_CORE_OUTER_GAIN value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        src.coneOuterGain = value;
        if (src.panner) {
          src.panner.coneOuterGain = value;
        }
        break;
      case AL_MAX_DISTANCE:
        if (!Number.isFinite(value) || value < 0.0) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_MAX_DISTANCE value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        src.maxDistance = value;
        if (src.panner) {
          src.panner.maxDistance = value;
        }
        break;
      case AL_SEC_OFFSET:
        if (value < 0.0 || value > AL.sourceDuration(src)) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_SEC_OFFSET value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        AL.sourceSeek(src, value);
        break;
      case AL_SAMPLE_OFFSET:
        var srcLen = AL.sourceDuration(src);
        if (srcLen > 0.0) {
          var frequency;
          for (var buf of src.bufQueue) {
            if (buf.id) {
              frequency = buf.frequency;
              break;
            }
          }
          value /= frequency;
        }
        if (value < 0.0 || value > srcLen) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_SAMPLE_OFFSET value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        AL.sourceSeek(src, value);
        break;
      case AL_BYTE_OFFSET:
        var srcLen = AL.sourceDuration(src);
        if (srcLen > 0.0) {
          var bytesPerSec;
          for (var buf of src.bufQueue) {
            if (buf.id) {
              bytesPerSec = buf.frequency * buf.bytesPerSample * buf.channels;
              break;
            }
          }
          value /= bytesPerSec;
        }
        if (value < 0.0 || value > srcLen) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_BYTE_OFFSET value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        AL.sourceSeek(src, value);
        break;
      case AL_SOURCE_SPATIALIZE_SOFT:
        if (value !== AL_FALSE && value !== AL_TRUE && value !== AL_AUTO_SOFT) {
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_SOURCE_SPATIALIZE_SOFT value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }

        src.spatialize = value;
        AL.initSourcePanner(src);
        break;
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
      case 0x200B /* AL_SEC_LENGTH_SOFT */:
#if OPENAL_DEBUG
        dbg(`${funcname}() param AL_*_LENGTH_SOFT is read only`);
#endif
        AL.currentCtx.err = AL_INVALID_OPERATION;
        break;
      case AL_DISTANCE_MODEL:
        switch (value) {
        case AL_NONE:
        case AL_INVERSE_DISTANCE:
        case AL_INVERSE_DISTANCE_CLAMPED:
        case AL_LINEAR_DISTANCE:
        case AL_LINEAR_DISTANCE_CLAMPED:
        case AL_EXPONENT_DISTANCE:
        case AL_EXPONENT_DISTANCE_CLAMPED:
          src.distanceModel = value;
          if (AL.currentCtx.sourceDistanceModel) {
            AL.updateContextGlobal(AL.currentCtx);
          }
          break;
        default:
#if OPENAL_DEBUG
          dbg(`${funcname}() param AL_DISTANCE_MODEL value ${value} is out of range`);
#endif
          AL.currentCtx.err = AL_INVALID_VALUE;
          return;
        }
        break;
      default:
#if OPENAL_DEBUG
        dbg(`${funcname}() param ${ptrToString(param)} is unknown or not implemented`);
#endif
        AL.currentCtx.err = AL_INVALID_ENUM;
        return;
      }
    },

    // -------------------------------------------------------
    // -- Capture
    // -------------------------------------------------------

    // A map of 'capture device contexts'.
    captures: new Map(),

    sharedCaptureAudioCtx: null,

    // Helper which:
    // - Asserts that deviceId is both non-NULL AND a known device ID;
    // - Returns a reference to it, or null if not found.
    // - Sets alcErr accordingly.
    // Treat NULL and <invalid> separately because careless
    // people might assume that most alcCapture functions
    // accept NULL as a 'use the default' device.
    requireValidCaptureDevice: (deviceId, funcname) => {
      if (!deviceId) {
#if OPENAL_DEBUG
        dbg(`${funcname}() on a NULL device is an error`);
#endif
        AL.alcErr = ALC_INVALID_DEVICE;
        return null;
      }
      var c = AL.captures.get(deviceId);
      if (!c) {
#if OPENAL_DEBUG
        dbg(`${funcname}() on an invalid device`);
#endif
        AL.alcErr = ALC_INVALID_DEVICE;
        return null;
      }
      var err = c.mediaStreamError;
      if (err) {
#if OPENAL_DEBUG
        switch (err.name) {
        case 'PermissionDeniedError':
          dbg(`${funcname}() but the user denied access to the device`);
          break;
        case 'NotFoundError':
          dbg(`${funcname}() but no capture device was found`);
          break;
        default:
          dbg(`${funcname}() but a MediaStreamError was encountered: ${err}`);
          break;
        }
#endif
        AL.alcErr = ALC_INVALID_DEVICE;
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
  alcCaptureOpenDevice: (pDeviceName, requestedSampleRate, format, bufferFrameCapacity) => {

    var resolvedDeviceName = AL_CAPTURE_DEVICE_NAME;

    // NULL is a valid device name here (resolves to default);
    if (pDeviceName) {
      resolvedDeviceName = UTF8ToString(pDeviceName);
      if (resolvedDeviceName !== AL_CAPTURE_DEVICE_NAME) {
#if OPENAL_DEBUG
        dbg(`alcCaptureOpenDevice() with invalid device name '${resolvedDeviceName}'`);
#endif
        // ALC_OUT_OF_MEMORY
        // From the programmer's guide, ALC_OUT_OF_MEMORY's meaning is
        // overloaded here, to mean:
        // 'The specified device is invalid, or can not capture audio.'
        // This may be misleading to API users, but well...
        AL.alcErr = ALC_OUT_OF_MEMORY;
        return 0;
      }
    }

    // Otherwise it's probably okay (though useless) for bufferFrameCapacity to be zero.
    if (bufferFrameCapacity < 0) { // ALCsizei is signed int
#if OPENAL_DEBUG
      dbg('alcCaptureOpenDevice() with negative bufferSize');
#endif
      AL.alcErr = ALC_INVALID_VALUE;
      return 0;
    }

    if (!navigator.mediaDevices?.getUserMedia) {
#if OPENAL_DEBUG
      dbg('alcCaptureOpenDevice() cannot capture audio, because your browser lacks a `getUserMedia()` implementation');
#endif
      // See previously mentioned rationale for ALC_OUT_OF_MEMORY
      AL.alcErr = ALC_OUT_OF_MEMORY;
      return 0;
    }

    if (!AL.sharedCaptureAudioCtx) {
      try {
        AL.sharedCaptureAudioCtx = new AudioContext();
      } catch(e) {
#if OPENAL_DEBUG
        dbg(`alcCaptureOpenDevice() could not create the shared capture AudioContext: ${e}`);
#endif
        // See previously mentioned rationale for ALC_OUT_OF_MEMORY
        AL.alcErr = ALC_OUT_OF_MEMORY;
        return 0;
      }
    }

    autoResumeAudioContext(AL.sharedCaptureAudioCtx);

    var outputChannelCount;

    switch (format) {
    case AL_FORMAT_MONO_FLOAT32:
    case AL_FORMAT_MONO16:
    case AL_FORMAT_MONO8:
      outputChannelCount = 1;
      break;
    case AL_FORMAT_STEREO_FLOAT32:
    case AL_FORMAT_STEREO16:
    case AL_FORMAT_STEREO8:
      outputChannelCount = 2;
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alcCaptureOpenDevice() with unsupported format ${format}`);
#endif
      AL.alcErr = ALC_INVALID_VALUE;
      return 0;
    }

    function newF32Array(cap) { return new Float32Array(cap);}
    function newI16Array(cap) { return new Int16Array(cap);  }
    function newU8Array(cap)  { return new Uint8Array(cap);  }

    var requestedSampleType;
    var newSampleArray;

    switch (format) {
    case AL_FORMAT_MONO_FLOAT32:
    case AL_FORMAT_STEREO_FLOAT32:
      requestedSampleType = 'f32';
      newSampleArray = newF32Array;
      break;
    case AL_FORMAT_MONO16:
    case AL_FORMAT_STEREO16:
      requestedSampleType = 'i16';
      newSampleArray = newI16Array;
      break;
    case AL_FORMAT_MONO8:
    case AL_FORMAT_STEREO8:
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
      dbg(`alcCaptureOpenDevice() failed to allocate internal buffers (is bufferSize low enough?): ${e}`);
#endif
      AL.alcErr = ALC_OUT_OF_MEMORY;
      return 0;
    }


    // What we'll place into the `AL.captures` array in the end,
    // declared here for closures to access it
    var newCapture = {
      audioCtx: AL.sharedCaptureAudioCtx,
      deviceName: resolvedDeviceName,
      requestedSampleRate,
      requestedSampleType,
      outputChannelCount,
      inputChannelCount: null, // Not known until the getUserMedia() promise resolves
      mediaStreamError: null, // Used by other functions to return early and report an error.
      mediaStreamSourceNode: null,
      mediaStream: null,
      // Either one, or none of the below two, is active.
      mergerNode: null,
      splitterNode: null,
      scriptProcessorNode: null,
      isCapturing: false,
      buffers,
      get bufferFrameCapacity() {
        return buffers[0].length;
      },
      capturePlayhead: 0, // current write position, in sample frames
      captureReadhead: 0,
      capturedFrameCount: 0
    };

    // Preparing for getUserMedia()

    var onError = (mediaStreamError) => {
      newCapture.mediaStreamError = mediaStreamError;
#if OPENAL_DEBUG
      dbg(`getUserMedia() errored with: ${mediaStreamError}`);
#endif
    };
    var onSuccess = (mediaStream) => {
      newCapture.mediaStreamSourceNode = newCapture.audioCtx.createMediaStreamSource(mediaStream);
      newCapture.mediaStream = mediaStream;

      var inputChannelCount = 1;
      switch (newCapture.mediaStreamSourceNode.channelCountMode) {
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
        dbg('The number of input or output channels is too high, capture might not work as expected!');
      }
#endif

      // Have to pick a size from 256, 512, 1024, 2048, 4096, 8192, 16384.
      // One can also set it to zero, which leaves the decision up to the impl.
      // An extension could allow specifying this value.
      const processorFrameCount = 512;

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

      newCapture.scriptProcessorNode.onaudioprocess = (audioProcessingEvent) => {
        if (!newCapture.isCapturing) {
          return;
        }

        var c = newCapture;
        var srcBuf = audioProcessingEvent.inputBuffer;

        // Actually just copy srcBuf's channel data into
        // c.buffers, optimizing for each case.
        switch (format) {
        case AL_FORMAT_MONO_FLOAT32:
          var channel0 = srcBuf.getChannelData(0);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i];
          }
          break;
        case AL_FORMAT_STEREO_FLOAT32:
          var channel0 = srcBuf.getChannelData(0);
          var channel1 = srcBuf.getChannelData(1);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i];
            c.buffers[1][wi] = channel1[i];
          }
          break;
        case AL_FORMAT_MONO16:
          var channel0 = srcBuf.getChannelData(0);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i] * 32767;
          }
          break;
        case AL_FORMAT_STEREO16:
          var channel0 = srcBuf.getChannelData(0);
          var channel1 = srcBuf.getChannelData(1);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = channel0[i] * 32767;
            c.buffers[1][wi] = channel1[i] * 32767;
          }
          break;
        case AL_FORMAT_MONO8:
          var channel0 = srcBuf.getChannelData(0);
          for (var i = 0 ; i < srcBuf.length; ++i) {
            var wi = (c.capturePlayhead + i) % c.bufferFrameCapacity;
            c.buffers[0][wi] = (channel0[i] + 1.0) * 127;
          }
          break;
        case AL_FORMAT_STEREO8:
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

    navigator.mediaDevices.getUserMedia({audio: true}).then(onSuccess).catch(onError);

    var id = AL.newId();
    AL.captures.set(id, newCapture);
    return id;
  },

  alcCaptureCloseDevice__proxy: 'sync',
  alcCaptureCloseDevice: (deviceId) => {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureCloseDevice');
    if (!c) return false;

    AL.captures.delete(deviceId);
    AL.freeIds.push(deviceId);

    // This clean-up might be unnecessary (paranoid) ?

    // May happen if user hasn't decided to grant or deny input
    c.mediaStreamSourceNode?.disconnect();
    c.mergerNode?.disconnect();
    c.splitterNode?.disconnect();
    // May happen if user hasn't decided to grant or deny input
    c.scriptProcessorNode?.disconnect();
    if (c.mediaStream) {
      // Disabling the microphone of the browser.
      // Without this operation, the red dot on the browser tab page will remain.
      c.mediaStream.getTracks().forEach((track) => track.stop());
    }

    delete c.buffers;

    c.capturedFrameCount = 0;
    c.isCapturing = false;

    return true;
  },

  alcCaptureStart__proxy: 'sync',
  alcCaptureStart: (deviceId) => {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureStart');
    if (!c) return;

    if (c.isCapturing) {
#if OPENAL_DEBUG
      dbg('Redundant call to alcCaptureStart()');
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
  alcCaptureStop: (deviceId) => {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureStop');
    if (!c) return;

#if OPENAL_DEBUG
    if (!c.isCapturing) {
      dbg('Redundant call to alcCaptureStop()');
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
  alcCaptureSamples: (deviceId, pFrames, requestedFrameCount) => {
    var c = AL.requireValidCaptureDevice(deviceId, 'alcCaptureSamples');
    if (!c) return;

    // ALCsizei is actually 32-bit signed int, so could be negative
    // Also, spec says :
    //   Requesting more sample frames than are currently available is
    //   an error.

    var dstfreq = c.requestedSampleRate;
    var srcfreq = c.audioCtx.sampleRate;

    var fratio = srcfreq / dstfreq;

    if (requestedFrameCount < 0
    ||  requestedFrameCount > (c.capturedFrameCount / fratio))
    {
#if OPENAL_DEBUG
      dbg('alcCaptureSamples() with invalid bufferSize');
#endif
      AL.alcErr = ALC_INVALID_VALUE;
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

    switch (c.requestedSampleType) {
    case 'f32': setSample = setF32Sample; break;
    case 'i16': setSample = setI16Sample; break;
    case 'u8' : setSample = setU8Sample ; break;
    default:
#if OPENAL_DEBUG
      dbg(`Internal error: Unknown sample type '${c.requestedSampleType}'`);
#endif
      return;
    }

    // If fratio is an integer we don't need linear resampling, just skip samples
    if (Math.floor(fratio) == fratio) {
      for (var i = 0, frame_i = 0; frame_i < requestedFrameCount; ++frame_i) {
        for (var chan = 0; chan < c.buffers.length; ++chan, ++i) {
          setSample(i, c.buffers[chan][c.captureReadhead]);
        }
        c.captureReadhead = (fratio + c.captureReadhead) % c.bufferFrameCapacity;
      }
    } else {
      // Perform linear resampling.

      // There is room for improvement - right now we're fine with linear resampling.
      // We don't use OfflineAudioContexts for this: See the discussion at
      // https://github.com/jpernst/emscripten/issues/2#issuecomment-312729735
      // if you're curious about why.
      for (var i = 0, frame_i = 0; frame_i < requestedFrameCount; ++frame_i) {
        var lefti = Math.floor(c.captureReadhead);
        var righti = Math.ceil(c.captureReadhead);
        var d = c.captureReadhead - lefti;
        for (var chan = 0; chan < c.buffers.length; ++chan, ++i) {
          var lefts = c.buffers[chan][lefti];
          var rights = c.buffers[chan][righti];
          setSample(i, (1 - d) * lefts + d * rights);
        }
        c.captureReadhead = (c.captureReadhead + fratio) % c.bufferFrameCapacity;
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
  alcOpenDevice: (pDeviceName) => {
    if (pDeviceName) {
      var name = UTF8ToString(pDeviceName);
      if (name !== AL_DEVICE_NAME) {
        return 0;
      }
    }

    if (!globalThis.AudioContext) {
      return 0;
    }

    var deviceId = AL.newId();
    AL.deviceRefCounts[deviceId] = 0;
    return deviceId;
  },

  alcCloseDevice__proxy: 'sync',
  alcCloseDevice: (deviceId) => {
    if (!(deviceId in AL.deviceRefCounts) || AL.deviceRefCounts[deviceId] > 0) {
      return ALC_FALSE;
    }

    delete AL.deviceRefCounts[deviceId];
    AL.freeIds.push(deviceId);
    return ALC_TRUE;
  },

  alcCreateContext__proxy: 'sync',
  alcCreateContext: (deviceId, pAttrList) => {
    if (!AL.requireValidDevice(deviceId, 'alcCreateContext')) {
      return 0;
    }

    var options = {};
    var attrs = [];
    var hrtf = null;
    pAttrList >>= 2;
    if (pAttrList) {
      var attr = 0;
      var val = 0;
      while (true) {
        attr = HEAP32[pAttrList++];
        attrs.push(attr);
        if (!attr) {
          break;
        }
        val = HEAP32[pAttrList++];
        attrs.push(val);

        switch (attr) {
        case ALC_FREQUENCY:
          options.sampleRate = val;
          break;
        case ALC_MONO_SOURCES: // fallthrough
        case ALC_STEREO_SOURCES:
          // Do nothing; these hints are satisfied by default
          break
        case ALC_HRTF_SOFT:
          switch (val) {
            case ALC_FALSE:
              hrtf = false;
              break;
            case ALC_TRUE:
              hrtf = true;
              break;
            case ALC_DONT_CARE_SOFT:
              break;
            default:
#if OPENAL_DEBUG
              dbg(`Unsupported ALC_HRTF_SOFT mode ${val}`);
#endif
              AL.alcErr = ALC_INVALID_VALUE;
              return 0;
          }
          break;
        case ALC_HRTF_ID_SOFT:
          if (val) {
#if OPENAL_DEBUG
            dbg(`Invalid ALC_HRTF_ID_SOFT index ${val}`);
#endif
            AL.alcErr = ALC_INVALID_VALUE;
            return 0;
          }
          break;
        default:
#if OPENAL_DEBUG
          dbg(`Unsupported context attribute ${ptrToString(attr)}`);
#endif
          AL.alcErr = ALC_INVALID_VALUE;
          return 0;
        }
      }
    }

    var ac;
    try {
      ac = new AudioContext(options);
    } catch (e) {
      if (e.name === 'NotSupportedError') {
#if OPENAL_DEBUG
        dbg('Invalid or unsupported options');
#endif
        AL.alcErr = ALC_INVALID_VALUE;
      } else {
        AL.alcErr = ALC_INVALID_DEVICE;
      }

      return 0;
    }

    autoResumeAudioContext(ac);

    // Old Web Audio API (e.g. Safari 6.0.5) had an inconsistently named createGainNode function.
    if (typeof ac.createGain === 'undefined') {
      ac.createGain = ac.createGainNode;
    }

    var gain = ac.createGain();
    gain.connect(ac.destination);
    var ctx = {
      deviceId,
      id: AL.newId(),
      attrs,
      audioCtx: ac,
      listener: {
        position: [0.0, 0.0, 0.0],
        velocity: [0.0, 0.0, 0.0],
        direction: [0.0, 0.0, 0.0],
        up: [0.0, 0.0, 0.0]
      },
      sources: new Map(),
      interval: setInterval(() => AL.scheduleContextAudio(ctx), AL_QUEUE_INTERVAL),
      gain,
      distanceModel: AL_INVERSE_DISTANCE_CLAMPED,
      speedOfSound: 343.3,
      dopplerFactor: 1.0,
      sourceDistanceModel: false,
      hrtf: hrtf || false,

      _err: AL_NO_ERROR,
      get err() {
        return this._err;
      },
      set err(val) {
        // Errors should not be overwritten by later errors until they are cleared by a query.
        if (this._err === AL_NO_ERROR || val === AL_NO_ERROR) {
          this._err = val;
        }
      }
    };
    AL.deviceRefCounts[deviceId]++;
    AL.contexts.set(ctx.id, ctx);

    if (hrtf !== null) {
      // Apply hrtf attrib to all contexts for this device
      for (var c of AL.contexts.values()) {
        if (c.deviceId === deviceId) {
          c.hrtf = hrtf;
          AL.updateContextGlobal(c);
        }
      }
    }

    return ctx.id;
  },

  alcDestroyContext__proxy: 'sync',
  alcDestroyContext: (contextId) => {
    var ctx = AL.contexts.get(contextId);
    if (!ctx || AL.currentCtx === ctx) {
#if OPENAL_DEBUG
      dbg('alcDestroyContext() called with an invalid context');
#endif
      AL.alcErr = ALC_INVALID_CONTEXT;
      return;
    }

    // Stop playback, etc
    if (ctx.interval) {
      clearInterval(ctx.interval);
    }
    AL.deviceRefCounts[ctx.deviceId]--;
    AL.contexts.delete(contextId);
    AL.freeIds.push(contextId);
  },

  // -------------------------------------------------------
  // -- ALC State
  // -------------------------------------------------------

  alcGetError__proxy: 'sync',
  alcGetError: (deviceId) => {
    var err = AL.alcErr;
    AL.alcErr = ALC_NO_ERROR;
    return err;
  },

  alcGetCurrentContext__proxy: 'sync',
  alcGetCurrentContext: () => AL.currentCtx ? AL.currentCtx.id : 0,

  alcMakeContextCurrent__proxy: 'sync',
  alcMakeContextCurrent: (contextId) => {
    AL.currentCtx = AL.contexts.get(contextId);
    return ALC_TRUE;
  },

  alcGetContextsDevice__proxy: 'sync',
  alcGetContextsDevice: (contextId) => AL.contexts.get(contextId)?.deviceId || 0,

  // The spec is vague about what these are actually supposed to do, and NOP is a reasonable implementation
  alcProcessContext: (contextId) => {},
  alcSuspendContext: (contextId) => {},

  alcIsExtensionPresent__proxy: 'sync',
  alcIsExtensionPresent: (deviceId, pExtName) => {
    var name = UTF8ToString(pExtName);

    return AL.ALC_EXTENSIONS[name] ? ALC_TRUE : ALC_FALSE;
  },

  $alcEnumValues__internal: true,
  // Note: using template literal here instead of plain object because jsify
  // serializes identifier keys w/o quotes and Closure would mangle them.
  $alcEnumValues: `{
    'ALC_NO_ERROR': ALC_NO_ERROR,
    'ALC_INVALID_DEVICE': ALC_INVALID_DEVICE,
    'ALC_INVALID_CONTEXT': ALC_INVALID_CONTEXT,
    'ALC_INVALID_ENUM': ALC_INVALID_ENUM,
    'ALC_INVALID_VALUE': ALC_INVALID_VALUE,
    'ALC_OUT_OF_MEMORY': ALC_OUT_OF_MEMORY,
    'ALC_MAJOR_VERSION': ALC_MAJOR_VERSION,
    'ALC_MINOR_VERSION': ALC_MINOR_VERSION,
    'ALC_ATTRIBUTES_SIZE': ALC_ATTRIBUTES_SIZE,
    'ALC_ALL_ATTRIBUTES': ALC_ALL_ATTRIBUTES,
    'ALC_DEFAULT_DEVICE_SPECIFIER': ALC_DEFAULT_DEVICE_SPECIFIER,
    'ALC_DEVICE_SPECIFIER': ALC_DEVICE_SPECIFIER,
    'ALC_EXTENSIONS': ALC_EXTENSIONS,
    'ALC_FREQUENCY': ALC_FREQUENCY,
    'ALC_REFRESH': ALC_REFRESH,
    'ALC_SYNC': ALC_SYNC,
    'ALC_MONO_SOURCES': ALC_MONO_SOURCES,
    'ALC_STEREO_SOURCES': ALC_STEREO_SOURCES,
    'ALC_CAPTURE_DEVICE_SPECIFIER': ALC_CAPTURE_DEVICE_SPECIFIER,
    'ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER': ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER,
    'ALC_CAPTURE_SAMPLES': ALC_CAPTURE_SAMPLES,

    /* Extensions */
    'ALC_HRTF_SOFT': ALC_HRTF_SOFT,
    'ALC_HRTF_ID_SOFT': ALC_HRTF_ID_SOFT,
    'ALC_DONT_CARE_SOFT': ALC_DONT_CARE_SOFT,
    'ALC_HRTF_STATUS_SOFT': ALC_HRTF_STATUS_SOFT,
    'ALC_NUM_HRTF_SPECIFIERS_SOFT': ALC_NUM_HRTF_SPECIFIERS_SOFT,
    'ALC_HRTF_SPECIFIER_SOFT': ALC_HRTF_SPECIFIER_SOFT,
    'ALC_HRTF_DISABLED_SOFT': ALC_HRTF_DISABLED_SOFT,
    'ALC_HRTF_ENABLED_SOFT': ALC_HRTF_ENABLED_SOFT,
    'ALC_HRTF_DENIED_SOFT': ALC_HRTF_DENIED_SOFT,
    'ALC_HRTF_REQUIRED_SOFT': ALC_HRTF_REQUIRED_SOFT,
    'ALC_HRTF_HEADPHONES_DETECTED_SOFT': ALC_HRTF_HEADPHONES_DETECTED_SOFT,
    'ALC_HRTF_UNSUPPORTED_FORMAT_SOFT': ALC_HRTF_UNSUPPORTED_FORMAT_SOFT,
  }`,

  alcGetEnumValue__proxy: 'sync',
  alcGetEnumValue: (deviceId, pEnumName) => {
    // Spec says :
    // Using a NULL handle is legal, but only the
    // tokens defined by the AL core are guaranteed.
    if (deviceId && !(deviceId in AL.deviceRefCounts)) {
#if OPENAL_DEBUG
      dbg('alcGetEnumValue() called with an invalid device');
#endif
      // ALC_INVALID_DEVICE is not listed as a possible error state for
      // this function, sadly.
      return 0;
    } else if (!pEnumName) {
      AL.alcErr = ALC_INVALID_VALUE;
      return 0;
    }
    var name = UTF8ToString(pEnumName);
    // See alGetEnumValue(), but basically behave the same as OpenAL-Soft
    if (hasOwn(alcEnumValues, name)) {
      return alcEnumValues[name];
    }
#if OPENAL_DEBUG
    dbg(`No value for '${pEnumName}' is known by alcGetEnumValue()`);
#endif
    AL.alcErr = ALC_INVALID_VALUE;
    return AL_NONE;
  },

  alcGetString__proxy: 'sync',
  alcGetString: (deviceId, param) => {
    if (AL.alcStringCache[param]) {
      return AL.alcStringCache[param];
    }

    var ret;
    switch (param) {
    case ALC_NO_ERROR:
      ret = 'No Error';
      break;
    case ALC_INVALID_DEVICE:
      ret = 'Invalid Device';
      break;
    case ALC_INVALID_CONTEXT:
      ret = 'Invalid Context';
      break;
    case ALC_INVALID_ENUM:
      ret = 'Invalid Enum';
      break;
    case ALC_INVALID_VALUE:
      ret = 'Invalid Value';
      break;
    case ALC_OUT_OF_MEMORY:
      ret = 'Out of Memory';
      break;
    case ALC_DEFAULT_DEVICE_SPECIFIER:
      if (globalThis.AudioContext) {
        ret = AL_DEVICE_NAME;
      } else {
        return 0;
      }
      break;
    case ALC_DEVICE_SPECIFIER:
      if (globalThis.AudioContext) {
        ret = AL_DEVICE_NAME + '\0';
      } else {
        ret = '\0';
      }
      break;
    case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
      ret = AL_CAPTURE_DEVICE_NAME;
      break;
    case ALC_CAPTURE_DEVICE_SPECIFIER:
      if (!deviceId) {
        ret = AL_CAPTURE_DEVICE_NAME + '\0';
      } else {
        var c = AL.requireValidCaptureDevice(deviceId, 'alcGetString');
        if (!c) {
          return 0;
        }
        ret = c.deviceName;
      }
      break;
    case ALC_EXTENSIONS:
      if (!deviceId) {
        AL.alcErr = ALC_INVALID_DEVICE;
        return 0;
      }

      ret = Object.keys(AL.ALC_EXTENSIONS).join(' ')
      break;
    default:
      AL.alcErr = ALC_INVALID_ENUM;
      return 0;
    }

    ret = stringToNewUTF8(ret);
    AL.alcStringCache[param] = ret;
    return ret;
  },

  alcGetIntegerv__proxy: 'sync',
  alcGetIntegerv: (deviceId, param, size, pValues) => {
    if (!size || !pValues) {
      // Ignore the query, per the spec
      return;
    }

    switch (param) {
    case ALC_MAJOR_VERSION:
      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
      break;
    case ALC_MINOR_VERSION:
      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
      break;
    case ALC_ATTRIBUTES_SIZE:
      if (!AL.requireValidDevice(deviceId, 'alcGetIntegerv')) {
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = ALC_INVALID_CONTEXT;
        return;
      }

      {{{ makeSetValue('pValues', '0', 'AL.currentCtx.attrs.length', 'i32') }}};
      break;
    case ALC_ALL_ATTRIBUTES:
      if (!AL.requireValidDevice(deviceId, 'alcGetIntegerv')) {
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = ALC_INVALID_CONTEXT;
        return;
      }

      for (var i = 0; i < AL.currentCtx.attrs.length; i++) {
        {{{ makeSetValue('pValues', 'i*4', 'AL.currentCtx.attrs[i]', 'i32') }}};
      }
      break;
    case ALC_FREQUENCY:
      if (!AL.requireValidDevice(deviceId, 'alcGetIntegerv')) {
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = ALC_INVALID_CONTEXT;
        return;
      }

      {{{ makeSetValue('pValues', '0', 'AL.currentCtx.audioCtx.sampleRate', 'i32') }}};
      break;
    case ALC_MONO_SOURCES:
    case ALC_STEREO_SOURCES:
      if (!AL.requireValidDevice(deviceId, 'alcGetIntegerv')) {
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = ALC_INVALID_CONTEXT;
        return;
      }

      {{{ makeSetValue('pValues', '0', cDefs.INT_MAX, 'i32') }}};
      break;
    case ALC_HRTF_SOFT:
    case ALC_HRTF_STATUS_SOFT:
      if (!AL.requireValidDevice(deviceId, 'alcGetIntegerv')) {
        return;
      }

      var hrtfStatus = ALC_HRTF_DISABLED_SOFT;
      for (var ctx of AL.contexts.values()) {
        if (ctx.deviceId === deviceId) {
          hrtfStatus = ctx.hrtf ? ALC_HRTF_ENABLED_SOFT : ALC_HRTF_DISABLED_SOFT;
        }
      }
      {{{ makeSetValue('pValues', '0', 'hrtfStatus', 'i32') }}};
      break;
    case ALC_NUM_HRTF_SPECIFIERS_SOFT:
      if (!AL.requireValidDevice(deviceId, 'alcGetIntegerv')) {
        return;
      }
      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
      break;
    case 0x20003 /* ALC_MAX_AUXILIARY_SENDS */:
      if (!AL.requireValidDevice(deviceId, 'alcGetIntegerv')) {
        return;
      }
      if (!AL.currentCtx) {
        AL.alcErr = ALC_INVALID_CONTEXT;
        return;
      }

      {{{ makeSetValue('pValues', '0', '1', 'i32') }}};
      break;
    case ALC_CAPTURE_SAMPLES:
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
      dbg(`alcGetIntegerv() with param ${ptrToString(param)} not implemented yet`);
#endif
      AL.alcErr = ALC_INVALID_ENUM;
      return;
    }
  },

  emscripten_alcDevicePauseSOFT__proxy: 'sync',
  emscripten_alcDevicePauseSOFT__sig: 'vi',
  emscripten_alcDevicePauseSOFT: (deviceId) => {
    if (!AL.requireValidDevice(deviceId, 'alcDevicePauseSOFT')) {
      return;
    }

    if (AL.paused) {
      return;
    }
    AL.paused = true;

    for (var ctx of AL.contexts.values()) {
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
  emscripten_alcDeviceResumeSOFT: (deviceId) => {
    if (!AL.requireValidDevice(deviceId, 'alcDeviceResumeSOFT')) {
      return;
    }

    if (!AL.paused) {
      return;
    }
    AL.paused = false;

    for (var ctx of AL.contexts.values()) {
      if (ctx.deviceId !== deviceId) {
        continue;
      }

      ctx.interval = setInterval(() => AL.scheduleContextAudio(ctx), AL_QUEUE_INTERVAL);
      ctx.audioCtx.resume();
    }
  },

  emscripten_alcGetStringiSOFT__proxy: 'sync',
  emscripten_alcGetStringiSOFT__sig: 'iiii',
  emscripten_alcGetStringiSOFT: (deviceId, param, index) => {
    if (!AL.requireValidDevice(deviceId, 'alcGetStringiSOFT')) {
      return 0;
    }

    if (AL.alcStringCache[param]) {
      return AL.alcStringCache[param];
    }

    var ret;
    switch (param) {
    case ALC_HRTF_SPECIFIER_SOFT:
      if (!index) {
        ret = 'Web Audio HRTF';
      } else {
#if OPENAL_DEBUG
        dbg(`alcGetStringiSOFT() with param ALC_HRTF_SPECIFIER_SOFT index ${index} is out of range`);
#endif
        AL.alcErr = ALC_INVALID_VALUE;
        return 0;
      }
      break;
    default:
      if (index) {
#if OPENAL_DEBUG
        dbg(`alcGetStringiSOFT() with param ${ptrToString(param)} not implemented yet`);
#endif
        AL.alcErr = ALC_INVALID_ENUM;
        return 0;
      }
      return _alcGetString(deviceId, param);
    }

    ret = stringToNewUTF8(ret);
    AL.alcStringCache[param] = ret;
    return ret;
  },

  emscripten_alcResetDeviceSOFT__proxy: 'sync',
  emscripten_alcResetDeviceSOFT__sig: 'iii',
  emscripten_alcResetDeviceSOFT: (deviceId, pAttrList) => {
    if (!AL.requireValidDevice(deviceId, 'alcResetDeviceSOFT')) {
      return ALC_FALSE;
    }

    var hrtf = null;
    pAttrList >>= 2;
    if (pAttrList) {
      var attr = 0;
      var val = 0;
      while (true) {
        attr = HEAP32[pAttrList++];
        if (!attr) {
          break;
        }
        val = HEAP32[pAttrList++];

        switch (attr) {
        case ALC_HRTF_SOFT:
          if (val === ALC_TRUE) {
            hrtf = true;
          } else if (val === ALC_FALSE) {
            hrtf = false;
          }
          break;
        }
      }
    }

    if (hrtf !== null) {
      // Apply hrtf attrib to all contexts for this device
      for (var ctx of AL.contexts.values()) {
        if (ctx.deviceId === deviceId) {
          ctx.hrtf = hrtf;
          AL.updateContextGlobal(ctx);
        }
      }
    }

    return ALC_TRUE;
  },

  // ***************************************************************************
  // ** AL API
  // ***************************************************************************

  // -------------------------------------------------------
  // -- AL Resources
  // -------------------------------------------------------

  alGenBuffers__proxy: 'sync',
  alGenBuffers: (count, pBufferIds) => {
    if (!AL.requireValidContext('alGenBuffers')) {
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
  alDeleteBuffers: (count, pBufferIds) => {
    if (!AL.requireValidContext('alDeleteBuffers')) {
      return;
    }

    for (var i = 0; i < count; ++i) {
      var bufId = {{{ makeGetValue('pBufferIds', 'i*4', 'i32') }}};
      // Deleting the zero buffer is a legal NOP, so ignore it
      if (!bufId) {
        continue;
      }

      // Make sure the buffer index is valid.
      if (!AL.buffers[bufId]) {
#if OPENAL_DEBUG
        dbg('alDeleteBuffers() called with an invalid buffer');
#endif
        AL.currentCtx.err = AL_INVALID_NAME;
        return;
      }

      // Make sure the buffer is no longer in use.
      if (AL.buffers[bufId].refCount) {
#if OPENAL_DEBUG
        dbg('alDeleteBuffers() called with a used buffer');
#endif
        AL.currentCtx.err = AL_INVALID_OPERATION;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      var bufId = {{{ makeGetValue('pBufferIds', 'i*4', 'i32') }}};
      if (!bufId) {
        continue;
      }

      AL.deviceRefCounts[AL.buffers[bufId].deviceId]--;
      delete AL.buffers[bufId];
      AL.freeIds.push(bufId);
    }
  },

  alGenSources__proxy: 'sync',
  alGenSources: (count, pSourceIds) => {
    if (!AL.requireValidContext('alGenSources')) {
      return;
    }
    for (var i = 0; i < count; ++i) {
      var gain = AL.currentCtx.audioCtx.createGain();
      gain.connect(AL.currentCtx.gain);
      var src = {
        context: AL.currentCtx,
        id: AL.newId(),
        type: AL_UNDETERMINED,
        state: AL_INITIAL,
        bufQueue: [AL.buffers[0]],
        audioQueue: [],
        looping: false,
        pitch: 1.0,
        dopplerShift: 1.0,
        gain,
        minGain: 0.0,
        maxGain: 1.0,
        panner: null,
        bufsProcessed: 0,
        bufStartTime: Number.NEGATIVE_INFINITY,
        bufOffset: 0.0,
        relative: false,
        refDistance: 1.0,
        maxDistance: FLT_MAX,
        rolloffFactor: 1.0,
        position: [0.0, 0.0, 0.0],
        velocity: [0.0, 0.0, 0.0],
        direction: [0.0, 0.0, 0.0],
        coneOuterGain: 0.0,
        coneInnerAngle: 360.0,
        coneOuterAngle: 360.0,
        distanceModel: AL_INVERSE_DISTANCE_CLAMPED,
        spatialize: AL_AUTO_SOFT,

        get playbackRate() {
          return this.pitch * this.dopplerShift;
        }
      };
      AL.currentCtx.sources.set(src.id, src);
      {{{ makeSetValue('pSourceIds', 'i*4', 'src.id', 'i32') }}};
    }
  },

  alDeleteSources__proxy: 'sync',
  alDeleteSources: (count, pSourceIds) => {
    if (!AL.requireValidSources(count, pSourceIds, 'alDeleteSources')) {
      return;
    }

    for (var i = 0; i < count; ++i) {
      var srcId = {{{ makeGetValue('pSourceIds', 'i*4', 'i32') }}};
      AL.setSourceState(AL.currentCtx.sources.get(srcId), AL_STOPPED);
      _alSourcei(srcId, AL_BUFFER, 0);
      AL.currentCtx.sources.delete(srcId);
      AL.freeIds.push(srcId);
    }
  },

  // -------------------------------------------------------
  // --- AL Context State
  // -------------------------------------------------------

  alGetError__proxy: 'sync',
  alGetError: () => {
    if (!AL.currentCtx) {
      return AL_INVALID_OPERATION;
    }
    // Reset error on get.
    var err = AL.currentCtx.err;
    AL.currentCtx.err = AL_NO_ERROR;
    return err;
  },

  alIsExtensionPresent__proxy: 'sync',
  alIsExtensionPresent: (pExtName) => {
    var name = UTF8ToString(pExtName);

    return AL.AL_EXTENSIONS[name] ? AL_TRUE : AL_FALSE;
  },

  $alEnumValues__internal: true,
  // Note: using template literal here instead of plain object because jsify
  // serializes identifier keys w/o quotes and Closure would mangle them.
  $alEnumValues: `{
    // Spec doesn't clearly state that alGetEnumValue() is required to
    // support _only_ extension tokens.
    // We should probably follow OpenAL-Soft's example and support all
    // of the names we know.
    // See http://repo.or.cz/openal-soft.git/blob/HEAD:/Alc/ALc.c
    'AL_BITS': AL_BITS,
    'AL_BUFFER': AL_BUFFER,
    'AL_BUFFERS_PROCESSED': AL_BUFFERS_PROCESSED,
    'AL_BUFFERS_QUEUED': AL_BUFFERS_QUEUED,
    'AL_BYTE_OFFSET': AL_BYTE_OFFSET,
    'AL_CHANNELS': AL_CHANNELS,
    'AL_CONE_INNER_ANGLE': AL_CONE_INNER_ANGLE,
    'AL_CONE_OUTER_ANGLE': AL_CONE_OUTER_ANGLE,
    'AL_CONE_OUTER_GAIN': AL_CONE_OUTER_GAIN,
    'AL_DIRECTION': AL_DIRECTION,
    'AL_DISTANCE_MODEL': AL_DISTANCE_MODEL,
    'AL_DOPPLER_FACTOR': AL_DOPPLER_FACTOR,
    'AL_DOPPLER_VELOCITY': AL_DOPPLER_VELOCITY,
    'AL_EXPONENT_DISTANCE': AL_EXPONENT_DISTANCE,
    'AL_EXPONENT_DISTANCE_CLAMPED': AL_EXPONENT_DISTANCE_CLAMPED,
    'AL_EXTENSIONS': AL_EXTENSIONS,
    'AL_FORMAT_MONO16': AL_FORMAT_MONO16,
    'AL_FORMAT_MONO8': AL_FORMAT_MONO8,
    'AL_FORMAT_STEREO16': AL_FORMAT_STEREO16,
    'AL_FORMAT_STEREO8': AL_FORMAT_STEREO8,
    'AL_FREQUENCY': AL_FREQUENCY,
    'AL_GAIN': AL_GAIN,
    'AL_INITIAL': AL_INITIAL,
    'AL_INVALID': AL_INVALID,
    'AL_ILLEGAL_ENUM': AL_INVALID_ENUM,
    'AL_INVALID_ENUM': AL_INVALID_ENUM,
    'AL_INVALID_NAME': AL_INVALID_NAME,
    'AL_ILLEGAL_COMMAND': AL_INVALID_OPERATION,
    'AL_INVALID_OPERATION': AL_INVALID_OPERATION,
    'AL_INVALID_VALUE': AL_INVALID_VALUE,
    'AL_INVERSE_DISTANCE': AL_INVERSE_DISTANCE,
    'AL_INVERSE_DISTANCE_CLAMPED': AL_INVERSE_DISTANCE_CLAMPED,
    'AL_LINEAR_DISTANCE': AL_LINEAR_DISTANCE,
    'AL_LINEAR_DISTANCE_CLAMPED': AL_LINEAR_DISTANCE_CLAMPED,
    'AL_LOOPING': AL_LOOPING,
    'AL_MAX_DISTANCE': AL_MAX_DISTANCE,
    'AL_MAX_GAIN': AL_MAX_GAIN,
    'AL_MIN_GAIN': AL_MIN_GAIN,
    'AL_NONE': AL_NONE,
    'AL_NO_ERROR': AL_NO_ERROR,
    'AL_ORIENTATION': AL_ORIENTATION,
    'AL_OUT_OF_MEMORY': AL_OUT_OF_MEMORY,
    'AL_PAUSED': AL_PAUSED,
    'AL_PENDING': AL_PENDING,
    'AL_PITCH': AL_PITCH,
    'AL_PLAYING': AL_PLAYING,
    'AL_POSITION': AL_POSITION,
    'AL_PROCESSED': AL_PROCESSED,
    'AL_REFERENCE_DISTANCE': AL_REFERENCE_DISTANCE,
    'AL_RENDERER': AL_RENDERER,
    'AL_ROLLOFF_FACTOR': AL_ROLLOFF_FACTOR,
    'AL_SAMPLE_OFFSET': AL_SAMPLE_OFFSET,
    'AL_SEC_OFFSET': AL_SEC_OFFSET,
    'AL_SIZE': AL_SIZE,
    'AL_SOURCE_RELATIVE': AL_SOURCE_RELATIVE,
    'AL_SOURCE_STATE': AL_SOURCE_STATE,
    'AL_SOURCE_TYPE': AL_SOURCE_TYPE,
    'AL_SPEED_OF_SOUND': AL_SPEED_OF_SOUND,
    'AL_STATIC': AL_STATIC,
    'AL_STOPPED': AL_STOPPED,
    'AL_STREAMING': AL_STREAMING,
    'AL_UNDETERMINED': AL_UNDETERMINED,
    'AL_UNUSED': AL_UNUSED,
    'AL_VELOCITY': AL_VELOCITY,
    'AL_VENDOR': AL_VENDOR,
    'AL_VERSION': AL_VERSION,

    /* Extensions */
    'AL_AUTO_SOFT': AL_AUTO_SOFT,
    'AL_SOURCE_DISTANCE_MODEL': AL_SOURCE_DISTANCE_MODEL,
    'AL_SOURCE_SPATIALIZE_SOFT': AL_SOURCE_SPATIALIZE_SOFT,
    'AL_LOOP_POINTS_SOFT': AL_LOOP_POINTS_SOFT,
    'AL_BYTE_LENGTH_SOFT': 0x2009,
    'AL_SAMPLE_LENGTH_SOFT': 0x200A,
    'AL_SEC_LENGTH_SOFT': 0x200B,
    'AL_FORMAT_MONO_FLOAT32': AL_FORMAT_MONO_FLOAT32,
    'AL_FORMAT_STEREO_FLOAT32': AL_FORMAT_STEREO_FLOAT32,
  }`,

  alGetEnumValue__proxy: 'sync',
  alGetEnumValue: (pEnumName) => {
    if (!AL.requireValidPointer(pEnumName, 'alGetEnumValue')) {
      return 0;
    }
    var name = UTF8ToString(pEnumName);
    if (hasOwn(alEnumValues, name)) {
      return alEnumValues[name];
    }
#if OPENAL_DEBUG
    dbg(`No value for '${name}' is known by alGetEnumValue()`);
#endif
    AL.currentCtx.err = AL_INVALID_VALUE;
    return 0;
  },

  alGetString__proxy: 'sync',
  alGetString: (param) => {
    if (AL.stringCache[param]) {
      return AL.stringCache[param];
    }

    var ret;
    switch (param) {
    case AL_NO_ERROR:
      ret = 'No Error';
      break;
    case AL_INVALID_NAME:
      ret = 'Invalid Name';
      break;
    case AL_INVALID_ENUM:
      ret = 'Invalid Enum';
      break;
    case AL_INVALID_VALUE:
      ret = 'Invalid Value';
      break;
    case AL_INVALID_OPERATION:
      ret = 'Invalid Operation';
      break;
    case AL_OUT_OF_MEMORY:
      ret = 'Out of Memory';
      break;
    case AL_VENDOR:
      ret = 'Emscripten';
      break;
    case AL_VERSION:
      ret = '1.1';
      break;
    case AL_RENDERER:
      ret = 'WebAudio';
      break;
    case AL_EXTENSIONS:
      ret = Object.keys(AL.AL_EXTENSIONS).join(' ');
      break;
    default:
      if (AL.requireValidContext('alGetString')) {
        AL.currentCtx.err = AL_INVALID_ENUM;
      }
      return 0;
    }

    ret = stringToNewUTF8(ret);
    AL.stringCache[param] = ret;
    return ret;
  },

  alEnable__proxy: 'sync',
  alEnable: (param) => {
    if (!AL.requireValidContext('alEnable')) {
      return;
    }
    switch (param) {
    case AL_SOURCE_DISTANCE_MODEL:
      AL.currentCtx.sourceDistanceModel = true;
      AL.updateContextGlobal(AL.currentCtx);
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alEnable() with param ${ptrToString(param)} not implemented yet`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alDisable__proxy: 'sync',
  alDisable: (param) => {
    if (!AL.requireValidContext('alDisable')) {
      return;
    }
    switch (param) {
    case AL_SOURCE_DISTANCE_MODEL:
      AL.currentCtx.sourceDistanceModel = false;
      AL.updateContextGlobal(AL.currentCtx);
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alDisable() with param ${ptrToString(param)} not implemented yet`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alIsEnabled__proxy: 'sync',
  alIsEnabled: (param) => {
    if (!AL.requireValidContext('alIsEnabled')) {
      return AL_FALSE;
    }
    switch (param) {
    case AL_SOURCE_DISTANCE_MODEL:
      return AL.currentCtx.sourceDistanceModel ? AL_TRUE : AL_FALSE;
    default:
#if OPENAL_DEBUG
      dbg(`alIsEnabled() with param ${ptrToString(param)} not implemented yet`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return AL_FALSE;
    }
  },

  alGetDouble__proxy: 'sync',
  alGetDouble: (param) => AL.getGlobalParam(param, 'alGetDouble') || 0.0,

  alGetDoublev__proxy: 'sync',
  alGetDoublev: (param, pValues) => {
    var val = AL.getGlobalParam(param, 'alGetDoublev');
    // Silently ignore null destinations, as per the spec for global state functions
    if (val !== null && pValues) {
      {{{ makeSetValue('pValues', '0', 'val', 'double') }}};
    }
  },

  alGetFloat__proxy: 'sync',
  alGetFloat: (param) => AL.getGlobalParam(param, 'alGetFloat') || 0.0,

  alGetFloatv__proxy: 'sync',
  alGetFloatv: (param, pValues) => {
    var val = AL.getGlobalParam(param, 'alGetFloatv');
    // Silently ignore null destinations, as per the spec for global state functions
    if (val !== null && pValues) {
      {{{ makeSetValue('pValues', '0', 'val', 'float') }}};
    }
  },

  alGetInteger__proxy: 'sync',
  alGetInteger: (param) => AL.getGlobalParam(param, 'alGetInteger') || 0,

  alGetIntegerv__proxy: 'sync',
  alGetIntegerv: (param, pValues) => {
    var val = AL.getGlobalParam(param, 'alGetIntegerv');
    // Silently ignore null destinations, as per the spec for global state functions
    if (val !== null && pValues) {
      {{{ makeSetValue('pValues', '0', 'val', 'i32') }}};
    }
  },

  alGetBoolean__proxy: 'sync',
  alGetBoolean: (param) => AL.getGlobalParam(param, 'alGetBoolean') ? AL_TRUE : AL_FALSE,

  alGetBooleanv__proxy: 'sync',
  alGetBooleanv: (param, pValues) => {
    var val = AL.getGlobalParam(param, 'alGetBooleanv');
    // Silently ignore null destinations, as per the spec for global state functions
    if (val !== null && pValues) {
      {{{ makeSetValue('pValues', '0', 'val', 'i8') }}};
    }
  },

  alDistanceModel__proxy: 'sync',
  alDistanceModel: (model) => {
    AL.setGlobalParam(AL_DISTANCE_MODEL, model, 'alDistanceModel');
  },

  alSpeedOfSound__proxy: 'sync',
  alSpeedOfSound: (value) => {
    AL.setGlobalParam(AL_SPEED_OF_SOUND, value, 'alSpeedOfSound');
  },

  alDopplerFactor__proxy: 'sync',
  alDopplerFactor: (value) => {
    AL.setGlobalParam(AL_DOPPLER_FACTOR, value, 'alDopplerFactor');
  },

  // http://openal.996291.n3.nabble.com/alSpeedOfSound-or-alDopperVelocity-tp1960.html
  // alDopplerVelocity() sets a multiplier for the speed of sound.
  // It's deprecated since it's equivalent to directly calling
  // alSpeedOfSound() with an appropriately premultiplied value.
  alDopplerVelocity__proxy: 'sync',
  alDopplerVelocity: (value) => {
    warnOnce('alDopplerVelocity() is deprecated, and only kept for compatibility with OpenAL 1.0. Use alSpeedOfSound() instead.');
    if (!AL.requireValidContext('alDopplerVelocity')) {
      return;
    }
    if (value <= 0) { // Negative or zero values are disallowed
      AL.currentCtx.err = AL_INVALID_VALUE;
      return;
    }
  },

  // -------------------------------------------------------
  // -- AL Listener State
  // -------------------------------------------------------

  alGetListenerf__proxy: 'sync',
  alGetListenerf: (param, pValue) => {
    var val = AL.getListenerParam(param, 'alGetListenerf');
    if (val === null || !AL.requireValidPointer(pValue, 'alGetListenerf')) {
      return;
    }

    switch (param) {
    case AL_GAIN:
      {{{ makeSetValue('pValue', '0', 'val', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetListenerf(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetListener3f__proxy: 'sync',
  alGetListener3f: (param, pValue0, pValue1, pValue2) => {
    var val = AL.getListenerParam(param, 'alGetListener3f');
    if (val === null || !AL.requireValidPointer(pValue0 && pValue1 && pValue2, 'alGetListener3f')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetListener3f(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetListenerfv__proxy: 'sync',
  alGetListenerfv: (param, pValues) => {
    var val = AL.getListenerParam(param, 'alGetListenerfv');
    if (val === null || !AL.requireValidPointer(pValues, 'alGetListenerfv')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'float') }}};
      break;
    case AL_ORIENTATION:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'float') }}};
      {{{ makeSetValue('pValues', '12', 'val[3]', 'float') }}};
      {{{ makeSetValue('pValues', '16', 'val[4]', 'float') }}};
      {{{ makeSetValue('pValues', '20', 'val[5]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetListenerfv(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetListeneri__proxy: 'sync',
  alGetListeneri: (param, pValue) => {
    var val = AL.getListenerParam(param, 'alGetListeneri');
    if (val === null || !AL.requireValidPointer(pValue, 'alGetListeneri')) {
      return;
    }

#if OPENAL_DEBUG
    dbg(`alGetListeneri(): param ${ptrToString(param)} has wrong signature`);
#endif
    AL.currentCtx.err = AL_INVALID_ENUM;
  },

  alGetListener3i__proxy: 'sync',
  alGetListener3i: (param, pValue0, pValue1, pValue2) => {
    var val = AL.getListenerParam(param, 'alGetListener3i');
    if (val === null || !AL.requireValidPointer(pValue0 && pValue1 && pValue2, 'alGetListener3i')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetListener3i(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetListeneriv__proxy: 'sync',
  alGetListeneriv: (param, pValues) => {
    var val = AL.getListenerParam(param, 'alGetListeneriv');
    if (val === null || !AL.requireValidPointer(pValues, 'alGetListeneriv')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'i32') }}};
      break;
    case AL_ORIENTATION:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'i32') }}};
      {{{ makeSetValue('pValues', '12', 'val[3]', 'i32') }}};
      {{{ makeSetValue('pValues', '16', 'val[4]', 'i32') }}};
      {{{ makeSetValue('pValues', '20', 'val[5]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetListeneriv(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alListenerf__proxy: 'sync',
  alListenerf: (param, value) => {
    switch (param) {
    case AL_GAIN:
      AL.setListenerParam(param, value, 'alListenerf');
      break;
    default:
      AL.setListenerParam(param, null, 'alListenerf');
      break;
    }
  },

  alListener3f__proxy: 'sync',
  alListener3f: (param, value0, value1, value2) => {
    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setListenerParam(param, AL.paramArray, 'alListener3f');
      break;
    default:
      AL.setListenerParam(param, null, 'alListener3f');
      break;
    }
  },

  alListenerfv__proxy: 'sync',
  alListenerfv: (param, pValues) => {
    if (!AL.requireValidPointer(pValues, 'alListenerfv')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'float') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'float') }}};
      AL.setListenerParam(param, AL.paramArray, 'alListenerfv');
      break;
    case AL_ORIENTATION:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'float') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'float') }}};
      AL.paramArray[3] = {{{ makeGetValue('pValues', '12', 'float') }}};
      AL.paramArray[4] = {{{ makeGetValue('pValues', '16', 'float') }}};
      AL.paramArray[5] = {{{ makeGetValue('pValues', '20', 'float') }}};
      AL.setListenerParam(param, AL.paramArray, 'alListenerfv');
      break;
    default:
      AL.setListenerParam(param, null, 'alListenerfv');
      break;
    }
  },

  alListeneri__proxy: 'sync',
  alListeneri: (param, value) => {
    AL.setListenerParam(param, null, 'alListeneri');
  },

  alListener3i__proxy: 'sync',
  alListener3i: (param, value0, value1, value2) => {
    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setListenerParam(param, AL.paramArray, 'alListener3i');
      break;
    default:
      AL.setListenerParam(param, null, 'alListener3i');
      break;
    }
  },

  alListeneriv__proxy: 'sync',
  alListeneriv: (param, pValues) => {
    if (!AL.requireValidPointer(pValues, 'alListeneriv')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_VELOCITY:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'i32') }}};
      AL.setListenerParam(param, AL.paramArray, 'alListeneriv');
      break;
    case AL_ORIENTATION:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'i32') }}};
      AL.paramArray[3] = {{{ makeGetValue('pValues', '12', 'i32') }}};
      AL.paramArray[4] = {{{ makeGetValue('pValues', '16', 'i32') }}};
      AL.paramArray[5] = {{{ makeGetValue('pValues', '20', 'i32') }}};
      AL.setListenerParam(param, AL.paramArray, 'alListeneriv');
      break;
    default:
      AL.setListenerParam(param, null, 'alListeneriv');
      break;
    }
  },

  // -------------------------------------------------------
  // -- AL Buffer State
  // -------------------------------------------------------

  alIsBuffer__proxy: 'sync',
  alIsBuffer: (bufferId) => {
    if (!AL.currentCtx) {
      return false;
    }

    if (!AL.buffers[bufferId]) {
      return false;
    }
    return true;
  },

  alBufferData__proxy: 'sync',
  alBufferData: (bufferId, format, pData, size, freq) => {
    if (!AL.requireValidContext('alBufferData')) {
      return;
    }
    var buf = AL.buffers[bufferId];
    if (!buf) {
#if OPENAL_DEBUG
      dbg('alBufferData() called with an invalid buffer');
#endif
      AL.currentCtx.err = AL_INVALID_VALUE;
      return;
    }
    if (freq <= 0) {
#if OPENAL_DEBUG
      dbg('alBufferData() called with an invalid frequency');
#endif
      AL.currentCtx.err = AL_INVALID_VALUE;
      return;
    }

    var audioBuf = null;
    try {
      switch (format) {
      case AL_FORMAT_MONO8:
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
      case AL_FORMAT_MONO16:
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
      case AL_FORMAT_STEREO8:
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
      case AL_FORMAT_STEREO16:
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
      case AL_FORMAT_MONO_FLOAT32:
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
      case AL_FORMAT_STEREO_FLOAT32:
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
        dbg(`alBufferData() called with invalid format ${format}`);
#endif
        AL.currentCtx.err = AL_INVALID_VALUE;
        return;
      }
      buf.frequency = freq;
      buf.audioBuf = audioBuf;
    } catch (e) {
#if OPENAL_DEBUG
      dbg(`alBufferData() upload failed with an exception ${e}`);
#endif
      AL.currentCtx.err = AL_INVALID_VALUE;
      return;
    }
  },

  alGetBufferf__proxy: 'sync',
  alGetBufferf: (bufferId, param, pValue) => {
    var val = AL.getBufferParam(bufferId, param, 'alGetBufferf');
    if (val === null || !AL.requireValidPointer(pValue, 'alGetBufferf')) {
      return;
    }

#if OPENAL_DEBUG
    dbg(`alGetBufferf(): param ${ptrToString(param)} has wrong signature`);
#endif
    AL.currentCtx.err = AL_INVALID_ENUM;
  },

  alGetBuffer3f__proxy: 'sync',
  alGetBuffer3f: (bufferId, param, pValue0, pValue1, pValue2) => {
    var val = AL.getBufferParam(bufferId, param, 'alGetBuffer3f');
    if (val === null || !AL.requireValidPointer(pValue0 && pValue1 && pValue2, 'alGetBuffer3f')) {
      return;
    }

#if OPENAL_DEBUG
    dbg(`alGetBuffer3f(): param ${ptrToString(param)} has wrong signature`);
#endif
    AL.currentCtx.err = AL_INVALID_ENUM;
  },

  alGetBufferfv__proxy: 'sync',
  alGetBufferfv: (bufferId, param, pValues) => {
    var val = AL.getBufferParam(bufferId, param, 'alGetBufferfv');
    if (val === null || !AL.requireValidPointer(pValues, 'alGetBufferfv')) {
      return;
    }

#if OPENAL_DEBUG
    dbg(`alGetBufferfv(): param ${ptrToString(param)} has wrong signature`);
#endif
    AL.currentCtx.err = AL_INVALID_ENUM;
  },

  alGetBufferi__proxy: 'sync',
  alGetBufferi: (bufferId, param, pValue) => {
    var val = AL.getBufferParam(bufferId, param, 'alGetBufferi');
    if (val === null || !AL.requireValidPointer(pValue, 'alGetBufferi')) {
      return;
    }

    switch (param) {
    case AL_FREQUENCY:
    case AL_BITS:
    case AL_CHANNELS:
    case AL_SIZE:
      {{{ makeSetValue('pValue', '0', 'val', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetBufferi(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetBuffer3i__proxy: 'sync',
  alGetBuffer3i: (bufferId, param, pValue0, pValue1, pValue2) => {
    var val = AL.getBufferParam(bufferId, param, 'alGetBuffer3i');
    if (val === null || !AL.requireValidPointer(pValue0 && pValue1 && pValue2, 'alGetBuffer3i')) {
      return;
    }

#if OPENAL_DEBUG
    dbg(`alGetBuffer3i(): param ${ptrToString(param)} has wrong signature`);
#endif
    AL.currentCtx.err = AL_INVALID_ENUM;
  },

  alGetBufferiv__proxy: 'sync',
  alGetBufferiv: (bufferId, param, pValues) => {
    var val = AL.getBufferParam(bufferId, param, 'alGetBufferiv');
    if (val === null || !AL.requireValidPointer(pValues, 'alGetBufferiv')) {
      return;
    }

    switch (param) {
    case AL_FREQUENCY:
    case AL_BITS:
    case AL_CHANNELS:
    case AL_SIZE:
      {{{ makeSetValue('pValues', '0', 'val', 'i32') }}};
      break;
    case AL_LOOP_POINTS_SOFT:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetBufferiv(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  // All of the remaining alBuffer* setters and getters are only of interest
  // to extensions which need them. Core OpenAL alone defines no valid
  // property for these.

  alBufferf__proxy: 'sync',
  alBufferf: (bufferId, param, value) => {
    AL.setBufferParam(bufferId, param, null, 'alBufferf');
  },

  alBuffer3f__proxy: 'sync',
  alBuffer3f: (bufferId, param, value0, value1, value2) => {
    AL.setBufferParam(bufferId, param, null, 'alBuffer3f');
  },

  alBufferfv__proxy: 'sync',
  alBufferfv: (bufferId, param, pValues) => {
    if (!AL.requireValidPointer(pValues, 'alBufferfv')) {
      return;
    }

    AL.setBufferParam(bufferId, param, null, 'alBufferfv');
  },

  alBufferi__proxy: 'sync',
  alBufferi: (bufferId, param, value) => {
    AL.setBufferParam(bufferId, param, null, 'alBufferi');
  },

  alBuffer3i__proxy: 'sync',
  alBuffer3i: (bufferId, param, value0, value1, value2) => {
    AL.setBufferParam(bufferId, param, null, 'alBuffer3i');
  },

  alBufferiv__proxy: 'sync',
  alBufferiv: (bufferId, param, pValues) => {
    if (!AL.requireValidPointer(pValues, 'alBufferiv')) {
      return;
    }

    switch (param) {
    case AL_LOOP_POINTS_SOFT:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.setBufferParam(bufferId, param, AL.paramArray, 'alBufferiv');
      break;
    default:
      AL.setBufferParam(bufferId, param, null, 'alBufferiv');
      break;
    }
  },

  // -------------------------------------------------------
  // -- AL Source State
  // -------------------------------------------------------

  alIsSource__proxy: 'sync',
  alIsSource: (sourceId) => {
    if (!AL.currentCtx) {
      return false;
    }
    return AL.currentCtx.sources.has(sourceId);
  },

  alSourceQueueBuffers__proxy: 'sync',
  alSourceQueueBuffers: (sourceId, count, pBufferIds) => {
    var src = AL.requireValidSource(sourceId, 'alSourceQueueBuffers');
    if (!src) {
      return;
    }
    if (src.type === AL_STATIC) {
#if OPENAL_DEBUG
      dbg('alSourceQueueBuffers() called while a static buffer is bound');
#endif
      AL.currentCtx.err = AL_INVALID_OPERATION;
      return;
    }

    if (!count) {
      return;
    }

    // Find the first non-zero buffer in the queue to determine the proper format
    var templateBuf = AL.buffers[0];
    for (var buf of src.bufQueue) {
      if (buf.id) {
        templateBuf = buf;
        break;
      }
    }

    for (var i = 0; i < count; ++i) {
      var bufId = {{{ makeGetValue('pBufferIds', 'i*4', 'i32') }}};
      var buf = AL.buffers[bufId];
      if (!buf) {
#if OPENAL_DEBUG
        dbg('alSourceQueueBuffers() called with an invalid buffer');
#endif
        AL.currentCtx.err = AL_INVALID_NAME;
        return;
      }

      // Check that the added buffer has the correct format. If the template is the zero buffer, any format is valid.
      if (templateBuf.id && (
        buf.frequency !== templateBuf.frequency
        || buf.bytesPerSample !== templateBuf.bytesPerSample
        || buf.channels !== templateBuf.channels)
      ) {
#if OPENAL_DEBUG
        dbg('alSourceQueueBuffers() called with a buffer of different format');
#endif
        AL.currentCtx.err = AL_INVALID_OPERATION;
        return;
      }
    }

    // If the only buffer in the queue is the zero buffer, clear the queue before we add anything.
    if (src.bufQueue.length === 1 && !src.bufQueue[0].id) {
      src.bufQueue.length = 0;
    }

    src.type = AL_STREAMING;
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
  alSourceUnqueueBuffers: (sourceId, count, pBufferIds) => {
    var src = AL.requireValidSource(sourceId, 'alSourceUnqueueBuffers');
    if (!src) {
      return;
    }
    if (count > (src.bufQueue.length === 1 && !src.bufQueue[0].id ? 0 : src.bufsProcessed)) {
      AL.currentCtx.err = AL_INVALID_VALUE;
      return;
    }

    if (!count) {
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
    if (!src.bufQueue.length) {
      src.bufQueue.push(AL.buffers[0]);
    }

    AL.initSourcePanner(src);
    AL.scheduleSourceAudio(src);
  },

  alSourcePlay__proxy: 'sync',
  alSourcePlay: (sourceId) => {
    var src = AL.requireValidSource(sourceId, 'alSourcePlay');
    if (src) {
      AL.setSourceState(src, AL_PLAYING);
    }
  },

  alSourcePlayv__proxy: 'sync',
  alSourcePlayv: (count, pSourceIds) => {
    AL.setSourceStatev(count, pSourceIds, AL_PLAYING, 'alSourcePlayv');
  },

  alSourceStop__proxy: 'sync',
  alSourceStop: (sourceId) => {
    var src = AL.requireValidSource(sourceId, 'alSourceStop');
    if (src) {
      AL.setSourceState(src, AL_STOPPED);
    }
  },

  alSourceStopv__proxy: 'sync',
  alSourceStopv: (count, pSourceIds) => {
    AL.setSourceStatev(count, pSourceIds, AL_STOPPED, 'alSourceStopv');
  },

  alSourceRewind__proxy: 'sync',
  alSourceRewind: (sourceId) => {
    var src = AL.requireValidSource(sourceId, 'alSourceRewind');
    if (src) {
      // Stop the source first to clear the source queue
      AL.setSourceState(src, AL_STOPPED);
      // Now set the state of AL_INITIAL according to the specification
      AL.setSourceState(src, AL_INITIAL);
    }
  },

  alSourceRewindv__proxy: 'sync',
  alSourceRewindv: (count, pSourceIds) => {
    AL.setSourceStatev(count, pSourceIds, AL_INITIAL, 'alSourceRewindv');
  },

  alSourcePause__proxy: 'sync',
  alSourcePause: (sourceId) => {
    var src = AL.requireValidSource(sourceId, 'alSourcePause');
    if (src) {
      AL.setSourceState(src, AL_PAUSED);
    }
  },

  alSourcePausev__proxy: 'sync',
  alSourcePausev: (count, pSourceIds) => {
    AL.setSourceStatev(count, pSourceIds, AL_PAUSED, 'alSourcePausev');
  },

  alGetSourcef__proxy: 'sync',
  alGetSourcef: (sourceId, param, pValue) => {
    var val = AL.getSourceParam(sourceId, param, 'alGetSourcef');
    if (val === null || !AL.requireValidPointer(pValue, 'alGetSourcef')) {
      return;
    }

    switch (param) {
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_PITCH:
    case AL_GAIN:
    case AL_MIN_GAIN:
    case AL_MAX_GAIN:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_CONE_OUTER_GAIN:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      {{{ makeSetValue('pValue', '0', 'val', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetSourcef(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetSource3f__proxy: 'sync',
  alGetSource3f: (sourceId, param, pValue0, pValue1, pValue2) => {
    var val = AL.getSourceParam(sourceId, param, 'alGetSource3f');
    if (val === null || !AL.requireValidPointer(pValue0 && pValue1 && pValue2, 'alGetSource3f')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetSource3f(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetSourcefv__proxy: 'sync',
  alGetSourcefv: (sourceId, param, pValues) => {
    var val = AL.getSourceParam(sourceId, param, 'alGetSourcefv');
    if (val === null || !AL.requireValidPointer(pValues, 'alGetSourcefv')) {
      return;
    }

    switch (param) {
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_PITCH:
    case AL_GAIN:
    case AL_MIN_GAIN:
    case AL_MAX_GAIN:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_CONE_OUTER_GAIN:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      {{{ makeSetValue('pValues', '0', 'val', 'float') }}};
      break;
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'float') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'float') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'float') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetSourcefv(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetSourcei__proxy: 'sync',
  alGetSourcei: (sourceId, param, pValue) => {
    var val = AL.getSourceParam(sourceId, param, 'alGetSourcei');
    if (val === null || !AL.requireValidPointer(pValue, 'alGetSourcei')) {
      return;
    }

    switch (param) {
    case AL_SOURCE_RELATIVE:
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_LOOPING:
    case AL_BUFFER:
    case AL_SOURCE_STATE:
    case AL_BUFFERS_QUEUED:
    case AL_BUFFERS_PROCESSED:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case AL_SOURCE_TYPE:
    case AL_SOURCE_SPATIALIZE_SOFT:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case AL_DISTANCE_MODEL:
      {{{ makeSetValue('pValue', '0', 'val', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetSourcei(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetSource3i__proxy: 'sync',
  alGetSource3i: (sourceId, param, pValue0, pValue1, pValue2) => {
    var val = AL.getSourceParam(sourceId, param, 'alGetSource3i');
    if (val === null || !AL.requireValidPointer(pValue0 && pValue1 && pValue2, 'alGetSource3i')) {
      return;
    }

    switch (param) {
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValue0', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValue1', '0', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValue2', '0', 'val[2]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetSource3i(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alGetSourceiv__proxy: 'sync',
  alGetSourceiv: (sourceId, param, pValues) => {
    var val = AL.getSourceParam(sourceId, param, 'alGetSourceiv');
    if (val === null || !AL.requireValidPointer(pValues, 'alGetSourceiv')) {
      return;
    }

    switch (param) {
    case AL_SOURCE_RELATIVE:
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_LOOPING:
    case AL_BUFFER:
    case AL_SOURCE_STATE:
    case AL_BUFFERS_QUEUED:
    case AL_BUFFERS_PROCESSED:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case AL_SOURCE_TYPE:
    case AL_SOURCE_SPATIALIZE_SOFT:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case AL_DISTANCE_MODEL:
      {{{ makeSetValue('pValues', '0', 'val', 'i32') }}};
      break;
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      {{{ makeSetValue('pValues', '0', 'val[0]', 'i32') }}};
      {{{ makeSetValue('pValues', '4', 'val[1]', 'i32') }}};
      {{{ makeSetValue('pValues', '8', 'val[2]', 'i32') }}};
      break;
    default:
#if OPENAL_DEBUG
      dbg(`alGetSourceiv(): param ${ptrToString(param)} has wrong signature`);
#endif
      AL.currentCtx.err = AL_INVALID_ENUM;
      return;
    }
  },

  alSourcef__proxy: 'sync',
  alSourcef: (sourceId, param, value) => {
    switch (param) {
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_PITCH:
    case AL_GAIN:
    case AL_MIN_GAIN:
    case AL_MAX_GAIN:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_CONE_OUTER_GAIN:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      AL.setSourceParam(sourceId, param, value, 'alSourcef');
      break;
    default:
      AL.setSourceParam(sourceId, param, null, 'alSourcef');
      break;
    }
  },

  alSource3f__proxy: 'sync',
  alSource3f: (sourceId, param, value0, value1, value2) => {
    switch (param) {
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setSourceParam(sourceId, param, AL.paramArray, 'alSource3f');
      break;
    default:
      AL.setSourceParam(sourceId, param, null, 'alSource3f');
      break;
    }
  },

  alSourcefv__proxy: 'sync',
  alSourcefv: (sourceId, param, pValues) => {
    if (!AL.requireValidPointer(pValues, 'alSourcefv')) {
      return;
    }

    switch (param) {
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_PITCH:
    case AL_GAIN:
    case AL_MIN_GAIN:
    case AL_MAX_GAIN:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_CONE_OUTER_GAIN:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case 0x200B /* AL_SEC_LENGTH_SOFT */:
      var val = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.setSourceParam(sourceId, param, val, 'alSourcefv');
      break;
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'float') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'float') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'float') }}};
      AL.setSourceParam(sourceId, param, AL.paramArray, 'alSourcefv');
      break;
    default:
      AL.setSourceParam(sourceId, param, null, 'alSourcefv');
      break;
    }
  },

  alSourcei__proxy: 'sync',
  alSourcei: (sourceId, param, value) => {
    switch (param) {
    case AL_SOURCE_RELATIVE:
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_LOOPING:
    case AL_BUFFER:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case AL_SOURCE_SPATIALIZE_SOFT:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case AL_DISTANCE_MODEL:
      AL.setSourceParam(sourceId, param, value, 'alSourcei');
      break;
    default:
      AL.setSourceParam(sourceId, param, null, 'alSourcei');
      break;
    }
  },

  alSource3i__proxy: 'sync',
  alSource3i: (sourceId, param, value0, value1, value2) => {
    switch (param) {
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      AL.paramArray[0] = value0;
      AL.paramArray[1] = value1;
      AL.paramArray[2] = value2;
      AL.setSourceParam(sourceId, param, AL.paramArray, 'alSource3i');
      break;
    default:
      AL.setSourceParam(sourceId, param, null, 'alSource3i');
      break;
    }
  },

  alSourceiv__proxy: 'sync',
  alSourceiv: (sourceId, param, pValues) => {
    if (!AL.requireValidPointer(pValues, 'alSourceiv')) {
      return;
    }

    switch (param) {
    case AL_SOURCE_RELATIVE:
    case AL_CONE_INNER_ANGLE:
    case AL_CONE_OUTER_ANGLE:
    case AL_LOOPING:
    case AL_BUFFER:
    case AL_REFERENCE_DISTANCE:
    case AL_ROLLOFF_FACTOR:
    case AL_MAX_DISTANCE:
    case AL_SEC_OFFSET:
    case AL_SAMPLE_OFFSET:
    case AL_BYTE_OFFSET:
    case AL_SOURCE_SPATIALIZE_SOFT:
    case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
    case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
    case AL_DISTANCE_MODEL:
      var val = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.setSourceParam(sourceId, param, val, 'alSourceiv');
      break;
    case AL_POSITION:
    case AL_DIRECTION:
    case AL_VELOCITY:
      AL.paramArray[0] = {{{ makeGetValue('pValues', '0', 'i32') }}};
      AL.paramArray[1] = {{{ makeGetValue('pValues', '4', 'i32') }}};
      AL.paramArray[2] = {{{ makeGetValue('pValues', '8', 'i32') }}};
      AL.setSourceParam(sourceId, param, AL.paramArray, 'alSourceiv');
      break;
    default:
      AL.setSourceParam(sourceId, param, null, 'alSourceiv');
      break;
    }
  }
};

addToLibrary(LibraryOpenAL);
