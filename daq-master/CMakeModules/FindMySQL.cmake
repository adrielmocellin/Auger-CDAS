################################################################################
# Module to find MySQL                                                         #
#                                                                              #
# This sets the following variables:                                           #
#   - MySQL_FOUND                                                              #
#   - MySQL_LIBRARIES                                                          #
#   - MySQL_INCLUDE_DIR                                                        #
#   - MYSQL_CPPFLAGS                                                           #
#   - MYSQL_LDFLAGS                                                            #
################################################################################

SET (DIRECTORIES
  $ENV{MYSQLROOT}
  /usr/local/mysql
  /usr
)

SET (INC_DIRECTORIES)
SET (LIB_DIRECTORIES)

FOREACH (DIRECTORY ${DIRECTORIES})
  LIST (APPEND INC_DIRECTORIES
    ${DIRECTORY}/include
    ${DIRECTORY}/mysql
    ${DIRECTORY}/include/mysql)
  LIST (APPEND LIB_DIRECTORIES
    ${DIRECTORY}/lib
    ${DIRECTORY}/lib64
    ${DIRECTORY}/mysql
    ${DIRECTORY}/lib/mysql
    ${DIRECTORY}/lib64/mysql)
ENDFOREACH (DIRECTORY)

# special case: weird mysql installation from macports
LIST (APPEND INC_DIRECTORIES /opt/local/include/mysql5)
LIST (APPEND LIB_DIRECTORIES /opt/local/lib/mysql5/mysql)

FIND_LIBRARY (MySQL_LIBRARIES mysqlclient ${LIB_DIRECTORIES} NO_DEFAULT_PATH)
FIND_LIBRARY (MySQL_LIBRARIES mysqlclient ${LIB_DIRECTORIES})
FIND_PATH (MySQL_INCLUDE_DIR mysql/mysql.h ${INC_DIRECTORIES} NO_DEFAULT_PATH)
FIND_PATH (MySQL_INCLUDE_DIR mysql/mysql.h ${INC_DIRECTORIES} )
# This is for auger-offline-config's LD_LIBRARY_PATH
GET_FILENAME_COMPONENT (MySQL_LIB_DIR ${MySQL_LIBRARIES} PATH)

IF (MySQL_LIBRARIES AND MySQL_INCLUDE_DIR)
  SET (MySQL_FOUND TRUE)
  SET (HAVE_MYSQL 1)
  SET (MYSQL_LDFLAGS ${MySQL_LIBRARIES})
  SET (MYSQL_CPPFLAGS "-I${MySQL_INCLUDE_DIR}")
ENDIF (MySQL_LIBRARIES AND MySQL_INCLUDE_DIR)

SET (DIRECTORIES)
SET (LIB_DIRECTORIES)
SET (INC_DIRECTORIES)

IF (MySQL_FOUND)
  IF (NOT MySQL_FIND_QUIETLY)
    MESSAGE (STATUS "MySQL library: ${MySQL_LIBRARIES}")
    MESSAGE (STATUS "MySQL include: ${MySQL_INCLUDE_DIR}")
  ENDIF (NOT MySQL_FIND_QUIETLY)
ELSE (MySQL_FOUND)
  IF (MySQL_FIND_REQUIRED)
    MESSAGE (FATAL_ERROR "Could not find MySQL")
  else (MySQL_FIND_REQUIRED)
    if (MySQL_FIND_QUIETLY)
      MESSAGE (STATUS "Could not find MySQL")
    endif (MySQL_FIND_QUIETLY)
  ENDIF (MySQL_FIND_REQUIRED)
ENDIF (MySQL_FOUND)
