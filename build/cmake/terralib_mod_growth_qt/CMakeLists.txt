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
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -DTEGROWTHQTDLL -D_CRT_SECURE_NO_WARNINGS)
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

file(GLOB GROWTH_SRC_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/terralib_mod_growth_qt/*.cpp)
file(GLOB GROWTH_HDR_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/terralib_mod_growth_qt/*.h)
file(GLOB GROWTH_UI_FILES ${URBANANALYSIS_ABSOLUTE_ROOT_DIR}/src/terralib_mod_growth_qt/ui/*.ui)

if(Qt5_FOUND)
  QT5_WRAP_UI(GROWTH_UI_GEN_FILES ${GROWTH_UI_FILES})
endif()

source_group("Source Files"  FILES ${GROWTH_SRC_FILES})
source_group("Header Files"  FILES ${GROWTH_HDR_FILES})

add_library(terralib_mod_growth_qt SHARED ${GROWTH_SRC_FILES} ${GROWTH_HDR_FILES} ${GROWTH_UI_GEN_FILES})

set(GROWTH_LIBRARIES_DEPENDENCIES "terralib_mod_growth")
list(APPEND GROWTH_LIBRARIES_DEPENDENCIES "terralib_mod_qt_widgets")

target_link_libraries(terralib_mod_growth_qt ${GROWTH_LIBRARIES_DEPENDENCIES})

set_target_properties(terralib_mod_growth_qt
                      PROPERTIES VERSION ${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR}
                                 SOVERSION ${URBANANALYSIS_VERSION_MAJOR}.${URBANANALYSIS_VERSION_MINOR})
								 
if(Qt5_FOUND)
  qt5_use_modules(terralib_mod_growth_qt Widgets)
endif()

install(TARGETS terralib_mod_growth_qt
        EXPORT urbanAnalysis-targets
        RUNTIME DESTINATION ${URBANANALYSIS_DESTINATION_RUNTIME} COMPONENT runtime
        LIBRARY DESTINATION ${URBANANALYSIS_DESTINATION_LIBRARY} COMPONENT runtime
        ARCHIVE DESTINATION ${URBANANALYSIS_DESTINATION_ARCHIVE} COMPONENT runtime)

install(FILES ${GROWTH_HDR_FILES}
        DESTINATION ${URBANANALYSIS_DESTINATION_HEADERS}/terralib_mod_growth_qt COMPONENT devel)

export(TARGETS terralib_mod_growth_qt APPEND FILE ${CMAKE_BINARY_DIR}/urbanAnalysis-exports.cmake)
