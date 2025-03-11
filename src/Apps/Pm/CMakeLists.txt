set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQ_INCDIR})

include_directories ( ${INCDIR} )

add_executable(Pm PmLog.cc PmDefs.cc PmServices.cc PmClientConnection.cc PmServer.cc Pm.cc)
target_link_libraries (Pm IkPaX DbC Ui CSet mysqlclient)
install (TARGETS Pm  DESTINATION  bin)
