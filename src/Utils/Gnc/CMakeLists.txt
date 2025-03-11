set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${GS_INCDIR})

include_directories ( ${INCDIR} )

set (GNC_HEADERS Gnc.h  GncClose.h  GncCmp.h  GncError.h  GncInit.h  GncLoop.h  GncProcessMessage.h  GncRead.h  GncWatch.h  GncWrite.h)

add_library (Gnc SHARED GncInit.cc GncClose.cc GncRead.cc GncWrite.cc GncLoop.cc GncProcessMessage.cc GncCmp.cc GncWatch.cc)

target_link_libraries (Gnc)
install (TARGETS Gnc LIBRARY DESTINATION  lib)

