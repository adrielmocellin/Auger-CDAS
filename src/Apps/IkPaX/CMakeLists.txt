set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR})
set (IkPaX_SOURCES IkClient.cc IkMessage.cc IkC.cc IkCTypify.cc IkCFiles.cc IkCSend.cc)
set (IkPaX_HEADERS Ik.h  IkC.h  IkClient.h  IkMessage.h  IkTCP.h IkCTypes.h)

include_directories ( ${INCDIR} ${CMAKE_CURRENT_BINARY_DIR} )

# FIXME Uglier impossible
add_custom_command(OUTPUT IkCTypify.cc IkCFiles.cc IkCTypes.h COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/ddl/*.ddl ${CMAKE_CURRENT_BINARY_DIR} \; ${CMAKE_CURRENT_SOURCE_DIR}/DDLManage_IkPaX.pl)

add_library (IkPaX SHARED ${IkPaX_SOURCES} ${IkPaX_HEADERS})
target_link_libraries (IkPaX)
install (TARGETS IkPaX LIBRARY DESTINATION  lib)

add_executable(IkServer IkServer.cc IkTCP.cc)
target_link_libraries (IkServer IkPaX)
install (TARGETS IkServer  DESTINATION  bin)
