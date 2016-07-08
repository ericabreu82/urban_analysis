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
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -DURBANANALYSISAPP -D_CRT_SECURE_NO_WARNINGS)
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

file(GLOB URBANANALYSIS_SRC_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/urbanAnalysis_app/*.cpp)
file(GLOB URBANANALYSIS_HDR_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/urbanAnalysis_app/*.h)
file(GLOB GROWTH_RESOURCES_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/resources/*.qrc)

source_group("Source Files"  FILES ${URBANANALYSIS_SRC_FILES})
source_group("Header Files"  FILES ${URBANANALYSIS_HDR_FILES})


set(URBANANALYSIS_DEPENDENCIES "terralib_mod_growth")
list(APPEND URBANANALYSIS_DEPENDENCIES "terralib_mod_growth_qt")

if(Qt5_FOUND)
  QT5_ADD_RESOURCES(GROWTH_RESOURCES ${GROWTH_RESOURCES_FILES})
  
  add_executable(urbanAnalysisApp ${URBANANALYSIS_SRC_FILES} ${URBANANALYSIS_HDR_FILES} ${GROWTH_RESOURCES})
  
  target_link_libraries(urbanAnalysisApp ${URBANANALYSIS_DEPENDENCIES})
  
  SET (QT_USE_QTMAIN TRUE)
  
  set_target_properties(urbanAnalysisApp PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  
endif()

set_target_properties(urbanAnalysisApp
                      PROPERTIES VERSION ${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR}
                                 SOVERSION ${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR})

install(TARGETS urbanAnalysisApp
        EXPORT urbanAnalysis-targets
        RUNTIME DESTINATION ${URBANANALYSIS_DESTINATION_RUNTIME} COMPONENT runtime
        LIBRARY DESTINATION ${URBANANALYSIS_DESTINATION_LIBRARY} COMPONENT runtime
        ARCHIVE DESTINATION ${URBANANALYSIS_DESTINATION_ARCHIVE} COMPONENT runtime)

install(FILES ${URBANANALYSIS_HDR_FILES}
        DESTINATION ${URBANANALYSIS_DESTINATION_HEADERS}/urbanAnalysisApp COMPONENT devel)
