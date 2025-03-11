# Macros defined here are
#  MARK_AS_INTERNAL
#  AUGER_CREATE_LINKS
#  AUGER_INCLUDE
#  FIRST
#  REMOVE_FIRST
#  LIST_CONTAINS
#  LIST_REGEX_REPLACE
#  PARSE_ARGUMENTS
#  MAKE_LDFLAGS

INCLUDE (CMakeCompareVersionStrings)

# MARK_AS_INTERNAL macro
# - Macro to hide a variable from the GUI
#

MACRO (MARK_AS_INTERNAL _var)
  SET (${_var} ${${_var}} CACHE INTERNAL "hide this!" FORCE)
ENDMACRO (MARK_AS_INTERNAL _var)


# AUGER_CREATE_LINKS
# - This macro is a convenience to create symbolic links to a list of files inside a directory
#
# Usage:
#  AUGER_CREATE_LINKS(FILES ${files} DIRECTORY ${directory} VERBOSE)

MACRO (AUGER_CREATE_LINKS)
  PARSE_ARGUMENTS (_AUGER_CREATE_LINKS
    "DIRECTORY;FILES"
    "VERBOSE"
    ${ARGN}
  )
  FILE (MAKE_DIRECTORY ${_AUGER_CREATE_LINKS_DIRECTORY})
  IF (_AUGER_CREATE_LINKS_VERBOSE)
    MESSAGE (STATUS "Will create links in directory ${_AUGER_CREATE_LINKS_DIRECTORY}.")
  ENDIF (_AUGER_CREATE_LINKS_VERBOSE)
  FOREACH (_file ${_AUGER_CREATE_LINKS_FILES})
    GET_FILENAME_COMPONENT (_link ${_file} NAME)
    GET_SOURCE_FILE_PROPERTY (_location ${_file} LOCATION)
    IF (_location)
      IF (_AUGER_CREATE_LINKS_VERBOSE)
        MESSAGE (STATUS "ln -f -s ${_location} ${_AUGER_CREATE_LINKS_DIRECTORY}/${_link}")
      ENDIF (_AUGER_CREATE_LINKS_VERBOSE)
      EXECUTE_PROCESS (COMMAND ln -f -s ${_location} ${_AUGER_CREATE_LINKS_DIRECTORY}/${_link})
    ELSE (_location)
      MESSAGE (WARNING "File ${_file} does not exist!")
    ENDIF (_location)
  ENDFOREACH (_file ${_AUGER_CREATE_LINKS_FILES})
ENDMACRO (AUGER_CREATE_LINKS)


# AUGER_INCLUDE macro
# - Auger specific macro to replace CMake's include macro.
#
# This macro is meant to substitude CMake's include macro while keeping
# track of the current directory. The current directory is stored in
# AUGER_CURRENT_SOURCE_DIR (and AUGER_CURRENT_BINARY_DIR).
# As you go deeper in the hierarchy structure, a stack of directories is
# created so when you return from the include macro the variables are restored.
#
# configure-style variables 'srcdir' and 'builddir' are set to _absolute_ paths.
#
# Let's assume we have a subdirectory 'Subdir_1' inside CMAKE_CURRENT_SOURCE_DIR.
# Inside this subdirectory there is another subdirectory: 'Subdir_2'
# If you have used this macro to include, then the following should produce the same result
# when invoked from within Subdir_1:
#  AUGER_INCLUDE(./Subdir_2/CMakeLists.txt)
#  AUGER_INCLUDE(${srcdir}/Subdir_2/CMakeLists.txt)
#  AUGER_INCLUDE(${AUGER_CURRENT_SOURCE_DIR}/Subdir_1/CMakeLists.txt)
#  AUGER_INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/Subdir_1/Subdir_2/CMakeLists.txt)

