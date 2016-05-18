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
#include "Utils.h"

//Terralib
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/Utils.h>


te::rst::Raster* te::urban::classifyUrbanizedArea(const std::string& inputFileName, double radius, const std::string& outputFileName)
{
  std::auto_ptr<te::rst::Raster> inputRaster = openRaster(inputFileName);

  assert(inputRaster.get());

  te::rst::Raster* outputRaster = createRaster(outputFileName, inputRaster.get());

  assert(outputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  int  maskSizeInPixels = te::rst::Round(radius / resX);

  te::common::TaskProgress task("Classify Urbanized Area");
  task.setTotalSteps((int)(numRows * numColumns));
  task.useTimer(true);

  for (std::size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (std::size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      double value = OUTPUT_NO_DATA;

      //WATER
      if (centerPixel == INPUT_WATER)
      {
        value = OUTPUT_WATER;
      }
      else if (centerPixel > 0 && centerPixel < 4)
      {
        //gets the pixels surrounding pixels that intersects the given radious
        std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster.get(), currentRow, currentColumn, radius);

        double permUrb = 0.;
        value = calculateUrbanizedArea((short)centerPixel, vecPixels, permUrb);
      }

      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);

      task.pulse();
    }
  }

  return outputRaster;
}

te::rst::Raster* te::urban::classifyUrbanFootprint(const std::string& inputFileName, double radius, const std::string& outputFileName)
{
  std::auto_ptr<te::rst::Raster> inputRaster = openRaster(inputFileName);

  assert(inputRaster.get());

  te::rst::Raster* outputRaster = createRaster(outputFileName, inputRaster.get());

  assert(outputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  int  maskSizeInPixels = te::rst::Round(radius / resX);

  te::common::TaskProgress task("Classify Urbanized Footprint");
  task.setTotalSteps((int)(numRows * numColumns));
  task.useTimer(true);

  for (std::size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (std::size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      //NO DATA
      double value = OUTPUT_NO_DATA;

      //WATER
      if (centerPixel == INPUT_WATER)
      {
        value = OUTPUT_WATER;
      }
      else if (centerPixel == INPUT_OTHER)
      {
        value = OUTPUT_URBANIZED_OS;
      }
      else if (centerPixel > 0 || centerPixel < 4)
      {
        //gets the pixels surrounding pixels that intersects the given radious
        std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster.get(), currentRow, currentColumn, radius);

        double permUrb = 0.;
        value = calculateUrbanFootprint((short)centerPixel, vecPixels, permUrb);
      }
      
      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);

      task.pulse();
    }
  }

  return outputRaster;
}

te::rst::Raster* te::urban::classifyUrbanOpenArea(te::rst::Raster* raster, double radius, const std::string& outputFileName)
{
  te::rst::Raster* inputRaster = raster;

  assert(inputRaster);

  te::rst::Raster* outputRaster = createRaster(outputFileName, inputRaster);

  assert(outputRaster);

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  int  maskSizeInPixels = te::rst::Round(radius / resX);

  te::common::TaskProgress task("Classify Urbanized Open Area");
  task.setTotalSteps((int)(numRows * numColumns));
  task.useTimer(true);

  for (std::size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (std::size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      double value = centerPixel;
      if (centerPixel == OUTPUT_URBANIZED_OS)
      {
        //gets the pixels surrounding pixels that intersects the given radious
        std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster, currentRow, currentColumn, radius);

        double permUrb = 0.;
        value = calculateUrbanOpenArea((short)centerPixel, vecPixels);
      }

      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);

      task.pulse();
    }
  }

  return outputRaster;
}

te::rst::Raster* te::urban::identifyIsolatedOpenPatches(te::rst::Raster* raster, const std::string& outputFileName)
{
  //we first need to create a binary image containing only the urban pixels
  te::rst::Raster* binaryUrbanRaster = filterUrbanPixels(raster, outputFileName);

  //then we vectorize the result
  std::vector<te::gm::Geometry*> vecGeometries;
  binaryUrbanRaster->vectorize(vecGeometries, 0);

  std::vector<te::gm::Geometry*> vecGaps = getGaps(vecGeometries, 200.);

  std::vector<double> vecClass;
  vecClass.resize(vecGaps.size(), 1.);
  binaryUrbanRaster->rasterize(vecGaps, vecClass);

  return binaryUrbanRaster;
}

