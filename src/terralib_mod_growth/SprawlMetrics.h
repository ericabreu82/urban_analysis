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
\file urban_analysis/src/growth/UrbanGrowth.h

\brief This class represents a list of algorithms to calculate urban growth
*/

#ifndef __URBANANALYSIS_INTERNAL_SPRAWL_METRICS_H
#define __URBANANALYSIS_INTERNAL_SPRAWL_METRICS_H

#include "Config.h"

#include <terralib/raster/Raster.h>


#include <memory>
#include <set>
#include <string>
#include <vector>

namespace te
{
  namespace urban
  {
    //calculates the centroid of the urban pixels. it also calculates the total area of the urban pixels (classes = 1, 2, 4 and 5)
    TEGROWTHEXPORT void calculateUrbanCentroid(te::rst::Raster* raster, double& urbanArea, double& centroidX, double& centroidY);

    //calculates the cohesion index
    TEGROWTHEXPORT void calculateCohesionIndex(te::rst::Raster* raster, double& averageDistance, double& averageDistanceSquare);

    //calculates the depth and the girth indexes
    TEGROWTHEXPORT void calculateDepthIndex(te::rst::Raster* raster, double radius, double& depthIndex, double& girthIndex);
  }
}

#endif //__URBANANALYSIS_INTERNAL_SPRAWL_METRICS_H
