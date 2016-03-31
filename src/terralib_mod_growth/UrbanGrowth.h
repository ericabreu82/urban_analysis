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

#include <boost/numeric/ublas/matrix.hpp>

namespace te
{
  namespace rst
  {
    class Raster;
  }

  namespace urban
  {
    TEGROWTHEXPORT te::rst::Raster* createRaster(te::rst::Raster* raster, const std::string& fileName);

    TEGROWTHEXPORT boost::numeric::ublas::matrix<double> getMatrix(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn, double radius);

    TEGROWTHEXPORT double calculateValue(const boost::numeric::ublas::matrix<double>& matrixMask);

    TEGROWTHEXPORT te::rst::Raster* classifyUrbanDensity(te::rst::Raster* inputRaster, double radius);
  }
}

#endif //__URBANANALYSIS_INTERNAL_GROWTH_URBANGROWTH_H
