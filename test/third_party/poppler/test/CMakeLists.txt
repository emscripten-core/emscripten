
if (ENABLE_SPLASH)

  if (HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)
    set (perf_test_SRCS
      perf-test.cc
      perf-test-preview-dummy.cc
    )
    add_executable(perf-test ${perf_test_SRCS})
    target_link_libraries(perf-test poppler)
    if (LIB_RT_HAS_NANOSLEEP)
      target_link_libraries(perf-test rt)
    endif (LIB_RT_HAS_NANOSLEEP)
  endif (HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)

endif (ENABLE_SPLASH)

if (GTK_FOUND)

  add_definitions(${GTK2_CFLAGS})

  include_directories(
    ${CMAKE_SOURCE_DIR}/glib
    ${CMAKE_BINARY_DIR}/glib
  )

  if (ENABLE_SPLASH)

    set (gtk_splash_test_SRCS
      gtk-splash-test.cc
    )
    poppler_add_test(gtk-splash-test BUILD_GTK_TESTS ${gtk_splash_test_SRCS})
    target_link_libraries(gtk-splash-test poppler ${GTK2_LIBRARIES})

  endif (ENABLE_SPLASH)

  if (HAVE_CAIRO)

    set (gtk_cairo_test_SRCS
      gtk-cairo-test.cc
    )
    poppler_add_test(gtk-cairo-test BUILD_GTK_TESTS ${gtk_cairo_test_SRCS})
    target_link_libraries(gtk-cairo-test poppler-glib ${GTK2_LIBRARIES})

    set (pdf_inspector_SRCS
      pdf-inspector.cc
    )
    poppler_add_test(pdf-inspector BUILD_GTK_TESTS ${pdf_inspector_SRCS})
    target_link_libraries(pdf-inspector poppler-glib ${GTK2_LIBRARIES})

  endif (HAVE_CAIRO)

endif (GTK_FOUND)

set (pdf_fullrewrite_SRCS
  pdf-fullrewrite.cc
)
add_executable(pdf-fullrewrite ${pdf_fullrewrite_SRCS})
target_link_libraries(pdf-fullrewrite poppler)


