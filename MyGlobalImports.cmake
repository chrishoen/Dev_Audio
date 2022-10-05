#*******************************************************************************
#*******************************************************************************
#*******************************************************************************

function(my_lib_import_RisLib _target)

   set (MyRisLibImportPath  "/mnt/c/Prime/AAA_Stenograph/NextGen/src_linux/gui/target/rislib/lib/arm/libRisLib.a")

   add_library(RisLib STATIC IMPORTED)
   set_target_properties(RisLib PROPERTIES IMPORTED_LOCATION ${MyRisLibImportPath})
   target_link_libraries(${_target} RisLib)
   target_link_libraries(${_target} pthread)
   target_link_libraries(${_target} rt)
   target_link_libraries(${_target} readline)

endfunction()

function(my_inc_import_RisLib _target)

#  set (MyRisLibIncludePath  "/mnt/c/Prime/AAA_Stenograph/NextGen/src_linux/gui/target/rislib/include")
   set (MyRisLibIncludePath  "/opt/prime/include/RisLib")
   target_include_directories(${_target} PUBLIC ${MyRisLibIncludePath})

endfunction()

#*******************************************************************************
#*******************************************************************************
#*******************************************************************************

function(my_lib_import_Audio _target)

   target_link_libraries(${_target} pulse)
   target_link_libraries(${_target} opusfile)

endfunction()

function(my_inc_import_Audio _target)

#  target_include_directories(${_target} PRIVATE "/opt/usr_data/sdk/sysroots/aarch64-fslc-linux/include/opus")
   target_include_directories(${_target} PRIVATE "/opt/prime/include/opus")

endfunction()


#*******************************************************************************
#*******************************************************************************
#*******************************************************************************

