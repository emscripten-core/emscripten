/* N.B. This file <GLES3/gl2ext.h> should not exist, see here:
   https://www.khronos.org/registry/gles/ : "OpenGL ES Extension Header File (this header is defined
   to contain all defined extension interfaces for OpenGL ES 2.0 and all later versions, since later
   versions are backwards-compatible with OpenGL ES 2.0)."

   However, we do provide this file for compatibility, since some other platforms seem to have
   it, and there's existing code in the wild that do #include <GLES3/gl2ext.h>.
   Please #include <GLES3/gl2ext.h> instead. */
#include "../GLES2/gl2ext.h"
