cmake_minimum_required (VERSION 3.8)

set (CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/MyOverrides.cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
include(MyCodeMacros.cmake)
include(MyGlobalImports.cmake)
include(MyLocalImports.cmake)
include(MyPrecompiledHeader.cmake)
include(MySpecial.cmake)

project ("Dev_Audio")
my_special1() 

add_subdirectory ("opusenc_example")
return()

add_subdirectory ("opusenc_example")
return()

add_subdirectory ("opus-test")
return()

add_subdirectory ("audio-test")
add_subdirectory ("Test")
return()


