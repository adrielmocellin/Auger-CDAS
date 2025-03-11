set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQCLIENT_INCDIR} ${IoSd_INCLUDE_DIR} ${STCoordinates_INCLUDE_DIR})

include_directories ( ${INCDIR} )

ROOT_GENERATE_DICTIONARY(FdDict LINKDEF T3NotifyLinkDef.h INFILES T3Notify.hh)

add_executable(Fd Fd.cc FdTrigger.cc T3Notify.cc ${FdDict_DICT_SOURCE})
target_link_libraries (Fd IkPaX DbC Ui CSet Gnc GsClient mysqlclient IoSd ${ROOTLIBS})
install (TARGETS Fd  DESTINATION  bin)

#add_executable(test test.cc T3Notify.cc FdDict.cc)
#target_link_libraries (test IkPaX DbC Ui CSet Gnc GsClient mysqlclient IoSd ${ROOTLIBS})
#install (TARGETS test  DESTINATION  bin)
