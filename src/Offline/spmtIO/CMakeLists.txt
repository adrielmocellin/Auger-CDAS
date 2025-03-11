set (INCDIR
     ${CMAKE_CURRENT_SOURCE_DIR}
    )

include_directories ( ${INCDIR} )

set (spmtIO_SOURCES
  spmtIO.cc
  spmtIOEvt.cc
  spmtIOMuonCalib.cc
  spmtIOThreshold.cc
)

set (spmtIO_HEADERS
  spmtIO.h
  spmtIOEvt.h
  spmtIOMuonCalib.h
  spmtIOThreshold.h
)

ROOT_GENERATE_DICTIONARY(spmtIO
        LINKDEF spmt_LinkDef.h
        INFILES ${spmtIO_HEADERS}
 )

list (APPEND spmtIO_SOURCES ${spmtIO_DICT_SOURCE})
list (APPEND spmtIO_HEADERS ${spmtIO_DICT_HEADER})


add_library ( spmtIO SHARED ${spmtIO_SOURCES} ${spmtIO_HEADERS})
target_link_libraries (spmtIO  ${ROOT_LIBRARIES} )

install (TARGETS spmtIO LIBRARY DESTINATION  lib)

