include_directories(
  ${GTK2_INCLUDE_DIRS}
)

add_definitions(${GTK2_CFLAGS_OTHER})

set(poppler_glib_demo_SRCS
  main.c
  find.c
  fonts.c
  forms.c
  info.cc
  images.c
  links.c
  outline.c
  page.c
  print.c
  render.c
  text.c
  transitions.c
  utils.c
  annots.c
  attachments.c
  layers.c
  selections.c
)
poppler_add_test(poppler-glib-demo BUILD_GTK_TESTS ${poppler_glib_demo_SRCS})
target_link_libraries(poppler-glib-demo poppler-glib ${GTK2_LIBRARIES})
