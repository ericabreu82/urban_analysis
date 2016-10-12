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
#include "Utils.h"

#include <terralib/geometry/Coord2D.h>

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <map>

namespace te
{
  namespace gm
  {
    class Geometry;
  }
  namespace rst
  {
    class Raster;
  }

  namespace urban
  {
    struct IndexesParams
    {
      te::rst::Raster* m_urbanRaster; //urban classified raster
      te::gm::Coord2D m_centroidCBD; //the centroid CBD
      te::gm::Geometry* m_studyArea; //study area
      te::rst::Raster* m_landCoverRaster; //land cover raster
      te::rst::Raster* m_slopeRaster;//slope raster
      bool m_calculateProximity;
      bool m_calculateCohesion;
      bool m_calculateDepth;

      IndexesParams()
        : m_calculateProximity(true)
        , m_calculateCohesion(true)
        , m_calculateDepth(true)
      {}
    };

    //calculates the centroid of the urban pixels. it also calculates the total area of the urban pixels (classes = 1, 2, 4 and 5)
    TEGROWTHEXPORT void calculateUrbanCentroid(te::rst::Raster* urbanRaster, double& urbanArea, te::gm::Coord2D& centroid);

    //calculates the proximity index
    TEGROWTHEXPORT UrbanIndexes calculateProximityIndex(te::rst::Raster* urbanRaster, te::rst::Raster* landCoverRaster, te::rst::Raster* slopeRaster, const te::gm::Coord2D& centroidCBD, const te::gm::Coord2D& centroidUrban, double radius, double urbanAreaHA);

    //calculates the cohesion index
    TEGROWTHEXPORT UrbanIndexes calculateCohesionIndex(te::rst::Raster* urbanRaster, double radius);

    //calculates the depth and the girth indexes
    TEGROWTHEXPORT UrbanIndexes calculateDepthIndex(te::rst::Raster* urbanRaster, te::gm::Geometry* studyArea, double radius);

    //calculates the depth and the girth indexes
    TEGROWTHEXPORT UrbanIndexes calculateIndexes(const IndexesParams& params);
  }
}

#endif //__URBANANALYSIS_INTERNAL_SPRAWL_METRICS_H
