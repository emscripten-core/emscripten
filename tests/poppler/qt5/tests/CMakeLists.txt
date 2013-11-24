set(CMAKE_AUTOMOC ON)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(-DTESTDATADIR=\"${TESTDATADIR}\")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Xml_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Xml_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

macro(QT5_ADD_SIMPLETEST exe source)
  string(REPLACE "-" "" test_name ${exe})
  set(${test_name}_SOURCES
    ${source}
  )
  poppler_add_test(${exe} BUILD_QT5_TESTS ${${test_name}_SOURCES})
  target_link_libraries(${exe} poppler-qt5 ${Qt5Widgets_LIBRARIES})
  if(MSVC)
    target_link_libraries(${exe} poppler ${poppler_LIBS})
  endif(MSVC)
endmacro(QT5_ADD_SIMPLETEST)

macro(QT5_ADD_QTEST exe source)
  if (Qt5Test_FOUND)	
    string(REPLACE "-" "" test_name ${exe})
    set(${test_name}_SOURCES
      ${source}
    )
    poppler_add_unittest(${exe} BUILD_QT5_TESTS ${${test_name}_SOURCES})
	target_link_libraries(${exe} poppler-qt5 ${Qt5Widgets_LIBRARIES} ${Qt5Test_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES})
    if(MSVC)
		target_link_libraries(${exe} poppler ${poppler_LIBS})
    endif(MSVC)
  endif (Qt5Test_FOUND)
endmacro(QT5_ADD_QTEST)


qt5_add_simpletest(test-poppler-qt5 test-poppler-qt5.cpp)
qt5_add_simpletest(test-password-qt5 test-password-qt5.cpp)
qt5_add_simpletest(poppler-forms poppler-forms.cpp)
qt5_add_simpletest(poppler-fonts poppler-fonts.cpp)
qt5_add_simpletest(poppler_attachments poppler-attachments.cpp)
qt5_add_simpletest(stress-poppler-qt5 stress-poppler-qt5.cpp)
qt5_add_simpletest(stress-poppler-dir stress-poppler-dir.cpp)
qt5_add_simpletest(stress-threads-qt5 stress-threads-qt5.cpp)
qt5_add_simpletest(poppler-texts poppler-texts.cpp)

qt5_add_qtest(check_attachments check_attachments.cpp)
qt5_add_qtest(check_dateConversion check_dateConversion.cpp)
qt5_add_qtest(check_fonts check_fonts.cpp)
qt5_add_qtest(check_links check_links.cpp)
qt5_add_qtest(check_metadata check_metadata.cpp)
qt5_add_qtest(check_optcontent check_optcontent.cpp)
qt5_add_qtest(check_pagelayout check_pagelayout.cpp)
qt5_add_qtest(check_pagemode check_pagemode.cpp)
qt5_add_qtest(check_password check_password.cpp)
qt5_add_qtest(check_permissions check_permissions.cpp)
qt5_add_qtest(check_search check_search.cpp)
qt5_add_qtest(check_actualtext check_actualtext.cpp)
qt5_add_qtest(check_lexer check_lexer.cpp)
qt5_add_qtest(check_pagelabelinfo check_pagelabelinfo.cpp)
qt5_add_qtest(check_goostring check_goostring.cpp)
if (NOT WIN32)
  qt5_add_qtest(check_strings check_strings.cpp)
endif (NOT WIN32)
