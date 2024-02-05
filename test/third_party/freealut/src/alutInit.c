#include "alutInternal.h"

static enum
{
  Unintialized,                 /* ALUT has not been initialized yet or has been de-initialised */
  ALUTDeviceAndContext,         /* alutInit has been called successfully */
  ExternalDeviceAndContext      /* alutInitWithoutContext has been called */
} initialisationState = Unintialized;

/*
 * Note: alutContext contains something valid only when initialisationState
 * contains ALUTDeviceAndContext.
 */
static ALCcontext *alutContext;

ALboolean _alutSanityCheck(void)
{
  ALCcontext *context;

  if (initialisationState == Unintialized)
  {
    _alutSetError(ALUT_ERROR_INVALID_OPERATION);
    return AL_FALSE;
  }

  context = alcGetCurrentContext();
  if (context == NULL)
  {
    _alutSetError(ALUT_ERROR_NO_CURRENT_CONTEXT);
    return AL_FALSE;
  }

  if (alGetError() != AL_NO_ERROR)
  {
    _alutSetError(ALUT_ERROR_AL_ERROR_ON_ENTRY);
    return AL_FALSE;
  }

  if (alcGetError(alcGetContextsDevice(context)) != ALC_NO_ERROR)
  {
    _alutSetError(ALUT_ERROR_ALC_ERROR_ON_ENTRY);
    return AL_FALSE;
  }

  return AL_TRUE;
}

ALboolean alutInit(int *argcp, char **argv)
{
  ALCdevice *device;
  ALCcontext *context;

  if (initialisationState != Unintialized)
  {
    _alutSetError(ALUT_ERROR_INVALID_OPERATION);
    return AL_FALSE;
  }

  if ((argcp == NULL) != (argv == NULL))
  {
    _alutSetError(ALUT_ERROR_INVALID_VALUE);
    return AL_FALSE;
  }

  device = alcOpenDevice(NULL);
  if (device == NULL)
  {
    _alutSetError(ALUT_ERROR_OPEN_DEVICE);
    return AL_FALSE;
  }

  context = alcCreateContext(device, NULL);
  if (context == NULL)
  {
    alcCloseDevice(device);
    _alutSetError(ALUT_ERROR_CREATE_CONTEXT);
    return AL_FALSE;
  }

  if (!alcMakeContextCurrent(context))
  {
    alcDestroyContext(context);
    alcCloseDevice(device);
    _alutSetError(ALUT_ERROR_MAKE_CONTEXT_CURRENT);
    return AL_FALSE;
  }

  initialisationState = ALUTDeviceAndContext;
  alutContext = context;
  return AL_TRUE;
}

ALboolean alutInitWithoutContext(int *argcp, char **argv)
{
  if (initialisationState != Unintialized)
  {
    _alutSetError(ALUT_ERROR_INVALID_OPERATION);
    return AL_FALSE;
  }

  if ((argcp == NULL) != (argv == NULL))
  {
    _alutSetError(ALUT_ERROR_INVALID_VALUE);
    return AL_FALSE;
  }

  initialisationState = ExternalDeviceAndContext;
  return AL_TRUE;
}

ALboolean alutExit(void)
{
  ALCdevice *device;

  if (initialisationState == Unintialized)
  {
    _alutSetError(ALUT_ERROR_INVALID_OPERATION);
    return AL_FALSE;
  }

  if (initialisationState == ExternalDeviceAndContext)
  {
    initialisationState = Unintialized;
    return AL_TRUE;
  }

  if (!_alutSanityCheck())
  {
    return AL_FALSE;
  }

  if (!alcMakeContextCurrent(NULL))
  {
    _alutSetError(ALUT_ERROR_MAKE_CONTEXT_CURRENT);
    return AL_FALSE;
  }

  device = alcGetContextsDevice(alutContext);
  alcDestroyContext(alutContext);
  if (alcGetError(device) != ALC_NO_ERROR)
  {
    _alutSetError(ALUT_ERROR_DESTROY_CONTEXT);
    return AL_FALSE;
  }

  if (!alcCloseDevice(device))
  {
    _alutSetError(ALUT_ERROR_CLOSE_DEVICE);
    return AL_FALSE;
  }

  initialisationState = Unintialized;
  return AL_TRUE;
}
