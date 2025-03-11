set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${MySQL_INCLUDE_DIR})

include_directories ( ${INCDIR} )

set (DBC_HEADERS DbC.h  DbCArray.h  DbCConfig.h)

add_library (DbC SHARED DbCArray.cc)

target_link_libraries (DbC)
install (TARGETS DbC LIBRARY DESTINATION  lib)

