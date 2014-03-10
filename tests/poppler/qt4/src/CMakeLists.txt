add_definitions(${QT4_DEFINITIONS})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QT4_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(poppler_qt4_SRCS
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
  ${CMAKE_SOURCE_DIR}/poppler/ArthurOutputDev.cc
)
qt4_automoc(${poppler_qt4_SRCS})
add_library(poppler-qt4 SHARED ${poppler_qt4_SRCS})
set_target_properties(poppler-qt4 PROPERTIES VERSION 3.4.0 SOVERSION 3)
target_link_libraries(poppler-qt4 poppler ${QT4_QTCORE_LIBRARY} ${QT4_QTGUI_LIBRARY} ${QT4_QTXML_LIBRARY})
if(MSVC)
target_link_libraries(poppler-qt4 poppler ${poppler_LIBS})
endif(MSVC)
install(TARGETS poppler-qt4 RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

install(FILES
  poppler-qt4.h
  poppler-link.h
  poppler-annotation.h
  poppler-form.h
  poppler-optcontent.h
  poppler-export.h
  poppler-page-transition.h
  DESTINATION include/poppler/qt4)

