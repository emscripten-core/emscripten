if(NOT TARGET SDL3::SDL3)
  add_library(SDL3::SDL3-static INTERFACE IMPORTED)
  set_target_properties(SDL3::SDL3-static PROPERTIES
    INTERFACE_COMPILE_OPTIONS "-sUSE_SDL=3"
    INTERFACE_LINK_LIBRARIES "-sUSE_SDL=3"
  )
  add_library(SDL3::SDL3 ALIAS SDL3::SDL3-static)
  add_library(SDL3::Headers INTERFACE IMPORTED)
endif()
