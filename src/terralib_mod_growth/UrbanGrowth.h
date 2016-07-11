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

#ifndef __URBANANALYSIS_INTERNAL_GROWTH_URBANGROWTH_H
#define __URBANANALYSIS_INTERNAL_GROWTH_URBANGROWTH_H

#include "Config.h"

#include "Utils.h"

#include <map>
#include <string>

namespace te
{
  namespace rst
  {
    class Raster;
  }

  namespace urban
  {
    typedef std::map<std::string, double> UrbanIndexes;
    typedef std::map<std::string, UrbanIndexes> UrbanSummary;

    //step 1 - this reclassification analyses the entire raster. Classify the urbanized area
    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> classifyUrbanizedArea(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius);

    //step 2 - this reclassification analyses the entire raster. Classify the urban footprint
    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> classifyUrbanFootprint(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius);

    //step 3 - this reclassification analyses the entire raster. Classify the urban open area
    TEGROWTHEXPORT void classifyUrbanOpenArea(te::rst::Raster* urbanFootprintRaster, double radius);

    //step 4 - this reclassification analyses the entire raster and returns a binary image containing the areas lower than 100 hectares that are completely sorrounded by urban areas
    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> identifyIsolatedOpenPatches(te::rst::Raster* raster, const std::string& outputPath, const std::string& outputPrefix);
    
    //step 5 - add isoleted patches to map
    TEGROWTHEXPORT void addIsolatedOpenPatches(te::rst::Raster* urbanRaster, te::rst::Raster* isolatedOpenPatchesRaster);

    //steps 4 and 5
    TEGROWTHEXPORT void classifyIsolatedOpenPatches(te::rst::Raster* raster, const std::string& outputPath, const std::string& outputPrefix);

    //the indexes calculation only considers the study area
    TEGROWTHEXPORT void calculateUrbanIndexes(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius, const std::string& spatialLimits, UrbanIndexes& urbanIndexes);

    TEGROWTHEXPORT UrbanRasters prepareRaster(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius, const std::string& outputPath, const std::string& outputPrefix);

    //step 9 - analyze new development
    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> compareRasterPeriods(const UrbanRasters& t1, const UrbanRasters& t2, const std::string& outputPath, const std::string& outputPrefix);
  }
}

#endif //__URBANANALYSIS_INTERNAL_GROWTH_URBANGROWTH_H
