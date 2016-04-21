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

#include "Utils.h"

#include <terralib/common.h>
#include <terralib/common/TerraLib.h>
#include <terralib/geometry/Coord2D.h>
#include <terralib/geometry/Point.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/plugin.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/Utils.h>

void te::urban::init()
{
  
  TerraLib::getInstance().initialize();

  te::plugin::PluginInfo* info;

  std::string plugins_path = te::common::FindInTerraLibPath("share/terralib/plugins");

#ifdef TERRALIB_MOD_OGR_ENABLED
  info = te::plugin::GetInstalledPlugin(plugins_path + "/te.da.ogr.teplg");
  te::plugin::PluginManager::getInstance().add(info);
#endif

#ifdef TERRALIB_MOD_GDAL_ENABLED
  info = te::plugin::GetInstalledPlugin(plugins_path + "/te.da.gdal.teplg");
  te::plugin::PluginManager::getInstance().add(info);
#endif

#ifdef TERRALIB_MOD_POSTGIS_ENABLED
  info = te::plugin::GetInstalledPlugin(plugins_path + "/te.da.pgis.teplg");
  te::plugin::PluginManager::getInstance().add(info);
#endif  

  te::plugin::PluginManager::getInstance().loadAll();
  
}

void te::urban::finalize()
{
  
  te::plugin::PluginManager::getInstance().unloadAll();

  TerraLib::getInstance().finalize();
  
}

te::rst::Raster* te::urban::openRaster(const std::string& fileName)
{
  std::map<std::string, std::string> rasterInfo;
  rasterInfo["URI"] = fileName;

  te::rst::Raster* raster = te::rst::RasterFactory::open(rasterInfo);
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

std::vector<short> te::urban::getPixelsWithinRadious(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn, double radius)
{
  std::size_t numRows = raster->getNumberOfRows();
  std::size_t numColumns = raster->getNumberOfColumns();

  double resX = raster->getResolutionX();
  int  maskSizeInPixels = te::rst::Round(radius / resX);

  int rasterRow = (int)(referenceRow - maskSizeInPixels);
  int rasterColumn = (int)(referenceColumn - maskSizeInPixels);

  te::gm::Coord2D referenceCoord = raster->getGrid()->gridToGeo((double)referenceColumn, (double)referenceRow);
  te::gm::Point referencePoint(referenceCoord.getX(), referenceCoord.getY());

  std::vector<short> vecPixels;
  vecPixels.reserve(maskSizeInPixels * maskSizeInPixels);

  for (size_t localRow = 0; localRow <= maskSizeInPixels; ++localRow, ++rasterRow)
  {
    for (size_t localColumn = 0; localColumn <= maskSizeInPixels; ++localColumn, ++rasterColumn)
    {
      if (localRow == referenceRow && localColumn == referenceColumn)
      {
        continue;
      }
      if (rasterRow < 0 || rasterColumn < 0)
      {
        continue;
      }
      if (rasterRow >= numRows || rasterColumn >= numColumns)
      {
        continue;
      }

      te::gm::Coord2D currentCoord = raster->getGrid()->gridToGeo(rasterColumn, rasterRow);
      te::gm::Point currentPoint(currentCoord.getX(), currentCoord.getY());

      double value = 0;
      if (referencePoint.distance(&currentPoint) > radius)
      {
        continue;
        
      }
      raster->getValue(rasterColumn, rasterRow, value);

      vecPixels.push_back((short)value);
    }
  }

  return vecPixels;
}

double te::urban::calculateUrbanClass(short centerPixelValue, const std::vector<short>& vecPixels, double& permUrb)
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
  if (centerPixelValue <= 0 || centerPixelValue >= 4)
  {
    return OUTPUT_NO_DATA;
  }
  //WATER
  if (centerPixelValue == INPUT_WATER)
  {
    return OUTPUT_WATER;
  } 

  std::size_t size = vecPixels.size();

  std::size_t urbanPixelsCount = 0;
  std::size_t allPixelsCount = 0;

  for (std::size_t i = 0; i < size; ++i)
  {
    double currentValue = vecPixels[i];
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

  double urbanPercentage = (double)urbanPixelsCount / (double)allPixelsCount;

  permUrb = urbanPercentage;

  if (centerPixelValue == INPUT_URBAN)
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
  else if (centerPixelValue == 1)
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

std::vector<te::gm::Geometry*> te::urban::getGaps(const std::vector<te::gm::Geometry*>& vecInput, double area)
{
  std::vector<te::gm::Geometry*> vecOutput;

  //for all the geometries inside the vector
  for (std::size_t i = 0; i < vecInput.size(); ++i)
  {
    te::gm::Geometry* geometry = vecInput[i];
    if (geometry->getTypeCode() != te::gm::PolygonType)
    {
      continue;
    }

    te::gm::Polygon* polygon = dynamic_cast<te::gm::Polygon*>(geometry);
    if (polygon == 0)
    {
      continue;
    }

    //now we analyse all the holes of the polygon. If a hole has area smaller the the given area, we convert it to a polygon add it to the output list
    for (std::size_t j = 1; j < polygon->getNumRings(); ++j)
    {

    }
  }

  return vecOutput;
}
