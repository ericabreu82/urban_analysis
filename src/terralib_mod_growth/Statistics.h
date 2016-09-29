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
\file urban_analysis/src/growth/Statistics.h

\brief This class represents a list of algorithms to calculate statistics
*/

#ifndef __URBANANALYSIS_INTERNAL_STATISTICS_H
#define __URBANANALYSIS_INTERNAL_STATISTICS_H

#include "Config.h"

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <map>

namespace te
{
  namespace da
  {
    class DataSet;
    class DataSetType;
    class DataSource;
  }
  namespace gm
  {
    class Geometry;
  }
  namespace mem
  {
    class DataSet;
  }
  namespace rst
  {
    class Raster;
  }

  namespace urban
  {
    //calculates the statistics for a result image from urban growth method (infill, leapfrog, extension)
    TEGROWTHEXPORT void CalculateStatistics(te::rst::Raster* raster, te::da::DataSource* ds, const std::string& dataSetName, 
                                            const bool& calculateArea, const bool& calculateCount, const std::string& outPath, const std::string& outDataSetName);

    TEGROWTHEXPORT std::auto_ptr<te::da::DataSetType> createStatisticsDataSetType(te::rst::Raster* raster, std::string dataSetName, te::da::DataSetType* inputDsType,
                                                                                  const bool& calculateArea, const bool& calculateCount);

    /*! Function used to create the output data */
    TEGROWTHEXPORT std::auto_ptr<te::mem::DataSet> createStatisticsDataSet(te::rst::Raster* raster, te::da::DataSetType* dsType, te::da::DataSet* inputDs,
                                                                           const bool& calculateArea, const bool& calculateCount);

    /*! Function used to save the output dataset */
    TEGROWTHEXPORT void saveStatisticsDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType, te::da::DataSource* ds, std::string dataSetName);

    /*! Function used to calculate statistics */
    TEGROWTHEXPORT std::map<int, std::size_t> computeStatistics(te::rst::Raster* raster, te::gm::Geometry* geom);

  }
}

#endif //__URBANANALYSIS_INTERNAL_STATISTICS_H
