# - Try to find Lv2
# Once done this will define
#
#  LV2_FOUND - system has Lv2
#  LV2_INCLUDE_DIRS - the Lv2 include directory
#  LV2_LIBRARIES - Link these to use Lv2
#  LV2_DEFINITIONS - Compiler switches required for using Lv2
#
#  Copyright (c) 2010 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (NOT LV2_INCLUDE_DIRS)

  find_path(LV2_INCLUDE_DIR
      NAMES lv2.h
  )

  set(LV2_INCLUDE_DIRS
      ${LV2_INCLUDE_DIR}
  )

  # show the LV2_INCLUDE_DIRS variables only in the advanced view
  mark_as_advanced(LV2_INCLUDE_DIRS)

endif (NOT LV2_INCLUDE_DIRS)

if (LV2_INCLUDE_DIRS)
  # in cache already
  set(LV2_FOUND TRUE)
endif (LV2_INCLUDE_DIRS)

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
