INCLUDE(${CMAKE_BINARY_DIR}/CPackConfig)

FOREACH(comp ${COMPS})
  SET(CMDS ${CMDS}  
  COMMAND set DESTDIR=testinstall/${comp}
  COMMAND ${CMAKE_COMMAND}
  -DCMAKE_INSTALL_CONFIG_NAME=${CMAKE_CFG_INTDIR} 
  -DCMAKE_INSTALL_COMPONENT=${comp} 
  -DCMAKE_INSTALL_PREFIX=
  -P ${CMAKE_BINARY_DIR}/cmake_install.cmake 
  )
ENDFOREACH()

MACRO(MAKE_WIX_IDENTIFIER str varname)
  STRING(REPLACE "/" "." ${varname} "${str}")
  STRING(REPLACE "-" "_" ${varname} "${${varname}}")
ENDMACRO()

FUNCTION(TRAVERSE_FILES dir topdir file file_comp dir_root)
  FILE(GLOB all_files ${dir}/*)
  IF(NOT all_files)
    RETURN()
  ENDIF()
  FILE(RELATIVE_PATH dir_rel ${topdir} ${dir})
  IF(dir_rel)
   MAKE_DIRECTORY(${dir_root}/${dir_rel})
   MAKE_WIX_IDENTIFIER("${dir_rel}" id)
   FILE(APPEND ${file} "<DirectoryRef Id='directory.${id}'>\n")
  ELSE()
   FILE(APPEND ${file} "<DirectoryRef Id='INSTALLDIR'>\n")
  ENDIF()
  
  FOREACH(f ${all_files})
    IF(NOT IS_DIRECTORY ${f})
      FILE(RELATIVE_PATH rel ${topdir} ${f})
      MAKE_WIX_IDENTIFIER("${rel}" id)
      FILE(TO_NATIVE_PATH ${f} f_native)
      FILE(APPEND ${file} "  <Component Id='component.${id}' Guid='*'>\n")
      FILE(APPEND ${file} "    <File Id='file.${id}' KeyPath='yes' Source='${f_native}'/>\n")
      FILE(APPEND ${file} "  </Component>\n")
      FILE(APPEND ${file_comp} "  <ComponentRef Id='component.${id}'/>\n")
    ENDIF()
  ENDFOREACH()
  FILE(APPEND ${file} "</DirectoryRef>\n")
  FOREACH(f ${all_files})
    IF(IS_DIRECTORY ${f})
      TRAVERSE_FILES(${f} ${topdir} ${file} ${file_comp} ${dir_root})
    ENDIF()
  ENDFOREACH()
ENDFUNCTION()

FUNCTION(TRAVERSE_DIRECTORIES dir topdir file prefix)
  FILE(RELATIVE_PATH rel ${topdir} ${dir})
  IF(rel)
    MAKE_WIX_IDENTIFIER("${rel}" id)
    GET_FILENAME_COMPONENT(name ${dir} NAME)
    FILE(APPEND ${file} "${prefix}<Directory Id='directory.${id}' Name='${name}'>\n")
  ENDIF()
  FILE(GLOB all_files ${dir}/*)
    FOREACH(f ${all_files})
    IF(IS_DIRECTORY ${f})
      TRAVERSE_DIRECTORIES(${f} ${topdir} ${file} "${prefix}  ")
    ENDIF()
  ENDFOREACH()
  IF(rel)
    FILE(APPEND ${file} "</Directory>\n")
  ENDIF()
ENDFUNCTION()


GET_FILENAME_COMPONENT(abs . ABSOLUTE)
FOREACH(d ${DIRS})
  GET_FILENAME_COMPONENT(d ${d} ABSOLUTE)
  GET_FILENAME_COMPONENT(d_name ${d} NAME)
  FILE(WRITE 
  ${abs}/${d_name}.wxs  "<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>\n<Fragment>\n")
  FILE(WRITE 
  ${abs}/${d_name}_component_group.wxs  "<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>\n<Fragment>\n<ComponentGroup Id='componentgroup.${d_name}'>\n")
  TRAVERSE_FILES(${d} ${d} ${abs}/${d_name}.wxs ${abs}/${d_name}_component_group.wxs "${abs}/dirs")
  FILE(APPEND  ${abs}/${d_name}.wxs   " </Fragment>\n</Wix>")
  FILE(APPEND  ${abs}/${d_name}_component_group.wxs   "</ComponentGroup>\n</Fragment>\n</Wix>") 
ENDFOREACH()
FILE(WRITE directories.wxs "<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>\n<Fragment>\n<DirectoryRef Id='INSTALLDIR'>\n")
TRAVERSE_DIRECTORIES(${abs}/dirs ${abs}/dirs directories.wxs "")
FILE(APPEND directories.wxs "</DirectoryRef>\n</Fragment>\n</Wix>\n")

