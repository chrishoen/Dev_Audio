cmake_minimum_required(VERSION 3.10)
set (project "Test")
project(${project} CXX)
my_find_src_files (src_files ${project})
add_executable (${project} ${src_files})
my_add_compile_options (${project})
my_add_pch (${project})

my_inc_import_RisLib(${project})
my_lib_import_RisLib(${project})

if (MSVC)
   install(TARGETS ${project} DESTINATION "C:/aaa_prime/RisLib")
elseif (MYMODE STREQUAL "cpdev1" OR MYMODE STREQUAL "wsl") 
   install(TARGETS ${project} DESTINATION "/opt/prime/bin")
elseif (MYMODE STREQUAL "wsl-cpdev1")
   install(CODE "execute_process(COMMAND_ECHO STDOUT
      COMMAND scp ${CMAKE_CURRENT_BINARY_DIR}/${project} main@cpdev1:/opt/prime/bin)")
elseif (MYMODE STREQUAL "beagle")
   install(CODE "execute_process(COMMAND_ECHO STDOUT
      COMMAND c:/windows/system32/openssh/scp ${CMAKE_CURRENT_BINARY_DIR}/${project} debian@bbx:/opt/prime/bin)")
elseif (MYMODE STREQUAL "steno-arm")
   install(CODE "execute_process(COMMAND_ECHO STDOUT
      COMMAND scp ${CMAKE_CURRENT_BINARY_DIR}/${project} root@stenotgt:/opt/prime/bin)")
endif()
