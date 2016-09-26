/*  Copyright (C) 2011-2012 National Institute For Space Research (INPE) - Brazil.

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
\file urban_analysis/src/growth/qt/Utils.h

\brief This class represents the Urban Analysis utils functions. 
*/

#ifndef __URBANANALYSIS_INTERNAL_URBANALYSIS_UTILS_H
#define __URBANANALYSIS_INTERNAL_URBANALYSIS_UTILS_H

#include "Config.h"

// Terralib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <memory>

namespace te
{
  namespace urban
  {
    namespace qt
    {
      TEGROWTHQTEXPORT te::map::AbstractLayerPtr CreateLayer(const std::string& path, const std::string& type);

      TEGROWTHQTEXPORT te::da::DataSourcePtr RegisterDataSource(const std::string& path, const std::string& type);

      TEGROWTHQTEXPORT std::string GenerateRandomId();
    }
  }
}

#endif //__URBANANALYSIS_INTERNAL_URBANALYSIS_UTILS_H
