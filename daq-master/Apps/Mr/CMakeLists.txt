set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQCLIENT_INCDIR} ${MoIO_INCLUDE_DIR})

include_directories ( ${INCDIR} )

add_definitions(-DkMrFilePath=\"/Raid/monit/Sd/\")

add_executable(Mr Mr.cc MrManager.cc MrUUBCalibMonit.cc MoRawData.cc CbRawData.cc MrRoot.cc MrSignal.cc Mr_ClassDef.cc )
target_link_libraries (Mr IkPaX DbC Ui CSet Gnc GsClient Cl mysqlclient IoSd ${ROOTLIBS} boost_iostreams MoIO)
install (TARGETS Mr  DESTINATION  bin)
