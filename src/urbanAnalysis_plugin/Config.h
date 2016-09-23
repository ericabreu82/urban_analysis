/*  Copyright (C) 2008-2014 National Institute For Space Research (INPE) - Brazil.

    This file is part of the TerraLib - a Framework for building GIS enabled applications.

    TerraLib is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    TerraLib is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TerraLib. See COPYING. If not, write to
    TerraLib Team at <terralib-team@terralib.org>.
 */

/*!
  \file analiseurbana/src/analiseurbana/Config.h

  \brief Configuration flags for the analiseurbana Qt plugin.
*/

#ifndef __URBANANALYSIS_INTERNAL_URBAN_ANALYSIS_PLUGIN_CONFIG_H
#define __URBANANALYSIS_INTERNAL_URBAN_ANALYSIS_PLUGIN_CONFIG_H

#ifdef WIN32
  #ifdef URBANANALYSISPLUGIN
    #define URBANANALYSISPLUGINEXPORT  __declspec(dllexport)   // export DLL information
  #else
    #define URBANANALYSISPLUGINEXPORT  __declspec(dllimport)   // import DLL information
  #endif 
#else
  #define URBANANALYSISPLUGINEXPORT
#endif

#endif  // __URBANANALYSIS_INTERNAL_URBAN_ANALYSIS_PLUGIN_CONFIG_H

