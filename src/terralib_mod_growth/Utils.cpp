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
#include <terralib/core/utils/Platform.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/geometry/Coord2D.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/Point.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/memory/CachedRaster.h>
#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/plugin.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/RasterSummary.h>
#include <terralib/raster/RasterSummaryManager.h>
#include <terralib/raster/Utils.h>

#include <cstdlib>

void te::urban::init()
{
  
  TerraLib::getInstance().initialize();

  te::plugin::PluginInfo* info;

  std::string plugins_path = te::core::FindInTerraLibPath("share/terralib/plugins");

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

std::auto_ptr<te::rst::Raster> te::urban::cloneRasterIntoMem(te::rst::Raster* raster, bool copyData)
{
  //load to mem
  std::vector<te::rst::BandProperty*> bprops;

  for (size_t t = 0; t < raster->getNumberOfBands(); ++t)
  {
    te::rst::Band* band = raster->getBand(t);

    te::rst::BandProperty* bp = new te::rst::BandProperty(*band->getProperty());
    if (bp->m_noDataValue == std::numeric_limits<double>::max())
    {
      bp->m_noDataValue = 0;
    }
    
    bprops.push_back(bp);
  }

  std::map< std::string, std::string > dummyRInfo;

  te::rst::Raster* rasterMem = te::rst::RasterFactory::make("MEM", new te::rst::Grid(*(raster->getGrid())), bprops, dummyRInfo, 0, 0);

  if (copyData == true)
  {
    te::rst::Copy(*raster, *rasterMem);
  }
  else
  {
    te::rst::FillRaster(rasterMem, 0.);
  }

  //create output auto_ptr
  std::auto_ptr<te::rst::Raster> rOut(rasterMem);
  return rOut;
}

std::auto_ptr<te::rst::Raster> te::urban::openRaster(const std::string& fileName)
{
  std::map<std::string, std::string> rasterInfo;
  rasterInfo["URI"] = fileName;

  std::auto_ptr<te::rst::Raster> rasterPointer(te::rst::RasterFactory::open(rasterInfo));

  std::auto_ptr<te::rst::Raster> memRaster = cloneRasterIntoMem(rasterPointer.get(), true);
  return memRaster;
}

std::auto_ptr<te::rst::Raster> te::urban::createRaster(const std::string& fileName, te::rst::Raster* raster)
{
  std::map<std::string, std::string> rasterInfo;
  rasterInfo["URI"] = fileName;

  std::vector<te::rst::BandProperty*> bandsProperties;
  for (size_t bandIndex = 0; bandIndex < raster->getNumberOfBands(); ++bandIndex)
  {
    te::rst::BandProperty* bp = new te::rst::BandProperty(*(raster->getBand(bandIndex)->getProperty()));

    bp->m_noDataValue = 0.;

    bandsProperties.push_back(bp);
  }
  
  te::rst::Raster* createdRaster = te::rst::RasterFactory::make("GDAL", new te::rst::Grid(*(raster->getGrid())), bandsProperties, rasterInfo, 0, 0);

  te::rst::FillRaster(createdRaster, 0.);

  std::auto_ptr<te::rst::Raster> createdRasterPtr(createdRaster);
  return createdRasterPtr;
}

std::auto_ptr<te::da::DataSource> te::urban::createDataSourceOGR(const std::string& fileName)
{
  std::map<std::string, std::string> connInfo;
  connInfo["URI"] = fileName;

  std::auto_ptr<te::da::DataSource> dsOGR = te::da::DataSourceFactory::make("OGR");
  dsOGR->setConnectionInfo(connInfo);
  dsOGR->open();

  return dsOGR;
}

void te::urban::saveRaster(const std::string& fileName, te::rst::Raster* raster)
{
  std::auto_ptr<te::rst::Raster> outputRaster = createRaster(fileName, raster);
  te::rst::Copy(*raster, *outputRaster);
}

void te::urban::saveVector(const std::string& fileName, const std::string& filePath, const std::vector<te::gm::Geometry*>& vecGeometries, const int& srid)
{
  //create datasource
  std::auto_ptr<te::da::DataSource> ds = createDataSourceOGR(filePath);

  //create dataSetType
  std::auto_ptr<te::da::DataSetType> dsType = createDataSetType(fileName, srid);

  //create dataSet
  std::auto_ptr<te::mem::DataSet> dsMem = createDataSet(dsType.get(), vecGeometries);

  //save dataSet
  saveDataSet(dsMem.get(), dsType.get(), ds.get(), fileName);
}

boost::numeric::ublas::matrix<bool> te::urban::createRadiusMask(double resolution, double radius)
{
  int  radiusInPixels = te::rst::Round(radius / resolution);
  int range = (radiusInPixels * 2) + 1;

  //te::gm::Coord2D referenceCoord((radiusInPixels + 1) * resolution, (radiusInPixels + 1) * resolution);
  te::gm::Coord2D referenceCoord(radiusInPixels * resolution, radiusInPixels * resolution);

  boost::numeric::ublas::matrix<bool> mask(range, range);

  for (size_t localRow = 0; localRow < range; ++localRow)
  {
    for (size_t localColumn = 0; localColumn < range; ++localColumn)
    {
      bool maskOn = false;

      te::gm::Coord2D currentCoord(localColumn * resolution, localRow * resolution);

      double currentDistance = TeDistance(referenceCoord, currentCoord);
      if (currentDistance <= radius)
      {
        maskOn = true;
      }

      mask(localRow, localColumn) = maskOn;
    }
  }
  return mask;
}

std::vector<short> te::urban::getPixelsWithinRadious(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn, double radius, const boost::numeric::ublas::matrix<bool>& mask)
{
  std::size_t numRows = raster->getNumberOfRows();
  std::size_t numColumns = raster->getNumberOfColumns();

  double resX = raster->getResolutionX();
  int  maskSizeInPixels = te::rst::Round(radius / resX);

  int range = (maskSizeInPixels * 2) + 1;

  std::size_t localNumRows = mask.size1();
  std::size_t localNumColumns = mask.size2();

  std::vector<short> vecPixels;
  vecPixels.reserve(localNumRows * localNumColumns);

  int rasterRow = ((int)referenceRow - maskSizeInPixels);
  for (size_t localRow = 0; localRow < range; ++localRow, ++rasterRow)
  {
    int rasterColumn = ((int)referenceColumn - maskSizeInPixels);
    for (size_t localColumn = 0; localColumn < range; ++localColumn, ++rasterColumn)
    {
      if (mask(localRow, localColumn) == false)
      {
        continue;
      }

      if (rasterRow == referenceRow && rasterColumn == referenceColumn)
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

      double value = 0;
      raster->getValue(rasterColumn, rasterRow, value);

      vecPixels.push_back((short)value);
    }
  }

  return vecPixels;
}

std::vector<short> te::urban::getAdjacentPixels(te::rst::Raster* raster, size_t referenceRow, size_t referenceColumn)
{
  std::size_t numRows = raster->getNumberOfRows();
  std::size_t numColumns = raster->getNumberOfColumns();

  int  maskSizeInPixels = 1;

  int rasterRow = ((int)referenceRow - maskSizeInPixels);
  int rasterColumn = ((int)referenceColumn - maskSizeInPixels);

  int range = (maskSizeInPixels * 2) + 1;

  te::gm::Coord2D referenceCoord = raster->getGrid()->gridToGeo((double)referenceColumn, (double)referenceRow);
  te::gm::Point referencePoint(referenceCoord.getX(), referenceCoord.getY());

  std::vector<short> vecPixels;
  vecPixels.reserve(maskSizeInPixels * maskSizeInPixels);

  for (size_t localRow = 0; localRow < range; ++localRow, ++rasterRow)
  {
    for (size_t localColumn = 0; localColumn < range; ++localColumn, ++rasterColumn)
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

      double value = 0;
      raster->getValue(rasterColumn, rasterRow, value);

      vecPixels.push_back((short)value);
    }
  }

  return vecPixels;
}

