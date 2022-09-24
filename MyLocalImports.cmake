#*******************************************************************************
#*******************************************************************************
#*******************************************************************************

function(my_lib_import_CCLib _target)

   target_link_libraries(${_target} CCLib)

endfunction()

function(my_inc_import_CCLib _target)

   target_include_directories(${_target} PRIVATE $<TARGET_PROPERTY:CCLib,INTERFACE_INCLUDE_DIRECTORIES>)

endfunction()

#*******************************************************************************
#*******************************************************************************
#*******************************************************************************

function(my_lib_import_ShareLib _target)

   target_link_libraries(${_target} ShareLib)

endfunction()

function(my_inc_import_ShareLib _target)

   target_include_directories(${_target} PRIVATE $<TARGET_PROPERTY:ShareLib,INTERFACE_INCLUDE_DIRECTORIES>)

endfunction()

#*******************************************************************************
#*******************************************************************************
#*******************************************************************************