MACRO (AUGER_INCLUDE FILE)
  SET (_dummy_variable NOTFOUND)
  FIND_FILE (_dummy_variable ${FILE} / ${CMAKE_CURRENT_SOURCE_DIR} ${AUGER_CURRENT_SOURCE_DIR} NO_DEFAULT_PATH)
  FIND_FILE (_dummy_variable ${FILE}.cmake / ${CMAKE_CURRENT_SOURCE_DIR} ${AUGER_CURRENT_SOURCE_DIR} NO_DEFAULT_PATH)
  IF (NOT _dummy_variable)
    MESSAGE (STATUS "ERROR!: File ${FILE}.cmake not found in:  / ${CMAKE_CURRENT_SOURCE_DIR} ${AUGER_CURRENT_SOURCE_DIR}")
  ELSE (NOT _dummy_variable)
    IF (AUGER_CURRENT_SOURCE_DIR)
      SET (AUGER_SOURCE_DIR_STACK ${AUGER_CURRENT_SOURCE_DIR} ${AUGER_SOURCE_DIR_STACK})
      SET (AUGER_BINARY_DIR_STACK ${AUGER_CURRENT_BINARY_DIR} ${AUGER_BINARY_DIR_STACK})
    ELSE (AUGER_CURRENT_SOURCE_DIR)
      SET (AUGER_SOURCE_DIR_STACK ${CMAKE_CURRENT_SOURCE_DIR} ${AUGER_SOURCE_DIR_STACK})
      SET (AUGER_BINARY_DIR_STACK ${CMAKE_CURRENT_SOURCE_DIR} ${AUGER_BINARY_DIR_STACK})
    ENDIF (AUGER_CURRENT_SOURCE_DIR)
    GET_SOURCE_FILE_PROPERTY (FILE_LOCATION ${_dummy_variable} LOCATION)
    GET_FILENAME_COMPONENT (AUGER_CURRENT_SOURCE_DIR ${FILE_LOCATION} PATH)
    STRING (REGEX REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} AUGER_CURRENT_BINARY_DIR ${AUGER_CURRENT_SOURCE_DIR})
    SET (srcdir ${AUGER_CURRENT_SOURCE_DIR})
    SET (builddir ${AUGER_CURRENT_BINARY_DIR})
    SET (abs_srcdir ${AUGER_CURRENT_SOURCE_DIR})
    SET (abs_builddir ${AUGER_CURRENT_BINARY_DIR})
    INCLUDE (${FILE_LOCATION})
    FIRST (AUGER_CURRENT_SOURCE_DIR ${AUGER_SOURCE_DIR_STACK})
    FIRST (AUGER_CURRENT_BINARY_DIR ${AUGER_BINARY_DIR_STACK})
    REMOVE_FIRST (AUGER_SOURCE_DIR_STACK ${AUGER_SOURCE_DIR_STACK})
    REMOVE_FIRST (AUGER_BINARY_DIR_STACK ${AUGER_BINARY_DIR_STACK})
    SET (srcdir ${AUGER_CURRENT_SOURCE_DIR})
    SET (builddir ${AUGER_CURRENT_BINARY_DIR})
    SET (abs_srcdir ${AUGER_CURRENT_SOURCE_DIR})
    SET (abs_builddir ${AUGER_CURRENT_BINARY_DIR})
  ENDIF (NOT _dummy_variable)
  MARK_AS_INTERNAL (_dummy_variable)
ENDMACRO (AUGER_INCLUDE)


# LIST_REGEX_PLACE macro
# - This macro does a regex replacement in a list of variables
#
# Usage:
#  LIST_REGEX_REPLACE(${regex_1} ${regex_2} ${string_list})
# will replace regex_1 by regex_2 in all elements in the list

MACRO (LIST_REGEX_PLACE value_1 value_2 destination)
  SET (${destination})
  FOREACH (_item ${ARGN})
    STRING (REGEX REPLACE ${value_1} ${value_2} _item ${_item})
    LIST (APPEND ${destination} ${_item})
  ENDFOREACH (_item)
ENDMACRO (LIST_REGEX_PLACE)


# LIST_CONTAINS macro
# - This macro checks if a list contains a given value
#
# The variable given as first argument will be set to either TRUE or FALSE.
#
# Usage:
#  LIST_CONTAINS(value ${list})
# value will be set to either TRUE or FALSE

MACRO  (LIST_CONTAINS var value)
  SET (${var})
  FOREACH (value2 ${ARGN})
    IF (${value} STREQUAL ${value2})
      SET (${var} TRUE)
    ENDIF (${value} STREQUAL ${value2})
  ENDFOREACH (value2)
ENDMACRO (LIST_CONTAINS)


