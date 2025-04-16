#
# FreeType 2 Type1 module definition
#


# Copyright 1996-2000, 2006 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


FTMODULE_H_COMMANDS += TYPE1_DRIVER

define TYPE1_DRIVER
$(OPEN_DRIVER) FT_Driver_ClassRec, t1_driver_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)type1     $(ECHO_DRIVER_DESC)Postscript font files with extension *.pfa or *.pfb$(ECHO_DRIVER_DONE)
endef

# EOF
