set (INCDIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories ( ${INCDIR} )

set (CSET_HEADERS CAnchor.h  CArray.ph  CCAnchor.hxx  CSet.h  GC.ph  GCC.hxx  Hash.ph  LkList.h   LkList.pp  Reg.h   Reg.pp     RegLex.pp   RegParse.ph  TAnchor.ph  TArray.ph CArray.h   CArray.pp  CCArray.hxx   GC.h    GC.pp  Hash.h   Hash.pp  LkList.ph  LkListP.h  Reg.ph  RegLex.ph  RegParse.h  RegParse.pp  TAnchor.pp  TArray.pp)

add_library (CSet SHARED  LkList.c Hash.c CArray.c GC.c)
# not used
#add_library (CCSet SHARED  GCC.cxx CCArray.cxx)
#add_library (Reg SHARED  Reg.c)

target_link_libraries (CSet )
install (TARGETS CSet LIBRARY DESTINATION  lib)
#target_link_libraries (CSet CCSet Reg)
#install (TARGETS CSet CCSet Reg LIBRARY DESTINATION  lib)

