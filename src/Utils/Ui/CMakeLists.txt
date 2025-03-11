set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR} ${CSET_INCDIR})

include_directories ( ${INCDIR} )

set (UI_HEADERS UiArgs.h  UiCmdLine.h  UiErr.h  UiLk.h  UiLst.h  UiMessage.h)

add_library (Ui SHARED UiErr.cc UiLk.cc UiArgs.cc UiCmdLine.cc UiMessage.cc)

target_link_libraries (Ui)
install (TARGETS Ui LIBRARY DESTINATION  lib)

