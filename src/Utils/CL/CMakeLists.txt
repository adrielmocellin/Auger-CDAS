add_executable(PmRead PmRead.c)
add_library (Cl SHARED msg_unpack.cc cl_msg_unknown_pack.cc)

target_link_libraries (Cl)
install(TARGETS PmRead DESTINATION bin)
install(TARGETS Cl DESTINATION lib)
