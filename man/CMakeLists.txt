if (BUILD_MANPAGE)
    find_program(XSLTPROC NAMES xsltproc REQUIRED)
    set(DB2MAN "/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/manpages/docbook.xsl")
    set(MAN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/cppcheck.1.xml")
    add_custom_target(man ${XSLTPROC} --nonet --param man.charmap.use.subset "0" ${DB2MAN} ${MAN_SOURCE}
        DEPENDS ${MAN_SOURCE})
endif()