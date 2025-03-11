set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CDASDAQCLIENT_INCDIR})

include_directories ( ${INCDIR} )

add_executable(SDDownload SDDownload.cc SDDownloadIk.cc SDDownloadPm.cc)
target_link_libraries (SDDownload IkPaX DbC Ui CSet Gnc GsClient mysqlclient)
install (TARGETS SDDownload  DESTINATION  bin)

