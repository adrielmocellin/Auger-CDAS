set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQCLIENT_INCDIR} ${MoIO_INCLUDE_DIR} ${spmtIO_INCLUDE_DIR})

include_directories ( ${INCDIR} )

add_definitions(-DkSpmtFilePath=\"/Raid/monit/Sd/\")

add_executable(spmt sPMT.cc sPMT_Pm.cc SPMTData.cc SPMTRoot.cc)
target_link_libraries (spmt IkPaX Ui CSet Gnc GsClient ${ROOTLIBS} spmtIO boost_iostreams)
install (TARGETS spmt  DESTINATION  bin)
