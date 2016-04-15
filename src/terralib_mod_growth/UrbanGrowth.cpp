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

#include <terralib/geometry/Coord2D.h>
#include <terralib/geometry/Point.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/Utils.h>

te::rst::Raster* te::urban::openRaster(const std::string& fileName)
{
  std::map<std::string, std::string> rasterInfo;
  rasterInfo["URI"] = fileName;

  te::rst::Raster* raster = te::rst::RasterFactory::make(rasterInfo);
  return raster;
}

te::rst::Raster* te::urban::createRaster(const std::string& fileName, te::rst::Raster* raster)
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

boost::numeric::ublas::matrix<double> te::urban::getMatrix(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn, double radius)
{
  double resX = raster->getResolutionX();
  int  maskSizeInPixels = te::rst::Round(radius / resX);

  int rasterRow = (int)(referenceRow - maskSizeInPixels);
  int rasterColumn = (int)(referenceColumn - maskSizeInPixels);

  te::gm::Coord2D referenceCoord = raster->getGrid()->gridToGeo((double)referenceColumn, (double)referenceRow);
  te::gm::Point referencePoint(referenceCoord.getX(), referenceCoord.getY());

  boost::numeric::ublas::matrix<double> matrixMask(maskSizeInPixels, maskSizeInPixels);

  for (size_t matrixRow = 0; matrixRow < maskSizeInPixels; ++matrixRow, ++rasterRow)
  {
    for (size_t matrixColumn = 0; matrixColumn < maskSizeInPixels; ++matrixColumn, ++rasterColumn)
    {
      te::gm::Coord2D currentCoord = raster->getGrid()->gridToGeo(rasterColumn, rasterRow);
      te::gm::Point currentPoint(currentCoord.getX(), currentCoord.getY());

      double value = 0;
      if (referencePoint.distance(&currentPoint) <= radius)
      {
        raster->getValue(rasterColumn, rasterRow, value);
      }
      else
      {
        value = -1;
      }

      matrixMask(matrixRow, matrixColumn) = value;
    }
  }

  return matrixMask;
}

double te::urban::calculateValue(double centerPixel, const boost::numeric::ublas::matrix<double>& matrixMask, double& permUrb)
{
  //INPUT CLASSES
  //NO_DATA = 0
  //OTHER = 1
  //WATER = 2
  //URBAN = 3

  //OUTPUTCLASSES
  //(1) URBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness > 50%
  //(2) SUBURBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness < 50% and > 10%
  //(3) RURAL ZONE BUILT-UP AREA: built-up pixels with imperviousness < 10%

  //NO DATA
  if (centerPixel <= 0 || centerPixel >= 4)
  {
    return OUTPUT_NO_DATA;
  }
  //WATER
  if (centerPixel == INPUT_WATER)
  {
    return OUTPUT_WATER;
  } 

  size_t numRows = matrixMask.size1();
  size_t numCols = matrixMask.size2();

  size_t urbanPixelsCount = 0;
  size_t allPixelsCount = 0;

  for (size_t r = 0; r < numRows; ++r)
  {
    for (size_t c = 0; c < numCols; ++c)
    {
      double currentValue = matrixMask(r, c);
      if (currentValue < 1 && currentValue > 3)
      {
        continue;
      }

      ++allPixelsCount;

      //check if the pixel is urban
      if (currentValue == INPUT_URBAN)
      {
        ++urbanPixelsCount;
      }
    }
  }

  double urbanPercentage = (double)urbanPixelsCount / (double)allPixelsCount;

  permUrb = urbanPercentage;

  if (centerPixel == INPUT_URBAN)
  {
    if (urbanPercentage > 0.5)
    {
      return OUTPUT_URBAN;
    }
    else if (urbanPercentage > 0.1 && urbanPercentage <= 0.5)
    {
      return OUTPUT_SUB_URBAN;
    }
    else
    {
      return OUTPUT_RURAL;
    }
  }
  else if (centerPixel == 1)
  {
    if (urbanPercentage > 0.5)
    { 
      return OUTPUT_URBANIZED_OS;
    }
    else
    {
      return OUTPUT_RURAL_OS;
    }
  }

  return OUTPUT_NO_DATA;
}

bool te::urban::calculateEdge(te::rst::Raster* raster, size_t column, size_t line)
{
  double value = 0;
  raster->getValue((unsigned int)column, (unsigned int)(line - 1), value, 0);
  if (value != INPUT_OTHER)
  {
    return true;
  }

  raster->getValue((unsigned int)(column - 1), (unsigned int)line, value, 0);
  if (value != INPUT_OTHER)
  {
    return true;
  }

  raster->getValue((unsigned int)(column + 1), (unsigned int)line, value, 0);
  if (value != INPUT_OTHER)
  {
    return true;
  }

  raster->getValue((unsigned int)column, (unsigned int)(line + 1), value, 0);
  if (value != INPUT_OTHER)
  {
    return true;
  }

  return false;
}

te::rst::Raster* te::urban::classifyUrbanDensity(const std::string& inputFileName, double radius, const std::string& outputFileName, std::map<std::string, double>& mapIndexes)
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

  size_t initRow = maskSizeInPixels;
  size_t initCol = maskSizeInPixels;
  size_t finalRow = numRows - maskSizeInPixels;
  size_t finalCol = numColumns - maskSizeInPixels;

  int numPix = 0;
  int edgeCount = 0; //edge index
  double sumPerUrb = 0;
  for (size_t currentRow = initRow; currentRow < finalRow; ++currentRow)
  {
    for (size_t currentColumn = initCol; currentColumn < finalCol; ++currentColumn)
    {
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);
      boost::numeric::ublas::matrix<double> maskMatrix = getMatrix(inputRaster, currentRow, currentColumn, radius);

      double permUrb = 0.;
      double value = calculateValue(centerPixel, maskMatrix, permUrb);
      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);

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

  return outputRaster;
}
