#
# For further details regarding this file, 
# see http://www.vtk.org/Wiki/CMake_Testing_With_CTest#Customizing_CTest
#

SET (CTEST_CUSTOM_MAXIMUM_NUMBER_OF_ERRORS   50)
SET (CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 50)

SET(CTEST_CUSTOM_COVERAGE_EXCLUDE
 ${CTEST_CUSTOM_COVERAGE_EXCLUDE}

 # Exclude files from the Testing directories
 ".*/Testing/.*"
 )

SET(CTEST_CUSTOM_WARNING_EXCEPTION
  ${CTEST_CUSTOM_WARNING_EXCEPTION}
  
  # Suppress warning caused by intentional messages about deprecation
  ".*warning,.* is deprecated"
)