# FIRST macro
# - This macro returns sets the first argument given equal to the first element in a list.
# The list should be the second argument
#
# Usage:
#  FIRST(variable ${list})

MACRO (FIRST var)
  SET (${var} ${ARGV1})
ENDMACRO (FIRST)


# REMOVE_FIRST macro
# - Removes the first element of the list given as argument
#
# Usage:
#  REMOVE_FIRST(list ${list})

MACRO (REMOVE_FIRST var junk)
  SET (${var} ${ARGN})
ENDMACRO (REMOVE_FIRST)


# PARSE_ARGUMENTS macro
# - Macro to parse arguments given to macros.
#
# This is copied from http://www.cmake.org/Wiki/CMake_User_Contributed_Macros.
#
# Usage examples:
#   PARSE_ARGUMENTS( ${prefix} ${arg_names} ${option_names})
#   PARSE_ARGUMENTS(  _PREFIX "A;B" "C" ${ARGN})
#
# It will parse ${ARGN} and define the variables _PREFIX_A and _PREFIX_B.
# It will set _PREFIX_C to either TRUE or FALSE.

MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  # Note: macro behavior changes for CMake version > 2.4.7.
  COMPARE_VERSION_STRINGS (
    "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}"
    "2.4.7"
    _version_GT_CMake247)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})
    # Version > 2.4.7
    IF (_version_GT_CMake247 EQUAL 1)
      SET(larg_names ${arg_names})
      LIST(FIND larg_names "${arg}" is_arg_name)
      IF (is_arg_name GREATER -1)
        SET(${prefix}_${current_arg_name} ${current_arg_list})
        SET(current_arg_name ${arg})
        SET(current_arg_list)
      ELSE (is_arg_name GREATER -1)
        SET(loption_names ${option_names})
        LIST(FIND loption_names "${arg}" is_option)
        IF (is_option GREATER -1)
         SET(${prefix}_${arg} TRUE)
        ELSE (is_option GREATER -1)
         SET(current_arg_list ${current_arg_list} ${arg})
        ENDIF (is_option GREATER -1)
      ENDIF (is_arg_name GREATER -1)
    # Version <= 2.4.7
    ELSE (_version_GT_CMake247 EQUAL 1)
      LIST_CONTAINS (is_arg_name ${arg} ${arg_names})
      IF (is_arg_name)
        SET (${prefix}_${current_arg_name} ${current_arg_list})
        SET (current_arg_name ${arg})
        SET (current_arg_list)
      ELSE (is_arg_name)
        LIST_CONTAINS (is_option ${arg} ${option_names})
        IF (is_option)
          SET (${prefix}_${arg} TRUE)
        ELSE (is_option)
          SET (current_arg_list ${current_arg_list} ${arg})
        ENDIF (is_option)
      ENDIF (is_arg_name)
    ENDIF (_version_GT_CMake247 EQUAL 1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)

# MAKE_LDFLAGS macro
# converts a list of full-path names of libraries into the -L -l format of the LDFLAGS
# example: MAKE_LDFLAGS (FOO_LDFLAGS "/a/b/c/libFoo1.so;/a/b/c/libFoo2.so;/a/b/z/libBar1.so") will
# create FOO_LDFLAGS = "-L/a/b/c -lFoo1 -lFoo2 -L/a/b/z -lBar1"

MACRO (MAKE_LDFLAGS var libs)
  SET (_libs "${libs}")
  LIST (SORT _libs)
  LIST (REMOVE_DUPLICATES _libs)
  SET (_Ldir)
  FOREACH (_lib ${_libs})
    GET_FILENAME_COMPONENT (_dir "${_lib}" PATH)
    IF (NOT "${_dir}" STREQUAL "${_Ldir}")
      IF (${${var}})
        SET (${var} "${${var}} -L${_dir}")
      ELSE ()
        SET (${var} "-L${_dir}")
      ENDIF ()
      SET (_Ldir "${_dir}")
    ENDIF ()
    GET_FILENAME_COMPONENT (_file "${_lib}" NAME_WE)
    STRING (REGEX REPLACE "^lib" "" _file "${_file}")
    SET (${var} "${${var}} -l${_file}")
  ENDFOREACH ()
ENDMACRO ()
