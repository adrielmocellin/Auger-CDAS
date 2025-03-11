set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQCLIENT_INCDIR} ${IoSd_INCLUDE_DIR})

include_directories ( ${INCDIR} )

add_executable(Eb Eb.cc EbMain.cc EbEvent.cc EbManager.cc T3_uub_v262.cc T3_uub_v263.cc T3_uub_v264.cc)
target_link_libraries (Eb IkPaX DbC Ui CSet Gnc GsClient Cl mysqlclient IoSd ${ROOTLIBS} boost_iostreams)
install (TARGETS Eb  DESTINATION  bin)

add_executable(EbFast EbFast.cc EbMain.cc EbEvent.cc EbManager.cc T3_uub_v262.cc T3_uub_v263.cc T3_uub_v264.cc)
target_link_libraries (EbFast IkPaX DbC Ui CSet Gnc GsClient Cl mysqlclient IoSd ${ROOTLIBS} boost_iostreams)
install (TARGETS EbFast  DESTINATION  bin)

