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

#include "UrbanGrowth.h"

#include <terralib/raster/Raster.h>
#include <terralib/raster/Utils.h>

te::rst::Raster* te::urban::classifyUrbanDensity(te::rst::Raster* inputRaster, double radius)
{
  assert(inputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  int  maskSizeInPixelsX = te::rst::Round(radius / resX);
  int  maskSizeInPixelsY = te::rst::Round(radius / resY);

  size_t initRow = maskSizeInPixelsX;
  size_t initCol = maskSizeInPixelsY;
  size_t finalRow = numRows - maskSizeInPixelsX;
  size_t finalCol = numColumns - maskSizeInPixelsY;

  for (size_t currentRow = initRow; currentRow < finalRow; ++currentRow)
  {
    for (size_t currentColumn = initCol; currentColumn < finalCol; ++currentColumn)
    {
      double value = executeFunction(inputRaster, currentRow, currentColumn);
    }
  }

  return 0;
}
