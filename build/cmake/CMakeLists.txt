cmake_minimum_required(VERSION 2.8.12)

project(urban_analysis)

cmake_policy(SET CMP0020 NEW)

if(${CMAKE_MAJOR_VERSION} GREATER 2)
  cmake_policy(SET CMP0007 NEW)
  cmake_policy(SET CMP0026 OLD)
  cmake_policy(SET CMP0042 NEW)  
  cmake_policy(SET CMP0043 NEW)  
  cmake_policy(SET CMP0053 OLD)  
endif()

set(URBANANALYSIS_ABSOLUTE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

include_directories(${CMAKE_BINARY_DIR})

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

#
# main variables for the build script
#

# variables that control the system versioning
set(URBANANALYSIS_VERSION_MAJOR 1)
set(URBANANALYSIS_VERSION_MINOR 1)
set(URBANANALYSIS_VERSION_PATCH 1)
set(URBANANALYSIS_VERSION_STATUS "rc")
set(URBANANALYSIS_VERSION_STRING "${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR}.${URBANANALYSIS_VERSION_PATCH}-${URBANANALYSIS_VERSION_STATUS}")
set(URBANANALYSIS_VERSION "0x050000")
set(URBANANALYSIS_PKG_VERSION ${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR}.${URBANANALYSIS_VERSION_PATCH})

find_package(terralib 5.2 REQUIRED)

find_package(Boost REQUIRED)

find_package(Qt5 5.1 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

find_package(Qt5LinguistTools REQUIRED COMPONENTS lupdate lrelease)

add_definitions(-D_SCL_SECURE_NO_WARNINGS -DBOOST_ALL_NO_LIB -DBOOST_FILESYSTEM_VERSION=3 -DBOOST_UBLAS_TYPE_CHECK=0)

set(CMAKE_AUTOMOC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(URBANANALYSIS_BASE_DESTINATION_DIR "")

set (TE_3RDPARTY "$ENV{TE_3RDPARTY}" CACHE PATH "Where to find TerraLib 3rdParty?")

if(NOT DEFINED URBANANALYSIS_DESTINATION_LIBRARY)
  set(URBANANALYSIS_DESTINATION_LIBRARY "${URBANANALYSIS_BASE_DESTINATION_DIR}lib")
endif()

if(NOT DEFINED URBANANALYSIS_DESTINATION_RUNTIME)
  set(URBANANALYSIS_DESTINATION_RUNTIME "${URBANANALYSIS_BASE_DESTINATION_DIR}bin")
endif()

if(NOT DEFINED URBANANALYSIS_DESTINATION_ARCHIVE)
  set(URBANANALYSIS_DESTINATION_ARCHIVE "${URBANANALYSIS_BASE_DESTINATION_DIR}lib")
endif()

if(NOT DEFINED URBANANALYSIS_DESTINATION_HEADERS)
  set(URBANANALYSIS_DESTINATION_HEADERS "${URBANANALYSIS_BASE_DESTINATION_DIR}include")
endif()

if(NOT DEFINED URBANANALYSIS_DESTINATION_SHARE)
  set(URBANANALYSIS_DESTINATION_SHARE "${URBANANALYSIS_BASE_DESTINATION_DIR}share")
endif()

include_directories(${terralib_DIR}
          ${terralib_INCLUDE_DIRS}
          ${terralib_INCLUDE_DIRS}/terralib
		  ${Boost_INCLUDE_DIR}
          )

 add_subdirectory(terralib_mod_growth)
 add_subdirectory(terralib_mod_growth_qt)
 add_subdirectory(urbanAnalysis_app)

# Track dependencies
#MACRO ( TeGetPlugins _plugins )
#  foreach(plugin ${TERRALIB_PLUGINS})
#    get_target_property(_loc ${plugin} LOCATION_RELEASE)
#    list(APPEND ${_plugins} ${_loc})
    
#    get_target_property(_locd ${plugin} LOCATION_DEBUG)
#    list(APPEND ${_plugins} ${_locd})
#  endforeach()
#ENDMACRO(TeGetPlugins)

TeInstallQt5Plugins()
#TeInstallQt5Plugins("qgif;qico;qjpeg;qmng;qtiff;qsvg;qsvgicon;qsqlite;qwindows;qcocoa")  

#set(_teplugins "")

#TeGetPlugins (_teplugins)

if(WIN32)
  get_filename_component(_teShare ${terralib_INCLUDE_DIRS}/../share/terralib DIRECTORY) 
  get_filename_component(_qtPlugins ${terralib_INCLUDE_DIRS}/../qtplugins/platforms DIRECTORY) 

  install (
    FILES ${_teplugins} ${_dep}
    DESTINATION bin
    COMPONENT runtime
  )  
  
  install(
    DIRECTORY ${_teShare}
    DESTINATION .
    COMPONENT runtime
  )
  
    
  #install(
  #  DIRECTORY ${_qtPlugins}
  #  DESTINATION .
  #  COMPONENT runtime
  #)
  
  file(GLOB _teBins ${terralib_DIR}/../../../bin/*.dll)
  
  install(
    FILES ${_teBins}
    DESTINATION bin
    COMPONENT runtime
  )
endif()


include(InstallRequiredSystemLibraries)

#configure_file(${CMAKE_SOURCE_DIR}/urban_analysis-fix-bundle.cmake.in
#               ${CMAKE_BINARY_DIR}/FixBundleUrban_analysis.cmake @ONLY)

#install(SCRIPT ${CMAKE_BINARY_DIR}/FixBundleUrban_analysis.cmake
 #       COMPONENT runtime)
		
#set(GDAL_DATA_DIR ${TE_3RDPARTY}/data)

#install(DIRECTORY ${GDAL_DATA_DIR}
       #DESTINATION share
       #COMPONENT runtime) 

# CPack
set(CPACK_GENERATOR NSIS)
set(CPACK_PACKAGE_NAME "UrbanAnalysis")
set(CPACK_PACKAGE_VENDOR "Terralib3rdParty")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Urban Analysis")
set(CPACK_PACKAGE_VERSION "1.1.1")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "UrbanAnalysisApp")
SET(CPACK_NSIS_MODIFY_PATH OFF)

#for the sortcuts
set(shortCutName "urban_analysis_${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR}_${CPACK_PACKAGE_VERSION_PATCH}${bitName}")

set(CPACK_PACKAGE_EXECUTABLES "urbanAnalysisApp;${shortCutName}")
  
set(CPACK_NSIS_CREATE_ICONS_EXTRA "SetOutPath \\\"$INSTDIR\\\\bin\\\";CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER/${shortCutName}.lnk\\\" \\\"$INSTDIRb\\\\bin\\\\urbanAnalysisApp.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\share\\\\terralib\\\\images\\\\ico\\\\terralib-globe.ico\\\";CreateShortCut \\\"$DESKTOP\\\\${shortCutName}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\urbanAnalysisApp.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\share\\\\terralib\\\\images\\\\ico\\\\terralib-globe.ico\\\"")

list (APPEND CPACK_NSIS_DELETE_ICONS_EXTRA "Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\${shortCutName}.lnk\\\";Delete \\\"$DESKTOP\\\\${shortCutName}.lnk\\\"")

INCLUDE(CPack)

