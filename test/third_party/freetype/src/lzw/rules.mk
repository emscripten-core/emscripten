#
# FreeType 2 LZW support configuration rules
#


# Copyright 2004, 2005, 2006 by
# Albert Chin-A-Young.
#
# Based on src/lzw/rules.mk, Copyright 2002 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# LZW driver directory
#
LZW_DIR := $(SRC_DIR)/lzw


# compilation flags for the driver
#
LZW_COMPILE := $(FT_COMPILE) $I$(subst /,$(COMPILER_SEP),$(LZW_DIR))


# LZW support sources (i.e., C files)
#
LZW_DRV_SRC := $(LZW_DIR)/ftlzw.c

# LZW support headers
#
LZW_DRV_H := $(LZW_DIR)/ftzopen.h \
             $(LZW_DIR)/ftzopen.c


# LZW driver object(s)
#
#   LZW_DRV_OBJ_M is used during `multi' builds
#   LZW_DRV_OBJ_S is used during `single' builds
#
LZW_DRV_OBJ_M := $(OBJ_DIR)/ftlzw.$O
LZW_DRV_OBJ_S := $(OBJ_DIR)/ftlzw.$O

# LZW support source file for single build
#
LZW_DRV_SRC_S := $(LZW_DIR)/ftlzw.c


# LZW support - single object
#
$(LZW_DRV_OBJ_S): $(LZW_DRV_SRC_S) $(LZW_DRV_SRC) $(FREETYPE_H) $(LZW_DRV_H)
	$(LZW_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(LZW_DRV_SRC_S))


# LZW support - multiple objects
#
$(OBJ_DIR)/%.$O: $(LZW_DIR)/%.c $(FREETYPE_H) $(LZW_DRV_H)
	$(LZW_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(LZW_DRV_OBJ_S)
DRV_OBJS_M += $(LZW_DRV_OBJ_M)


# EOF
