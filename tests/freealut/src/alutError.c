#include "alutInternal.h"
#include <stdio.h>

static ALenum lastError = ALUT_ERROR_NO_ERROR;

void _alutSetError(ALenum err)
{
  /* print a message to stderr if ALUT_DEBUG environment variable is defined */
  if (getenv("ALUT_DEBUG"))
  {
    fprintf(stderr, "ALUT error: %s\n", alutGetErrorString(err));
  }

  if (lastError == ALUT_ERROR_NO_ERROR)
  {
    lastError = err;
  }
}

ALenum alutGetError(void)
{
  ALint ret = lastError;

  lastError = ALUT_ERROR_NO_ERROR;
  return ret;
}

const char *alutGetErrorString(ALenum error)
{
  switch (error)
  {
  case ALUT_ERROR_NO_ERROR:
    return "No ALUT error found";

  case ALUT_ERROR_OUT_OF_MEMORY:
    return "ALUT ran out of memory";

  case ALUT_ERROR_INVALID_ENUM:
    return "ALUT was given an invalid enumeration token";

  case ALUT_ERROR_INVALID_VALUE:
    return "ALUT was given an invalid value";

  case ALUT_ERROR_INVALID_OPERATION:
    return "The operation was invalid in the current ALUT state";

  case ALUT_ERROR_NO_CURRENT_CONTEXT:
    return "There is no current AL context";

  case ALUT_ERROR_AL_ERROR_ON_ENTRY:
    return "There was already an AL error on entry to an ALUT function";

  case ALUT_ERROR_ALC_ERROR_ON_ENTRY:
    return "There was already an ALC error on entry to an ALUT function";

  case ALUT_ERROR_OPEN_DEVICE:
    return "There was an error opening the ALC device";

  case ALUT_ERROR_CLOSE_DEVICE:
    return "There was an error closing the ALC device";

  case ALUT_ERROR_CREATE_CONTEXT:
    return "There was an error creating an ALC context";

  case ALUT_ERROR_MAKE_CONTEXT_CURRENT:
    return "Could not change the current ALC context";

  case ALUT_ERROR_DESTROY_CONTEXT:
    return "There was an error destroying the ALC context";

  case ALUT_ERROR_GEN_BUFFERS:
    return "There was an error generating an AL buffer";

  case ALUT_ERROR_BUFFER_DATA:
    return "There was an error passing buffer data to AL";

  case ALUT_ERROR_IO_ERROR:
    return "I/O error";

  case ALUT_ERROR_UNSUPPORTED_FILE_TYPE:
    return "Unsupported file type";

  case ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE:
    return "Unsupported mode within an otherwise usable file type";

  case ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA:
    return "The sound data was corrupt or truncated";

  default:
    return "An impossible ALUT error condition was reported?!?";
  }
}
