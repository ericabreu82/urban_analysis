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
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/Utils.h>

te::rst::Raster* te::urban::createRaster(te::rst::Raster* raster, const std::string& fileName)
{
  std::map<std::string, std::string> rasterInfo;
  rasterInfo["URI"] = fileName;

  std::vector<te::rst::BandProperty*> bandsProperties;
  for (size_t bandIndex = 0; bandIndex < raster->getNumberOfBands(); ++bandIndex)
  {
    bandsProperties.push_back(new te::rst::BandProperty(*(raster->getBand(bandIndex)->getProperty())));
  }

  te::rst::Raster* createdRaster = te::rst::RasterFactory::make("GDAL", new te::rst::Grid(*(raster->getGrid())), bandsProperties, rasterInfo, 0, 0);
  return createdRaster;
}

boost::numeric::ublas::matrix<double> te::urban::getMatrix(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn, size_t maskSizeInPixels)
{
  int rasterRow = (int)(referenceRow - maskSizeInPixels);
  int rasterColumn = (int)(referenceColumn - maskSizeInPixels);

  boost::numeric::ublas::matrix<double> matrixMask(maskSizeInPixels, maskSizeInPixels);

  for (size_t matrixRow = 0; matrixRow < maskSizeInPixels; ++matrixRow, ++rasterRow)
  {
    for (size_t matrixColumn = 0; matrixColumn < maskSizeInPixels; ++matrixColumn, ++rasterColumn)
    {
      double value = 0;
      raster->getValue(rasterColumn, rasterRow, value);

      matrixMask(matrixRow, matrixColumn) = value;
    }
  }

  return matrixMask;
}

double te::urban::calculateValue(const boost::numeric::ublas::matrix<double>& matrixMask)
{
  double value = 0;

  return value;
}

te::rst::Raster* te::urban::classifyUrbanDensity(te::rst::Raster* inputRaster, double radius)
{
  assert(inputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  int  maskSizeInPixels = te::rst::Round(radius / resX);

  size_t initRow = maskSizeInPixels;
  size_t initCol = maskSizeInPixels;
  size_t finalRow = numRows - maskSizeInPixels;
  size_t finalCol = numColumns - maskSizeInPixels;

  for (size_t currentRow = initRow; currentRow < finalRow; ++currentRow)
  {
    for (size_t currentColumn = initCol; currentColumn < finalCol; ++currentColumn)
    {
      boost::numeric::ublas::matrix<double> maskMatrix = getMatrix(inputRaster, currentRow, currentColumn, maskSizeInPixels);
      double value = calculateValue(maskMatrix);
    }
  }

  return 0;
}