void te::urban::addIsolatedOpenPatches(te::rst::Raster* urbanRaster, te::rst::Raster* isolatedOpenPatchesRaster)
{
  assert(urbanRaster);
  assert(isolatedOpenPatchesRaster);

  std::size_t numRows = urbanRaster->getNumberOfRows();
  std::size_t numColumns = urbanRaster->getNumberOfColumns();

  if (numRows != isolatedOpenPatchesRaster->getNumberOfRows())
  {
    return;
  }
  if (numColumns != isolatedOpenPatchesRaster->getNumberOfColumns())
  {
    return;
  }

  for (std::size_t row = 0; row < numRows; ++row)
  {
    for (std::size_t column = 0; column < numColumns; ++numColumns)
    {
      double urbanRasterValue = 0.;
      urbanRaster->getValue((unsigned int)column, (unsigned int)row, urbanRasterValue);

      //if it is not  Rural Open Space, we do not chance the raster
      if (urbanRasterValue != OUTPUT_RURAL_OS)
      {
        continue;
      }

      double isolatedOpenPatchesRasterValue = 0.;
      isolatedOpenPatchesRaster->getValue((unsigned int)column, (unsigned int)row, isolatedOpenPatchesRasterValue);

      //if it is an isolated open patch, we set the raster value to 
      if (isolatedOpenPatchesRasterValue == 1)
      {
        urbanRaster->setValue((unsigned int)column, (unsigned int)row, OUTPUT_SUBURBAN_ZONE_OPEN_AREA);
      }
    }
  }
}

void te::urban::classifyIsolatedOpenPatches(te::rst::Raster* raster, const std::string& outputPath, const std::string& outputPrefix)
{
  std::string isolatedOpenPatchesRasterFileName = outputPath + "/" + outputPrefix + "_isolated_open_patches.tif";
  
  te::rst::Raster* isolatedOpenPatchesRaster = identifyIsolatedOpenPatches(raster, isolatedOpenPatchesRasterFileName);

  addIsolatedOpenPatches(raster, isolatedOpenPatchesRaster);

  //apagar o raster o disco também
  delete isolatedOpenPatchesRaster;
}

void te::urban::calculateUrbanIndexes(const std::string& inputFileName, double radius, std::map<std::string, double>& mapIndexes)
{
  std::auto_ptr<te::rst::Raster> inputRaster = openRaster(inputFileName);

  assert(inputRaster.get());

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
      std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster.get(), currentRow, currentColumn, radius);

      double permUrb = 0.;
      double value = calculateUrbanizedArea((short)centerPixel, vecPixels, permUrb);

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

      bool hasEdge = calculateEdge(inputRaster.get(), currentColumn, currentRow);
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

te::urban::UrbanRasters te::urban::prepareRaster(const std::string& inputFileName, double radius, const std::string& outputPath, const std::string& outputPrefix)
{
  std::string urbanizedPrefix = outputPrefix + "_urbanized";
  std::string footprintPrefix = outputPrefix + "_footprint";
  std::string footprintOpenAreaPrefix = outputPrefix + "_footprintOpenArea";

  UrbanRasters urbanRaster;

  //step 1 - classify the urbanized areas
  std::string urbanizedAreaFileName = outputPath + "/" + urbanizedPrefix + ".tif";
  urbanRaster .m_urbanizedAreaRaster.reset(classifyUrbanizedArea(inputFileName, radius, urbanizedAreaFileName));
  //urbanRaster.m_urbanizedAreaRaster = openRaster("D:\\Workspace\\FGV\\data\\belem_aug92_t90_final1_reclass_urbanized.tif");

  //step 2 - classify the urban footprints
  std::string urbanFootprintsFileName = outputPath + "/" + footprintPrefix + ".tif";
  urbanRaster.m_urbanFootprintRaster.reset(classifyUrbanFootprint(inputFileName, radius, urbanFootprintsFileName));
  //urbanRaster.m_urbanFootprintRaster = openRaster("D:\\Workspace\\FGV\\data\\belem_aug92_t90_final1_reclass_footprint.tif");

  //step 3 - classify fringe open areas
  std::string urbanFootprintsOpenAreaFileName = outputPath + "/" + footprintOpenAreaPrefix + ".tif";
  urbanRaster.m_urbanFootprintRaster.reset(classifyUrbanOpenArea(urbanRaster.m_urbanFootprintRaster.get(), radius, urbanFootprintsOpenAreaFileName));
  //urbanRaster.m_urbanFootprintRaster = openRaster("D:\\Workspace\\FGV\\data\\belem_aug92_t90_final1_reclass_footprintOpenArea.tif");

  //step 4 and 5- identify isolated patches and classify them into the given raster
  classifyIsolatedOpenPatches(urbanRaster.m_urbanizedAreaRaster.get(), outputPath, urbanizedPrefix);
  classifyIsolatedOpenPatches(urbanRaster.m_urbanFootprintRaster.get(), outputPath, footprintPrefix);

  return urbanRaster;
}