double te::urban::calculateUrbanizedArea(short centerPixelValue, const InputClassesMap& inputClassesMap, const std::vector<short>& vecPixels, double& permUrb)
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

  const short InputWater = inputClassesMap.find(INPUT_WATER)->second;
  const short InputUrban = inputClassesMap.find(INPUT_URBAN)->second;
  const short InputOther = inputClassesMap.find(INPUT_OTHER)->second;

  std::size_t size = vecPixels.size();

  std::size_t urbanPixelsCount = 0;
  std::size_t allPixelsCount = 0;

  for (std::size_t i = 0; i < size; ++i)
  {
    double currentValue = vecPixels[i];
    if (currentValue != InputWater && currentValue != InputUrban && currentValue != InputOther)
    {
      continue;
    }

    ++allPixelsCount;

    //check if the pixel is urban
    if (currentValue == InputUrban)
    {
      ++urbanPixelsCount;
    }
  }

  //all adjacent pixels are NOT_DATA
  if (allPixelsCount == 0)
  {
    return OUTPUT_NO_DATA;
  }

  double urbanPercentage = (double)urbanPixelsCount / (double)allPixelsCount;

  permUrb = urbanPercentage;

  if (centerPixelValue == InputUrban)
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
  else if (centerPixelValue == InputOther)
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

