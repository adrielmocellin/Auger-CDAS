set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQCLIENT_INCDIR})

include_directories ( ${INCDIR} )

add_executable(Xb Xb.cc XbIk.cc XbPm.cc XbArray.cc XbAlgo.cc XbArrayDb.cc)
target_link_libraries (Xb IkPaX DbC Ui CSet Gnc GsClient mysqlclient boost_system)
install (TARGETS Xb  DESTINATION  bin)

add_executable(XbDumpT2 XbDumpT2.cc XbIk.cc XbPm.cc XbArray.cc XbAlgo.cc XbArrayDb.cc)
target_link_libraries (XbDumpT2 IkPaX DbC Ui CSet Gnc GsClient mysqlclient boost_system)
install (TARGETS XbDumpT2  DESTINATION  bin)

add_executable(XbFast XbFast.cc XbIk.cc XbPm.cc XbArray.cc XbAlgo.cc XbArrayDb.cc)
target_link_libraries (XbFast IkPaX DbC Ui CSet Gnc GsClient mysqlclient boost_system pthread)
install (TARGETS XbFast  DESTINATION  bin)

MESSAGE(WARNING "servers.list has to be installed in /Raid/var/XbFast.servers.list for XbFast to send properly udp packets")
