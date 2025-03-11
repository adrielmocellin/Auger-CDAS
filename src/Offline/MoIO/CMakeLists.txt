set (INCDIR
     ${CMAKE_CURRENT_SOURCE_DIR}
    )

include_directories ( ${INCDIR} )

set (MoIO_SOURCES
   TSDCalibration.cc
   TSDMonCal.cc
   TSDMonitoring.cc
   TSDRawMonitoring.cc
   TSDSystem.cc
)

set (MoIO_HEADERS
  TSDCalibration.h
  TSDMonCal.h
  TSDMonitoring.h
  TSDRawMonitoring.h
  TSDSystem.h
)


ROOT_GENERATE_DICTIONARY(MoIO
        LINKDEF MoIO_LinkDef.h
        INFILES ${MoIO_HEADERS}
 )

list (APPEND MoIO_SOURCES ${MoIO_DICT_SOURCE})
list (APPEND MoIO_HEADERS ${MoIO_DICT_HEADER})


add_library ( MoIO SHARED ${MoIO_SOURCES} ${MoIO_HEADERS})
target_link_libraries (MoIO  ${ROOT_LIBRARIES} )

install (TARGETS MoIO LIBRARY DESTINATION  lib)

