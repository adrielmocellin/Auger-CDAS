set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQCLIENT_INCDIR})

include_directories ( ${INCDIR} )

add_executable(Rc Rc.cc RcIk.cc RcDb.cc)
target_link_libraries (Rc IkPaX DbC Ui CSet Gnc GsClient mysqlclient)
install (TARGETS Rc  DESTINATION  bin)

add_executable(SqlDbQuery MySqlDbQuery.cc)
target_link_libraries (SqlDbQuery IkPaX DbC Ui CSet Gnc GsClient mysqlclient)
install (TARGETS SqlDbQuery  DESTINATION  bin)

add_executable(AddStation AddStation.cc  RcIk.cc RcDb.cc)
target_link_libraries (AddStation IkPaX DbC Ui CSet Gnc GsClient mysqlclient)
install (TARGETS AddStation  DESTINATION  bin)

add_executable(StationHistory StationsHistory.cc)
target_link_libraries (StationHistory IkPaX DbC Ui CSet Gnc GsClient mysqlclient)
install (TARGETS StationHistory  DESTINATION  bin)

