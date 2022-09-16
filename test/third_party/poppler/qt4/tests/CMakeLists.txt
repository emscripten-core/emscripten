add_definitions(${QT4_DEFINITIONS})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QT4_INCLUDE_DIR}
)

macro(QT4_ADD_SIMPLETEST exe source)
  string(REPLACE "-" "" test_name ${exe})
  set(${test_name}_SOURCES
    ${source}
  )
  poppler_add_test(${exe} BUILD_QT4_TESTS ${${test_name}_SOURCES})
  target_link_libraries(${exe} poppler-qt4)
  if(MSVC)
    target_link_libraries(${exe} poppler ${poppler_LIBS})
  endif(MSVC)
endmacro(QT4_ADD_SIMPLETEST)

macro(QT4_ADD_QTEST exe source)
  if (QT4_QTTEST_FOUND)
    string(REPLACE "-" "" test_name ${exe})
    set(${test_name}_SOURCES
      ${source}
    )
    poppler_add_unittest(${exe} BUILD_QT4_TESTS ${${test_name}_SOURCES})
    qt4_automoc(${${test_name}_SOURCES})
    target_link_libraries(${exe} poppler-qt4 ${QT4_QTTEST_LIBRARY})
    if(MSVC)
      target_link_libraries(${exe} poppler ${poppler_LIBS})
    endif(MSVC)
  endif (QT4_QTTEST_FOUND)
endmacro(QT4_ADD_QTEST)


qt4_add_simpletest(test-poppler-qt4 test-poppler-qt4.cpp)
qt4_add_simpletest(test-password-qt4 test-password-qt4.cpp)
qt4_add_simpletest(poppler-fonts poppler-fonts.cpp)
qt4_add_simpletest(poppler_attachments poppler-attachments.cpp)
qt4_add_simpletest(stress-poppler-qt4 stress-poppler-qt4.cpp)
qt4_add_simpletest(stress-poppler-dir stress-poppler-dir.cpp)
qt4_add_simpletest(poppler-texts poppler-texts.cpp)

qt4_add_qtest(check_attachments check_attachments.cpp)
qt4_add_qtest(check_dateConversion check_dateConversion.cpp)
qt4_add_qtest(check_fonts check_fonts.cpp)
qt4_add_qtest(check_links check_links.cpp)
qt4_add_qtest(check_metadata check_metadata.cpp)
qt4_add_qtest(check_optcontent check_optcontent.cpp)
qt4_add_qtest(check_pagelayout check_pagelayout.cpp)
qt4_add_qtest(check_pagemode check_pagemode.cpp)
qt4_add_qtest(check_password check_password.cpp)
qt4_add_qtest(check_permissions check_permissions.cpp)
qt4_add_qtest(check_search check_search.cpp)
qt4_add_qtest(check_actualtext check_actualtext.cpp)
if (NOT WIN32)
  qt4_add_qtest(check_strings check_strings.cpp)
endif (NOT WIN32)
