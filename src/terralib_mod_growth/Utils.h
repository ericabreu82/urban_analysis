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

#ifndef __URBANANALYSIS_INTERNAL_GROWTH_UTILS_H
#define __URBANANALYSIS_INTERNAL_GROWTH_UTILS_H

#include "Config.h"

#include <boost/numeric/ublas/matrix.hpp>

#include <string>
#include <vector>

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
    enum InputUrbanClasses
    {
      INPUT_NO_DATA = 0, INPUT_URBAN = 1, INPUT_WATER = 2, INPUT_OTHER = 3
    };

    enum OutputUrbanClasses
    {
      OUTPUT_NO_DATA = 0, OUTPUT_URBAN = 1, OUTPUT_SUB_URBAN = 2, OUTPUT_RURAL = 3, OUTPUT_URBANIZED_OS = 4, OUTPUT_RURAL_OS = 6, OUTPUT_WATER = 7
    };

    TEGROWTHEXPORT void init();

    TEGROWTHEXPORT void finalize();

    TEGROWTHEXPORT te::rst::Raster* openRaster(const std::string& fileName);

    TEGROWTHEXPORT te::rst::Raster* createRaster(const std::string& fileName, te::rst::Raster* raster);

    //!< Returns all the pixels within the given radious
    TEGROWTHEXPORT std::vector<short> getPixelsWithinRadious(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn, double radius);

    //Calculate the urban class value based in the value of center pixel and in the value of the adjacent pixels
    TEGROWTHEXPORT double calculateUrbanClass(short centerPixelValue, const std::vector<short>& vecPixels, double& permUrb);

    TEGROWTHEXPORT bool calculateEdge(te::rst::Raster* raster, size_t column, size_t line);

    //!< Search for all the gaps (holes) that [optionally] have area smaller then the given reference area
    TEGROWTHEXPORT std::vector<te::gm::Geometry*> getGaps(const std::vector<te::gm::Geometry*>& vecPolygons, double area = 0.);
  }
}

#endif //__URBANANALYSIS_INTERNAL_GROWTH_UTILS_H
