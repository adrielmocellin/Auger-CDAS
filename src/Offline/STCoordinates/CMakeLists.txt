set (INCDIR
      ${CMAKE_CURRENT_SOURCE_DIR}
    )


include_directories ( ${INCDIR} )


set (STCoordinates_SOURCES
    LatLong-UTMConversion.cc
    STClibrary.cc	
    TCdasUtil.cc
    TSTSky.cc
    TShowerParams.cc
)    

set (STCoordinates_HEADERS
     LatLong-UTMConversion.h
     STCconstants.h
     STClibrary.h
     TCdasUtil.h
     TSTSky.h
     TShowerParams.h 
)

add_library ( STCoordinates SHARED ${STCoordinates_SOURCES} ${STCoordinates_HEADERS}) 
target_link_libraries (STCoordinates ${ROOT_LIBRARIES} )

install (TARGETS STCoordinates LIBRARY DESTINATION lib)

