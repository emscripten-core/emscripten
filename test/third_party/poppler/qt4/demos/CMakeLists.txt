add_definitions(${QT4_DEFINITIONS})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QT4_INCLUDE_DIR}
)

set(poppler_qt4viewer_SRCS
  abstractinfodock.cpp
  documentobserver.cpp
  embeddedfiles.cpp
  fonts.cpp
  info.cpp
  main_viewer.cpp
  metadata.cpp
  navigationtoolbar.cpp
  optcontent.cpp
  pageview.cpp
  permissions.cpp
  thumbnails.cpp
  toc.cpp
  viewer.cpp
)
qt4_automoc(${poppler_qt4viewer_SRCS})
poppler_add_test(poppler_qt4viewer BUILD_QT4_TESTS ${poppler_qt4viewer_SRCS})
target_link_libraries(poppler_qt4viewer poppler-qt4)