double te::urban::calculateUrbanFootprint(short centerPixelValue, const InputClassesMap& inputClassesMap, const std::vector<short>& vecPixels, double& permUrb)
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

  const short InputWater = inputClassesMap.find(INPUT_WATER)->second;
  const short InputUrban = inputClassesMap.find(INPUT_URBAN)->second;
  const short InputOther = inputClassesMap.find(INPUT_OTHER)->second;

  std::size_t size = vecPixels.size();

  std::size_t urbanPixelsCount = 0;
  std::size_t allPixelsCount = 0;

  for (std::size_t i = 0; i < size; ++i)
  {
    double currentValue = vecPixels[i];
    if (currentValue != InputWater && currentValue != InputUrban && currentValue != InputOther)
    {
      continue;
    }

    ++allPixelsCount;

    //check if the pixel is urban
    if (currentValue == InputUrban)
    {
      ++urbanPixelsCount;
    }
  }
  //all adjacent pixels are NOT_DATA
  if (allPixelsCount == 0)
  {
    return OUTPUT_NO_DATA;
  }

  double urbanPercentage = (double)urbanPixelsCount / (double)allPixelsCount;

  permUrb = urbanPercentage;

  if (centerPixelValue == InputUrban)
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

  return OUTPUT_NO_DATA;
}

double te::urban::calculateUrbanOpenArea(short centerPixelValue, const std::vector<short>& vecPixels)
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

  double newPixel = OUTPUT_RURAL_OS; // default to rural OS

  std::size_t size = vecPixels.size();
  for (std::size_t i = 0; i < size; ++i)
  {
    double currentValue = vecPixels[i];

    //check if the pixel is urban
    if (currentValue == OUTPUT_URBAN || currentValue == OUTPUT_SUB_URBAN)
    {
      newPixel = OUTPUT_URBANIZED_OS;
      break;
    }
  }

  return newPixel;
}

bool te::urban::calculateEdge(te::rst::Raster* raster, const InputClassesMap& inputClassesMap, size_t column, size_t line)
{
  //the script seems to be wrong. double check
  const short InputUrban = inputClassesMap.find(INPUT_URBAN)->second;

  double value = 0;
  raster->getValue((unsigned int)column, (unsigned int)(line - 1), value, 0);
  if (value != InputUrban)
  {
    return true;
  }

  raster->getValue((unsigned int)(column - 1), (unsigned int)line, value, 0);
  if (value != InputUrban)
  {
    return true;
  }

  raster->getValue((unsigned int)(column + 1), (unsigned int)line, value, 0);
  if (value != InputUrban)
  {
    return true;
  }

  raster->getValue((unsigned int)column, (unsigned int)(line + 1), value, 0);
  if (value != InputUrban)
  {
    return true;
  }

  return false;
}

