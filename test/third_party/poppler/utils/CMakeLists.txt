
set(common_srcs
  parseargs.cc
)
set(common_libs
  poppler
)
if (FONTCONFIG_FOUND)
  set(common_libs ${common_libs} ${FONTCONFIG_LIBRARIES})
endif (FONTCONFIG_FOUND)

if (ENABLE_SPLASH)
  # pdftoppm
  set(pdftoppm_SOURCES ${common_srcs}
    pdftoppm.cc
  )
  add_executable(pdftoppm ${pdftoppm_SOURCES})
  target_link_libraries(pdftoppm ${common_libs})
  install(TARGETS pdftoppm DESTINATION bin)
  install(FILES pdftoppm.1 DESTINATION share/man/man1)
endif (ENABLE_SPLASH)

# pdffonts
set(pdffonts_SOURCES ${common_srcs}
  pdffonts.cc
)
add_executable(pdffonts ${pdffonts_SOURCES})
target_link_libraries(pdffonts ${common_libs})
install(TARGETS pdffonts DESTINATION bin)
install(FILES pdffonts.1 DESTINATION share/man/man1)

# pdfimages
set(pdfimages_SOURCES ${common_srcs}
  pdfimages.cc
  ImageOutputDev.cc
  ImageOutputDev.h
)
add_executable(pdfimages ${pdfimages_SOURCES})
target_link_libraries(pdfimages ${common_libs})
install(TARGETS pdfimages DESTINATION bin)
install(FILES pdfimages.1 DESTINATION share/man/man1)

# pdfinfo
set(pdfinfo_SOURCES ${common_srcs}
  pdfinfo.cc printencodings.cc
)
add_executable(pdfinfo ${pdfinfo_SOURCES})
target_link_libraries(pdfinfo ${common_libs})
install(TARGETS pdfinfo DESTINATION bin)
install(FILES pdfinfo.1 DESTINATION share/man/man1)

# pdftops
set(pdftops_SOURCES ${common_srcs}
  pdftops.cc
)
add_executable(pdftops ${pdftops_SOURCES})
target_link_libraries(pdftops ${common_libs})
install(TARGETS pdftops DESTINATION bin)
install(FILES pdftops.1 DESTINATION share/man/man1)

# pdftotext
set(pdftotext_SOURCES ${common_srcs}
  pdftotext.cc printencodings.cc
)
add_executable(pdftotext ${pdftotext_SOURCES})
target_link_libraries(pdftotext ${common_libs})
install(TARGETS pdftotext DESTINATION bin)
install(FILES pdftotext.1 DESTINATION share/man/man1)

# pdftohtml
set(pdftohtml_SOURCES ${common_srcs}
  pdftohtml.cc
  HtmlFonts.cc
  HtmlLinks.cc
  HtmlOutputDev.cc
)
add_executable(pdftohtml ${pdftohtml_SOURCES})
target_link_libraries(pdftohtml ${common_libs})
install(TARGETS pdftohtml DESTINATION bin)
install(FILES pdftohtml.1 DESTINATION share/man/man1)

if (ENABLE_ABIWORD)
  # pdftoabw
  set(pdftoabw_SOURCES ${common_srcs}
    pdftoabw.cc
  )
  add_executable(pdftoabw ${pdftoabw_SOURCES})
  target_link_libraries(pdftoabw ${common_libs} ${LIBXML2_LIBRARIES})
  install(TARGETS pdftoabw DESTINATION bin)
endif (ENABLE_ABIWORD)

