add_definitions(${QT5_DEFINITIONS})
add_definitions(${Qt5Core_DEFINITIONS})

set(CMAKE_AUTOMOC ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(poppler_qt5_SRCS
  poppler-annotation.cc
  poppler-document.cc
  poppler-embeddedfile.cc
  poppler-fontinfo.cc
  poppler-form.cc
  poppler-link.cc
  poppler-link-extractor.cc
  poppler-movie.cc
  poppler-optcontent.cc
  poppler-page.cc
  poppler-base-converter.cc
  poppler-pdf-converter.cc
  poppler-private.cc
  poppler-ps-converter.cc
  poppler-qiodeviceoutstream.cc
  poppler-sound.cc
  poppler-textbox.cc
  poppler-page-transition.cc
  poppler-media.cc
  ArthurOutputDev.cc
)
add_library(poppler-qt5 SHARED ${poppler_qt5_SRCS})
set_target_properties(poppler-qt5 PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(poppler-qt5 poppler ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Xml_LIBRARIES})
if(MSVC)
target_link_libraries(poppler-qt5 poppler ${poppler_LIBS})
endif(MSVC)
install(TARGETS poppler-qt5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

install(FILES
  poppler-qt5.h
  poppler-link.h
  poppler-annotation.h
  poppler-form.h
  poppler-optcontent.h
  poppler-export.h
  poppler-page-transition.h
  poppler-media.h
  DESTINATION include/poppler/qt5)