std::auto_ptr<te::rst::Raster> te::urban::filterUrbanPixels(te::rst::Raster* raster)
{
  te::rst::Raster* inputRaster = raster;

  assert(inputRaster);

  std::auto_ptr<te::rst::Raster> outputRaster = cloneRasterIntoMem(inputRaster, false);

  assert(outputRaster.get());

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  double noDataValue = 0.;

  for (std::size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (std::size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      double value = noDataValue;
      if (centerPixel == 1 || centerPixel == 2 || centerPixel == 4)
      {
        value = 1.;
      }

      //gets the pixels surrounding pixels that intersects the given radiouss
      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);
    }
  }

  return outputRaster;
}

std::vector<te::gm::Geometry*> te::urban::getGaps(const std::vector<te::gm::Geometry*>& vecInput, double area)
{
  std::vector<te::gm::Geometry*> vecOutput;

  //for all the geometries inside the vector
  for (std::size_t i = 0; i < vecInput.size(); ++i)
  {
    te::gm::Geometry* geometry = vecInput[i];
    if (geometry->getGeomTypeId() != te::gm::PolygonType)
    {
      continue;
    }

    te::gm::Polygon* polygon = dynamic_cast<te::gm::Polygon*>(geometry);
    if (polygon == 0)
    {
      continue;
    }

    //now we analyse all the holes of the polygon. If a hole has area smaller the the given area, we convert it to a polygon add it to the output list
    for (std::size_t j = 0; j < polygon->getNumInteriorRings(); ++j)
    {
      te::gm::Curve* curve = polygon->getInteriorRingN(j);

      te::gm::Curve* newCurve = static_cast<te::gm::Curve*>(curve->clone());

      te::gm::Polygon* newPolygon = new te::gm::Polygon(1, te::gm::PolygonType, polygon->getSRID());
      newPolygon->setRingN(0, newCurve);

      double newPolArea = newPolygon->getArea() / 10000.; //convert to hectare

      if (newPolArea < area)
      {
        vecOutput.push_back(newPolygon);
      }
      else
      {
        delete newPolygon;
      }
    }
  }

  return vecOutput;
}

std::auto_ptr<te::rst::Raster> te::urban::createDistinctGroups(te::rst::Raster* inputRaster)
{
  assert(inputRaster);

  std::auto_ptr<te::rst::Raster> outputRaster = cloneRasterIntoMem(inputRaster, true);

  //we first vectorize the raster
  std::vector<te::gm::Geometry*> vecGeometries;
  outputRaster->vectorize(vecGeometries, 0);

  //then we define one class for each group
  std::vector<double> vecClasses;
  vecClasses.reserve(vecGeometries.size());
  for (std::size_t i = 0; i < vecGeometries.size(); ++i)
  {
    vecClasses[i] = (double)i;
  }

  //finally we rasterize the geometries
  outputRaster->rasterize(vecGeometries, vecClasses);

  te::common::FreeContents(vecGeometries);

  return outputRaster;
}

std::set<double> te::urban::detectEdgeOpenAreaGroups(te::rst::Raster* otherNewDevRaster, te::rst::Raster* otherNewDevGroupedRaster, te::rst::Raster* footprintRaster)
{
  assert(otherNewDevRaster);
  assert(otherNewDevGroupedRaster);
  assert(footprintRaster);

  std::set<double> setGroupsWithEdges;

  unsigned int numRows = otherNewDevRaster->getNumberOfRows();
  unsigned int numColumns = otherNewDevRaster->getNumberOfColumns();

  for (unsigned int row = 0; row < numRows; ++row)
  {
    for (unsigned int column = 0; column < numColumns; ++column)
    {
      //we first read the pixel from newDev raster. If its value is not 1, we continue
      double newDevValue = 0.;
      otherNewDevRaster->getValue(column, row, newDevValue);
      if (newDevValue != 1)
      {
        continue;
      }

      double otherDevGroupedValue = 0.;
      otherNewDevGroupedRaster->getValue(column, row, otherDevGroupedValue);

      //then we check the value of the footprint image. If 4 or 5, we register the current group in the SET
      double footprintValue = 0.;
      footprintRaster->getValue(column, row, footprintValue);

      if (footprintValue == 4 || footprintValue == 5)
      {
        setGroupsWithEdges.insert(otherDevGroupedValue);
        continue;
      }

      //if we got here, we must analyse the adjacent pixels in the footprint raster looking for any urban pixel (1, 2 and 3)
      std::vector<short> vecPixels = getAdjacentPixels(footprintRaster, row, column);
      for (std::size_t i = 0; i < vecPixels.size(); ++i)
      {
        if (vecPixels[i] >= 1 && vecPixels[i] <= 3)
        {
          setGroupsWithEdges.insert(otherDevGroupedValue);
          break;
        }
      }
    }
  }

  return setGroupsWithEdges;
}

