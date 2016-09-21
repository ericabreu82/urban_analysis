#
#  Copyright (C) 2008-2014 National Institute For Space Research (INPE) - Brazil.
#
#  This file is part of the TerraLib - a Framework for building GIS enabled applications.
#
#  TerraLib is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License,
#  or (at your option) any later version.
#
#  TerraLib is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with TerraLib. See COPYING. If not, write to
#  TerraLib Team at <terralib-team@terralib.org>.
#
#
#  Description: Build configuration for the Geometry Module.
#
#  Author: 	Eric Silva Abreu <eric.abreu@dpi.inpe.br>
#			Mário Rocco Pettinati <mariopettinati@gmail.com>
#          

if(WIN32)
  add_definitions(-DURBANANALYSISPLUGIN -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DBOOST_LOG_DYN_LINK)
endif()

include_directories(
  ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src
  ${terralib_INCLUDE_DIRS}
  ${terralib_DIR}
  ${Boost_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}/qwt
)

if(Qt5_FOUND)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

file(GLOB TERRAVIEWPLUGIN_SRC_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/urbanAnalysis_plugin/*.cpp)
file(GLOB TERRAVIEWPLUGIN_HDR_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/urbanAnalysis_plugin/*.h)
file(GLOB TERRAVIEWPLUGIN_UI_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/urbanAnalysis_plugin/ui/*.ui)

if(Qt5_FOUND)
  QT5_WRAP_UI(TERRAVIEWPLUGIN_UI_GEN_FILES ${TERRAVIEWPLUGIN_UI_FILES})
endif()

source_group("Source Files"  FILES ${TERRAVIEWPLUGIN_SRC_FILES})
source_group("Header Files"  FILES ${TERRAVIEWPLUGIN_HDR_FILES})

add_library(urbanAnalysis_plugin SHARED ${TERRAVIEWPLUGIN_SRC_FILES} ${TERRAVIEWPLUGIN_HDR_FILES} ${TERRAVIEWPLUGIN_UI_GEN_FILES})

set(TERRAVIEWPLUGIN_LIBRARIES_DEPENDENCIES "terralib_mod_growth")
list(APPEND TERRAVIEWPLUGIN_LIBRARIES_DEPENDENCIES "terralib_mod_growth_qt")
list(APPEND TERRAVIEWPLUGIN_LIBRARIES_DEPENDENCIES "terralib_mod_qt_widgets")
list(APPEND TERRAVIEWPLUGIN_LIBRARIES_DEPENDENCIES "terralib_mod_plugin")
list(APPEND TERRAVIEWPLUGIN_LIBRARIES_DEPENDENCIES "terralib_mod_qt_apf")

target_link_libraries(urbanAnalysis_plugin ${TERRAVIEWPLUGIN_LIBRARIES_DEPENDENCIES})

set_target_properties(urbanAnalysis_plugin
                      PROPERTIES VERSION ${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR}
                                 SOVERSION ${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR})
								 
if(Qt5_FOUND)
  qt5_use_modules(urbanAnalysis_plugin Widgets)
endif()

install(TARGETS urbanAnalysis_plugin
        EXPORT urbanAnalysis-targets
        RUNTIME DESTINATION ${URBANANALYSIS_DESTINATION_RUNTIME} COMPONENT runtime
        LIBRARY DESTINATION ${URBANANALYSIS_DESTINATION_LIBRARY} COMPONENT runtime
        ARCHIVE DESTINATION ${URBANANALYSIS_DESTINATION_ARCHIVE} COMPONENT runtime)

install(FILES ${TERRAVIEWPLUGIN_HDR_FILES}
        DESTINATION ${URBANANALYSIS_DESTINATION_HEADERS}/urbanAnalysis_plugin COMPONENT devel)

export(TARGETS urbanAnalysis_plugin APPEND FILE ${CMAKE_BINARY_DIR}/urbanAnalysis-exports.cmake)

#add_custom_command(
#      TARGET urbanAnalysis_plugin
#      POST_BUILD
#	  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/Debug/urbanAnalysis_plugind.dll" "${terralib_DIR}/debug"
#      
#      COMMENT "Copying plugin files..."
#      VERBATIM
#    )
