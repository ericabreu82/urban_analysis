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

#include "Utils.h"

te::rst::Raster* te::urban::classifyUrbanAreas(const std::string& inputFileName, double radius, const std::string& outputFileName)
{
  te::rst::Raster* inputRaster = openRaster(inputFileName);

  assert(inputRaster);

  te::rst::Raster* outputRaster = createRaster(outputFileName, inputRaster);

  assert(outputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  int  maskSizeInPixels = te::rst::Round(radius / resX);

  std::size_t initRow = maskSizeInPixels;
  std::size_t initCol = maskSizeInPixels;
  std::size_t finalRow = numRows - maskSizeInPixels;
  std::size_t finalCol = numColumns - maskSizeInPixels;

  for (std::size_t currentRow = initRow; currentRow < finalRow; ++currentRow)
  {
    for (std::size_t currentColumn = initCol; currentColumn < finalCol; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      //gets the pixels surrounding pixels that intersects the given radious
      std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster, currentRow, currentColumn, radius);

      double permUrb = 0.;
      double value = calculateUrbanClass((short)centerPixel, vecPixels, permUrb);
      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);
    }
  }

  return outputRaster;
}

te::rst::Raster* te::urban::filterUrbanPixels(const std::string& inputFileName, const std::string& outputFileName)
{
  te::rst::Raster* inputRaster = openRaster(inputFileName);

  assert(inputRaster);

  te::rst::Raster* outputRaster = createRaster(outputFileName, inputRaster);

  assert(outputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  std::size_t initRow = 0;
  std::size_t initCol = 0;
  std::size_t finalRow = numRows;
  std::size_t finalCol = numColumns;

  double noDataValue = 0.;

  for (std::size_t currentRow = initRow; currentRow < finalRow; ++currentRow)
  {
    for (std::size_t currentColumn = initCol; currentColumn < finalCol; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      double value = noDataValue;
      if (centerPixel == 1 || centerPixel == 2 || centerPixel == 4)
      {
        value = 1;
      }

      //gets the pixels surrounding pixels that intersects the given radiouss
      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);
    }
  }

  return outputRaster;
}

te::rst::Raster* te::urban::classifyIsolatedOpenPatches(const std::string& inputFileName, const std::string& outputFileName)
{
  //we first need to create a binary image containing only the urban pixels
  te::rst::Raster* binaryUrbanRaster = filterUrbanPixels(inputFileName, outputFileName);

  //then we vectorize the result
  std::vector<te::gm::Geometry*> vecGeometries;
  binaryUrbanRaster->vectorize(vecGeometries, 0);

  return binaryUrbanRaster;

}

void te::urban::calculateUrbanIndexes(const std::string& inputFileName, double radius, std::map<std::string, double>& mapIndexes)
{
  te::rst::Raster* inputRaster = openRaster(inputFileName);

  assert(inputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  size_t initRow = 0;
  size_t initCol = 0;
  size_t finalRow = numRows;
  size_t finalCol = numColumns;

  int numPix = 0;
  int edgeCount = 0; //edge index
  double sumPerUrb = 0;
  for (size_t currentRow = initRow; currentRow < finalRow; ++currentRow)
  {
    for (size_t currentColumn = initCol; currentColumn < finalCol; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      //gets the pixels surrounding pixels that intersects the given radious
      std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster, currentRow, currentColumn, radius);

      double permUrb = 0.;
      double value = calculateUrbanClass((short)centerPixel, vecPixels, permUrb);

      if (centerPixel != INPUT_URBAN && centerPixel != INPUT_OTHER)
      {
        continue;
      }

      //sum the perviousness
      if (centerPixel == INPUT_OTHER)
      {
        sumPerUrb += permUrb;
        ++numPix;
      }

      bool hasEdge = calculateEdge(inputRaster, currentColumn, currentRow);
      if (hasEdge == true)
      {
        ++edgeCount;
      }
    }
  }

  double openness = 1 - (sumPerUrb / numPix);
  double edgeIndex = double(edgeCount) / numPix;

  mapIndexes["openness"] = openness;
  mapIndexes["edgeIndex"] = edgeIndex;
}
