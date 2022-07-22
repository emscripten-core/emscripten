include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_SOURCE_DIR}/utils
)

macro(CPP_ADD_SIMPLETEST exe)
  string(REPLACE "-" "" test_name ${exe})
  set(${test_name}_SOURCES
    ${ARGN}
  )
  poppler_add_test(${exe} BUILD_CPP_TESTS ${${test_name}_SOURCES})
  target_link_libraries(${exe} poppler-cpp)
  if(MSVC)
    target_link_libraries(${exe} poppler ${poppler_LIBS})
  endif(MSVC)
endmacro(CPP_ADD_SIMPLETEST)


cpp_add_simpletest(poppler-dump poppler-dump.cpp ${CMAKE_SOURCE_DIR}/utils/parseargs.cc)
target_link_libraries(poppler-dump poppler)

cpp_add_simpletest(poppler-render poppler-render.cpp ${CMAKE_SOURCE_DIR}/utils/parseargs.cc)
target_link_libraries(poppler-render poppler)
