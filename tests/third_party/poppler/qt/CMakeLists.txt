add_definitions(${QT3_DEFINITIONS})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QT3_INCLUDE_DIR}
)

set(poppler_qt_SRCS
  poppler-document.cc
  poppler-fontinfo.cc
  poppler-link.cc
  poppler-page.cc
  poppler-page-transition.cc
  poppler-private.cc
)
add_library(poppler-qt SHARED ${poppler_qt_SRCS})
set_target_properties(poppler-qt PROPERTIES VERSION 3.0.0 SOVERSION 3)
target_link_libraries(poppler-qt poppler ${QT3_LIBRARIES})
install(TARGETS poppler-qt RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

install(FILES
  poppler-qt.h
  poppler-page-transition.h
  poppler-link-qt3.h
  DESTINATION include/poppler/qt3)


set(test_poppler_qt_SOURCES
  test-poppler-qt.cpp
)
poppler_add_test(test-poppler-qt BUILD_QT3_TESTS ${test_poppler_qt_SOURCES})
target_link_libraries(test-poppler-qt poppler-qt)


