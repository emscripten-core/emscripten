include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GLIB2_INCLUDE_DIRS}
  ${CAIRO_INCLUDE_DIRS}
)
add_definitions(
  -DG_LOG_DOMAIN=\"Poppler\"
  ${GLIB2_CFLAGS_OTHER}
  ${CAIRO_CFLAGS}
  ${POPPLER_GLIB_DISABLE_DEPRECATED}
  ${POPPLER_GLIB_DISABLE_SINGLE_INCLUDES}
)

if (GDK_FOUND)
  include_directories(${GDK2_INCLUDE_DIRS})
  add_definitions(${GDK2_CFLAGS_OTHER})
endif (GDK_FOUND)

configure_file(poppler-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/poppler-features.h @ONLY)

if (GTK_FOUND AND BUILD_GTK_TESTS)
  add_subdirectory(demo)
endif (GTK_FOUND AND BUILD_GTK_TESTS)

set(poppler_glib_public_headers
  poppler-action.h
  poppler-date.h
  poppler-document.h
  poppler-page.h
  poppler-attachment.h
  poppler-form-field.h
  poppler-annot.h
  poppler-layer.h
  poppler-movie.h
  poppler-media.h
  poppler.h
)

find_program(GLIB2_MKENUMS glib-mkenums)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.h
  COMMAND ${GLIB2_MKENUMS}
  ARGS
    --template poppler-enums.h.template
    ${poppler_glib_public_headers} > ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${poppler_glib_public_headers}
          ${CMAKE_CURRENT_SOURCE_DIR}/poppler-enums.h.template
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.c
  COMMAND ${GLIB2_MKENUMS}
  ARGS
    --template poppler-enums.c.template
    ${poppler_glib_public_headers} > ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${poppler_glib_public_headers} ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.h
          ${CMAKE_CURRENT_SOURCE_DIR}/poppler-enums.c.template
)


set(poppler_glib_SRCS
  poppler-action.cc
  poppler-date.cc
  poppler-document.cc
  poppler-page.cc
  poppler-attachment.cc
  poppler-form-field.cc
  poppler-annot.cc
  poppler-layer.cc
  poppler-movie.cc
  poppler-media.cc
  poppler.cc
)
set(poppler_glib_generated_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.c
  ${CMAKE_SOURCE_DIR}/poppler/CairoFontEngine.cc
  ${CMAKE_SOURCE_DIR}/poppler/CairoOutputDev.cc
  ${CMAKE_SOURCE_DIR}/poppler/CairoRescaleBox.cc
)
add_library(poppler-glib SHARED ${poppler_glib_SRCS} ${poppler_glib_generated_SRCS})
set_target_properties(poppler-glib PROPERTIES VERSION 6.0.0 SOVERSION 6)
target_link_libraries(poppler-glib poppler ${GLIB2_LIBRARIES} ${CAIRO_LIBRARIES} ${FREETYPE_LIBRARIES})
if (GDK_FOUND)
  target_link_libraries(poppler-glib ${GDK2_LIBRARIES})
endif (GDK_FOUND)
install(TARGETS poppler-glib RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

install(FILES
  ${poppler_glib_public_headers}
  ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.h
  ${CMAKE_CURRENT_BINARY_DIR}/poppler-features.h
  DESTINATION include/poppler/glib)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.h ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.c"
)


if (GDK_FOUND)
  set(test_poppler_glib_SRCS
    test-poppler-glib.cc
  )
  poppler_add_test(test-poppler-glib BUILD_GTK_TESTS ${test_poppler_glib_SRCS})
  target_link_libraries(test-poppler-glib poppler-glib ${GDK2_LIBRARIES})
endif (GDK_FOUND)

macro(_list_prefix _outvar _listvar _prefix)
  set(${_outvar})
  foreach(_item IN LISTS ${_listvar})
    list(APPEND ${_outvar} ${_prefix}${_item})
  endforeach()
endmacro(_list_prefix)

# GObject Introspection
if (HAVE_INTROSPECTION)
  include(GObjectIntrospectionMacros)
  set(INTROSPECTION_GIRS)
  set(INTROSPECTION_SCANNER_ARGS "--add-include-path=${CMAKE_CURRENT_SOURCE_DIR}")
  set(INTROSPECTION_COMPILER_ARGS "--includedir=${CMAKE_CURRENT_SOURCE_DIR}")

  set(introspection_files ${poppler_glib_SRCS} ${poppler_glib_public_headers} poppler-private.h)
  set(Poppler_0_16_gir "poppler-glib")
  set(Poppler_0_16_gir_INCLUDES GObject-2.0 cairo-1.0)
  get_directory_property(_tmp_includes INCLUDE_DIRECTORIES)
  _list_prefix(_includes _tmp_includes "-I")
  set(Poppler_0_16_gir_CFLAGS ${_includes})
  set(Poppler_0_16_gir_LIBS poppler-glib)
  _list_prefix(_abs_introspection_files introspection_files "${CMAKE_CURRENT_SOURCE_DIR}/")
  list(APPEND _abs_introspection_files
    ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.c
    ${CMAKE_CURRENT_BINARY_DIR}/poppler-enums.h
  )
  set(Poppler_0_16_gir_FILES ${_abs_introspection_files})

  if (GDK_FOUND)
    list(APPEND Poppler_0_16_gir_INCLUDES Gdk-2.0)
  endif ()

  list(APPEND INTROSPECTION_GIRS Poppler-0.16.gir)

  gir_add_introspections(INTROSPECTION_GIRS)
endif ()
