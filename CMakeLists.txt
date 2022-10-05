cmake_minimum_required (VERSION 3.8)

set (CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/MyOverrides.cmake")
include(MyCodeMacros.cmake)
include(MyGlobalImports.cmake)
include(MyLocalImports.cmake)
include(MyPrecompiledHeader.cmake)
include(MySpecial.cmake)

project ("Dev_Audio")
my_special1() 

add_subdirectory ("opus-test")
return()

add_subdirectory ("audio-test")
add_subdirectory ("Test")
return()

add_subdirectory ("CCLib")
add_subdirectory ("ShareLib")
add_subdirectory ("RingWriter")
add_subdirectory ("RingReader1")
add_subdirectory ("RingReader2")
return()

add_subdirectory ("Counter")
return()

add_subdirectory ("CCLib")
add_subdirectory ("ShareLib")
add_subdirectory ("RingWriter")
add_subdirectory ("RingReader2")
return()

add_subdirectory ("Modulo")
return()

add_subdirectory ("CCLib")
add_subdirectory ("Concurrent")
add_subdirectory ("Test")
return()

add_subdirectory ("CCLib")
add_subdirectory ("ShareLib")
add_subdirectory ("RingWriter")
add_subdirectory ("RingReader")
return()

add_subdirectory ("CCLib")
add_subdirectory ("Concurrent")
add_subdirectory ("Test")
return()

add_subdirectory ("CCLib")
add_subdirectory ("Concurrent")
add_subdirectory ("LockFree")
add_subdirectory ("MyBlock")
add_subdirectory ("Test")