void te::urban::generateInfillOtherDevRasters(te::rst::Raster* rasterT1, te::rst::Raster* rasterT2, const std::string& infillRasterFileName, const std::string& otherDevRasterFileName)
{
  assert(rasterT1);
  assert(rasterT2);

  std::auto_ptr<te::rst::Raster> infillRaster = cloneRasterIntoMem(rasterT1, false);
  std::auto_ptr<te::rst::Raster> otherDevRaster = cloneRasterIntoMem(rasterT1, false);

  assert(infillRaster.get());
  assert(otherDevRaster.get());

  std::size_t numRows = rasterT1->getNumberOfRows();
  std::size_t numColumns = rasterT1->getNumberOfColumns();

  if (numRows != rasterT2->getNumberOfRows())
  {
    return;
  }
  if (numColumns != rasterT2->getNumberOfColumns())
  {
    return;
  }

  for (std::size_t row = 0; row < numRows; ++row)
  {
    for (std::size_t column = 0; column < numColumns; ++column)
    {
      double valueT1 = 0.;
      rasterT1->getValue((unsigned int)column, (unsigned int)row, valueT1);

      double valueT2 = 0.;
      rasterT2->getValue((unsigned int)column, (unsigned int)row, valueT2);

      double valueInFill = 0;
      double valueOtherDev = 0;

      //if urban in T2
      if (valueT2 == OUTPUT_URBAN || valueT2 == OUTPUT_SUB_URBAN || valueT2 == OUTPUT_RURAL)
      {
        //if urbanized open space in T1
        if (valueT1 == OUTPUT_URBANIZED_OS || valueT1 == OUTPUT_SUBURBAN_ZONE_OPEN_AREA)
        {
          valueInFill = 1;
        }
        else if (valueT1 == OUTPUT_RURAL_OS || valueT1 == OUTPUT_WATER)
        {
          valueInFill = 2;
          valueOtherDev = 1;
        }
      }

      infillRaster->setValue((unsigned int)column, (unsigned int)row, valueInFill);
      otherDevRaster->setValue((unsigned int)column, (unsigned int)row, valueOtherDev);
    }
  }

  saveRaster(infillRasterFileName, infillRaster.get());
  saveRaster(otherDevRasterFileName, otherDevRaster.get());
}

std::auto_ptr<te::rst::Raster> te::urban::classifyNewDevelopment(te::rst::Raster* infillRaster, te::rst::Raster* otherDevGroupedRaster, const std::set<double>& setEdgesOpenAreaGroups)
{
  assert(infillRaster);
  assert(otherDevGroupedRaster);

  unsigned int numRows = infillRaster->getNumberOfRows();
  unsigned int numColumns = infillRaster->getNumberOfColumns();

  if (numRows != otherDevGroupedRaster->getNumberOfRows())
  {
    return std::auto_ptr<te::rst::Raster>();
  }
  if (numColumns != otherDevGroupedRaster->getNumberOfColumns())
  {
    return std::auto_ptr<te::rst::Raster>();
  }

  std::auto_ptr<te::rst::Raster> outputRaster = cloneRasterIntoMem(infillRaster, false);
  for (unsigned int row = 0; row < numRows; ++row)
  {
    for (unsigned int column = 0; column < numColumns; ++column)
    {
      double infillValue = 0.;
      infillRaster->getValue(column, row, infillValue);

      double outputValue = 0.;

      if (infillValue == 1)
      {
        //infill
        outputValue = 1;
      }
      else if (infillValue == 2)
      {
        double otherDevValue = 0.;
        otherDevGroupedRaster->getValue(column, row, otherDevValue);

        std::set<double>::iterator it = setEdgesOpenAreaGroups.find(otherDevValue);
        if (it != setEdgesOpenAreaGroups.end())
        {
          //extension
          outputValue = 2;
        }
        else
        {
          //leapfrog
          outputValue = 3;
        }
      }

      outputRaster->setValue(column, row, outputValue);
    }
  }

  return outputRaster;
}

