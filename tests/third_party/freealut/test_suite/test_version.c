#include <stdlib.h>
#include <stdio.h>
#include <AL/alut.h>

/*
  This program checks that the version of OpenAL in the
  library agrees with the header file we're compiled against.
*/

int main(int argc, char **argv)
{
  ALboolean ok = AL_FALSE;

  alutInit(&argc, argv);

#ifdef ALUT_API_MAJOR_VERSION
  if (alutGetMajorVersion() != ALUT_API_MAJOR_VERSION || alutGetMinorVersion() != ALUT_API_MINOR_VERSION)
  {
    fprintf(stderr,
            "WARNING: The ALUT library is version %d.%d.x but <AL/alut.h> says it's %d.%d.x!\n",
            alutGetMajorVersion(), alutGetMinorVersion(), ALUT_API_MAJOR_VERSION, ALUT_API_MINOR_VERSION);
  }
  else
  {
    fprintf(stderr, "The ALUT library is at version %d.%d.x.\n", alutGetMajorVersion(), alutGetMinorVersion());
    ok = AL_TRUE;
  }
#else
  fprintf(stderr, "WARNING: Your copy of <AL/alut.h> is pre-1.0.0,\n");
  fprintf(stderr, "but you are running the ALUT test suite from ALUT\n");
  fprintf(stderr, "version 1.0.0 or later.\n");
#endif

  alutExit();
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
