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

#include <terralib/raster/Raster.h>
#include <terralib/dataaccess/datasource/DataSource.h>


#include <memory>
#include <set>
#include <string>
#include <vector>

namespace te
{
  namespace mem
  {
    class DataSet;
  }

  namespace urban
  {
    //calculates the statistics for a result image from urban growth method (infill, leapfrog, extension)
    TEGROWTHEXPORT void CalculateStatistics(te::rst::Raster* raster, te::da::DataSource* ds, const std::string& dataSetName, 
                                            const bool& calculateArea, const bool& calculateCount, const std::string& outPath, const std::string& outDataSetName);

    TEGROWTHEXPORT std::auto_ptr<te::da::DataSetType> createStatisticsDataSetType(std::string dataSetName, te::da::DataSetType* inputDsType, 
                                                                                  const bool& calculateArea, const bool& calculateCount);

    /*! Function used to create the output data */
    TEGROWTHEXPORT std::auto_ptr<te::mem::DataSet> createStatisticsDataSet(te::rst::Raster* raster, te::da::DataSetType* dsType, te::da::DataSet* inputDs,
                                                                           const bool& calculateArea, const bool& calculateCount);

    /*! Function used to save the output dataset */
    TEGROWTHEXPORT void saveStatisticsDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType, te::da::DataSource* ds, std::string dataSetName);

    /*! Function used to calculate statistics */
    TEGROWTHEXPORT void calculateStatisticsDataSet(te::rst::Raster* raster, te::gm::Geometry* geom,
                                                   int& if_count, int& lf_count, int& ext_count,
                                                   double& if_area, double& lf_area, double& ext_area,
                                                   const bool& calculateArea, const bool& calculateCount);

  }
}

#endif //__URBANANALYSIS_INTERNAL_STATISTICS_H