double te::urban::TeDistance(const te::gm::Coord2D& c1, const te::gm::Coord2D& c2)
{
  return sqrt(((c2.getX() - c1.getX()) * (c2.getX() - c1.getX())) + ((c2.getY() - c1.getY()) * (c2.getY() - c1.getY())));
}

void te::urban::getUrbanCoordinates(te::rst::Raster* raster, std::vector<te::gm::Coord2D>& vecUrbanCoords)
{
  assert(raster);

  unsigned int numRows = raster->getNumberOfRows();
  unsigned int numColumns = raster->getNumberOfColumns();
  double resX = raster->getResolutionX();
  double resY = raster->getResolutionY();

  //empiric value to avoid unnecessary resize (copy) of the vector. it supposes that 1/4 of the pixels are urban
  vecUrbanCoords.reserve(numRows * numColumns / 4);

  for (std::size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (std::size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      raster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      if (centerPixel == OUTPUT_URBAN || centerPixel == OUTPUT_SUB_URBAN || centerPixel == OUTPUT_URBANIZED_OS || centerPixel == OUTPUT_SUBURBAN_ZONE_OPEN_AREA)
      {
        te::gm::Coord2D coord = raster->getGrid()->gridToGeo((double)currentColumn, (double)currentRow);
        vecUrbanCoords.push_back(coord);
      }
    }
  }
}

std::vector<te::gm::Coord2D> te::urban::getRandomCoordSubset(const std::vector<te::gm::Coord2D>& vecUrbanCoords, std::size_t subsetSize)
{
  std::vector<te::gm::Coord2D> vecSubset;

  for (std::size_t i = 0; i < subsetSize; ++i)
  {
    int randValue = rand();
    double factor = (double)randValue / RAND_MAX;

    std::size_t randIndex = (std::size_t)(subsetSize * factor);
    if (randIndex >= vecUrbanCoords.size())
    {
      break;
    }

    vecSubset.push_back(vecUrbanCoords[randIndex]);
  }

  return vecSubset;
}

std::auto_ptr<te::rst::Raster> te::urban::reclassify(te::rst::Raster* inputRaster, const std::map<int, int>& mapValues, int defaultValue)
{
  assert(inputRaster);

  std::auto_ptr<te::rst::Raster> outputRaster = cloneRasterIntoMem(inputRaster, false);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();

  for (std::size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (std::size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      //gets the value of the current center pixel
      double oldValue = 0;
      double newValue = defaultValue;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, oldValue);

      std::map<int, int>::const_iterator it = mapValues.find((int)oldValue);
      if (it != mapValues.end())
      {
        newValue = (double)it->second;
      }

      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, newValue);
    }
  }

  return outputRaster;
}

std::auto_ptr<te::da::DataSetType> te::urban::createDataSetType(std::string dataSetName, int srid)
{
  std::auto_ptr<te::da::DataSetType> dsType(new te::da::DataSetType(dataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
  dsType->add(idProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::PolygonType);
  dsType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_id";
  pkName += "_" + dataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dsType.get());
  pk->add(idProperty);

  return dsType;
}

std::auto_ptr<te::mem::DataSet> te::urban::createDataSet(te::da::DataSetType* dsType, const std::vector<te::gm::Geometry*>& geoms)
{
  std::auto_ptr<te::mem::DataSet> ds(new te::mem::DataSet(dsType));

  for (std::size_t t = 0; t < geoms.size(); ++t)
  {
    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(ds.get());

    //set id
    item->setInt32("id", (int)t);

    //set geometry
    item->setGeometry("geom", (te::gm::Geometry*)geoms[t]->clone());

    ds->add(item);
  }

  return ds;
}

void te::urban::saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType, te::da::DataSource* ds, std::string dataSetName)
{
  //save dataset
  dataSet->moveBeforeFirst();

  std::map<std::string, std::string> options;

  ds->createDataSet(dsType, options);

  ds->add(dataSetName, dataSet, options);
}
