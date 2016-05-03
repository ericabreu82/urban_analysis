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


#include <memory>
#include <set>
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
      INPUT_NO_DATA = 0, INPUT_OTHER = 1, INPUT_WATER = 2, INPUT_URBAN = 3
    };

    enum OutputUrbanClasses
    {
      OUTPUT_NO_DATA = 0, OUTPUT_URBAN = 1, OUTPUT_SUB_URBAN = 2, OUTPUT_RURAL = 3, OUTPUT_URBANIZED_OS = 4, OUTPUT_SUBURBAN_ZONE_OPEN_AREA = 5, OUTPUT_RURAL_OS = 6, OUTPUT_WATER = 7
    };

    struct UrbanRasters
    {
      UrbanRasters()
        : m_urbanizedAreaRaster(0)
        , m_urbanFootprintRaster(0)
      {

      }

      std::auto_ptr<te::rst::Raster> m_urbanizedAreaRaster;
      std::auto_ptr<te::rst::Raster> m_urbanFootprintRaster;
    };

    TEGROWTHEXPORT void init();

    TEGROWTHEXPORT void finalize();

    TEGROWTHEXPORT te::rst::Raster* openRaster(const std::string& fileName);

    TEGROWTHEXPORT te::rst::Raster* createRaster(const std::string& fileName, te::rst::Raster* raster);

    //!< Returns all the pixels within the given radius
    TEGROWTHEXPORT std::vector<short> getPixelsWithinRadious(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn, double radius);

    //!< Returns all the adjacent pixels
    TEGROWTHEXPORT std::vector<short> getAdjacentPixels(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn);

    //Calculate the urbanized area value based in the value of center pixel and in the value of the adjacent pixels
    TEGROWTHEXPORT double calculateUrbanizedArea(short centerPixelValue, const std::vector<short>& vecPixels, double& permUrb);

    //Calculate the urban footprint value based in the value of center pixel and in the value of the adjacent pixels
    TEGROWTHEXPORT double calculateUrbanFootprint(short centerPixelValue, const std::vector<short>& vecPixels, double& permUrb);

    //Calculate the urban open area value based in the value of center pixel and in the value of the adjacent pixels
    TEGROWTHEXPORT double calculateUrbanOpenArea(short centerPixelValue, const std::vector<short>& vecPixels);

    TEGROWTHEXPORT bool calculateEdge(te::rst::Raster* raster, size_t column, size_t line);

    //this reclassification analyses the entire raster, where the output will be 1 if the pixel is urban and no_data if the pixel is not urban
    TEGROWTHEXPORT te::rst::Raster* filterUrbanPixels(te::rst::Raster* raster, const std::string& outputFileName);

    //!< Search for all the gaps (holes) that [optionally] have area smaller then the given reference area
    TEGROWTHEXPORT std::vector<te::gm::Geometry*> getGaps(const std::vector<te::gm::Geometry*>& vecPolygons, double area = 0.);

    //!< For each region, creates a new group using sequential values
    TEGROWTHEXPORT te::rst::Raster* createDistinctGroups(te::rst::Raster* inputRaster, const std::string& outputFileName);

    //!< DETERMINE EDGE OPEN AREA (100 meter buffer around built-up)
    TEGROWTHEXPORT std::set<double> detectEdgeOpenAreaGroups(te::rst::Raster* newDevRaster, te::rst::Raster* otherDevGroupedRaster, te::rst::Raster* footprintRaster);

    //compare the images in two diferrent times, creating two new classified images
    TEGROWTHEXPORT void compareRasters(te::rst::Raster* rasterT1, te::rst::Raster* rasterT2, const std::string& infillRasterFileName, const std::string& otherDevRasterFileName);

    TEGROWTHEXPORT te::rst::Raster* classifyNewDevelopment(te::rst::Raster* infillRaster, te::rst::Raster* otherDevGroupedRaster, const std::set<double>& setEdgesOpenAreaGroups, const std::string& outputRasterFileName);

  }
}

#endif //__URBANANALYSIS_INTERNAL_GROWTH_UTILS_H
