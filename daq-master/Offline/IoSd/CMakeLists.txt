set (INCDIR
      ${STCoordinates_INCLUDE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
    )


include_directories ( ${INCDIR} )


set (IoSd_SOURCES
    IoSd.cc
    IoSdData.cc
    IoSdFile.cc
    IoSdMeteo.cc
    IoSdRootI.cc
)

set (IoSd_HEADERS
    IoSd.h
    IoSdData.h
    IoSdFile.h
    IoSdMeteo.h
    IoSdRootI.h
    counted_ptr.h
)


ROOT_GENERATE_DICTIONARY(IoSdData
        LINKDEF IoSdDataLinkDef.h
        INFILES ${IoSd_HEADERS}
 )

list (APPEND IoSd_SOURCES ${IoSdData_DICT_SOURCE})
list (APPEND IoSd_HEADERS ${IoSdData_DICT_HEADER})


add_library ( IoSd SHARED ${IoSd_SOURCES} ${IoSd_HEADERS})
target_link_libraries (IoSd STCoordinates ${ROOT_LIBRARIES} )



install (TARGETS IoSd LIBRARY DESTINATION  lib)

