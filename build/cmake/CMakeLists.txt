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
set(URBANANALYSIS_VERSION_MINOR 2)
set(URBANANALYSIS_VERSION_PATCH 0)
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
 add_subdirectory(urbanAnalysis_plugin)

TeInstallQt5Plugins()

if(WIN32)
  get_filename_component(_uaShare ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/share/terralib DIRECTORY)
  get_filename_component(_teShare ${terralib_INCLUDE_DIRS}/../share/terralib DIRECTORY) 
  get_filename_component(_qtPlugins ${terralib_INCLUDE_DIRS}/../qtplugins/platforms DIRECTORY) 

  install (
    FILES ${_teplugins} ${_dep}
    DESTINATION bin
    COMPONENT runtime
  )  
  
  install(
    DIRECTORY ${_uaShare}
    DESTINATION .
    COMPONENT runtime
  )
  
  install(
    DIRECTORY ${_teShare}
    DESTINATION .
    COMPONENT runtime
  )
   
  file(GLOB dll_list ${terralib_DIR}/../../../bin/*.dll)
  file(GLOB exe_list ${terralib_DIR}/../../../bin/*.exe)
  
  set(_teBins ${dll_list} ${exe_list})
  
  install(
    FILES ${_teBins}
    DESTINATION bin
    COMPONENT runtime
  )
endif()


include(InstallRequiredSystemLibraries)

# CPack
set(CPACK_GENERATOR NSIS)
set(CPACK_PACKAGE_NAME "UrbanAnalysis")
set(CPACK_PACKAGE_VENDOR "Terralib3rdParty")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Urban Analysis")
set(CPACK_PACKAGE_VERSION "1.2.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Urban Analysis")
SET(CPACK_NSIS_MODIFY_PATH OFF)

#for the sortcuts
set(shortCutName "Urban Analysis ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}${bitName}") 
set(shortCutNameTerraview "TerraView 5.2.0")

set(shortcutUA_startmenu "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER/${shortCutName}.lnk\\\" \\\"$INSTDIRb\\\\bin\\\\urbanAnalysisApp.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\share\\\\terralib\\\\images\\\\ico\\\\terralib-globe.ico\\\"")
set(shortcutUA_desktop "CreateShortCut \\\"$DESKTOP\\\\${shortCutName}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\urbanAnalysisApp.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\share\\\\terralib\\\\images\\\\ico\\\\terralib-globe.ico\\\"")
set(shortcutTV_startmenu "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER/${shortCutNameTerraview}.lnk\\\" \\\"$INSTDIRb\\\\bin\\\\terraview.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\share\\\\terralib\\\\images\\\\ico\\\\terralib-globe.ico\\\"")
set(shortcutTV_desktop "CreateShortCut \\\"$DESKTOP\\\\${shortCutNameTerraview}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\terraview.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\share\\\\terralib\\\\images\\\\ico\\\\terralib-globe.ico\\\"")
set(full_shortcut_list "${shortcutUA_startmenu};${shortcutUA_desktop};${shortcutTV_startmenu};${shortcutTV_desktop}")

set(CPACK_PACKAGE_EXECUTABLES "urbanAnalysisApp;${shortCutName}" "terraview;${shortCutNameTerraview}")
set(CPACK_NSIS_CREATE_ICONS_EXTRA "SetOutPath \\\"$INSTDIR\\\\bin\\\";${full_shortcut_list}")

list (APPEND CPACK_NSIS_DELETE_ICONS_EXTRA "Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\${shortCutName}.lnk\\\";Delete \\\"$DESKTOP\\\\${shortCutName}.lnk\\\"")
list (APPEND CPACK_NSIS_DELETE_ICONS_EXTRA ";Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\${shortCutNameTerraview}.lnk\\\";Delete \\\"$DESKTOP\\\\${shortCutNameTerraview}.lnk\\\"")

INCLUDE(CPack)

