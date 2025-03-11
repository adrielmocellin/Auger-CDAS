set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${UI_INCDIR})

include_directories ( ${INCDIR} )

set (GS_HEADERS Gs.h  GsClients.hxx  GsIk.h  GsMess.h  GsNotify.h  GsProtocol.h  GsServer.h  GsServices.h  GsThreads.h  GsTools.h  Gsp.h)

add_library (Gs SHARED GsLoop.cc GsClientManager.cc GsInit.cc GsNotify.cc GsEndOfLoop.cc GsMess.cc)
add_library (GsClient SHARED GsMess.cc)

target_link_libraries (Gs GsClient)
install (TARGETS Gs GsClient LIBRARY DESTINATION  lib)

