include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(poppler-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/poppler-version.h @ONLY)

add_subdirectory(tests)

set(poppler_cpp_SRCS
  poppler-document.cpp
  poppler-embedded-file.cpp
  poppler-font.cpp
  poppler-global.cpp
  poppler-image.cpp
  poppler-page.cpp
  poppler-page-renderer.cpp
  poppler-page-transition.cpp
  poppler-private.cpp
  poppler-rectangle.cpp
  poppler-toc.cpp
  poppler-version.cpp
)

add_library(poppler-cpp SHARED ${poppler_cpp_SRCS})
set_target_properties(poppler-cpp PROPERTIES VERSION 0.2.0 SOVERSION 0)
target_link_libraries(poppler-cpp poppler ${ICONV_LIBRARIES})
if(MSVC)
target_link_libraries(poppler-cpp ${poppler_LIBS})
endif(MSVC)
target_link_libraries(poppler-cpp LINK_INTERFACE_LIBRARIES "")
install(TARGETS poppler-cpp RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

install(FILES
  poppler-document.h
  poppler-embedded-file.h
  poppler-font.h
  poppler-global.h
  poppler-image.h
  poppler-page.h
  poppler-page-renderer.h
  poppler-page-transition.h
  poppler-rectangle.h
  poppler-toc.h
  ${CMAKE_CURRENT_BINARY_DIR}/poppler-version.h
  DESTINATION include/poppler/cpp)

