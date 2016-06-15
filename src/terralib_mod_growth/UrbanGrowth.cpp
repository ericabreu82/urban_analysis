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


std::auto_ptr<te::rst::Raster> te::urban::classifyUrbanizedArea(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius)
{
  assert(inputRaster.get());

  std::auto_ptr<te::rst::Raster> outputRaster = cloneRasterIntoMem(inputRaster, false);

  assert(outputRaster.get());

  const short InputWater = inputClassesMap.find(INPUT_WATER)->second;
  const short InputUrban = inputClassesMap.find(INPUT_URBAN)->second;
  const short InputOther = inputClassesMap.find(INPUT_OTHER)->second;

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  int  maskSizeInPixels = te::rst::Round(radius / resX);

  boost::numeric::ublas::matrix<bool> mask = createRadiusMask(resX, radius);

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
      if (centerPixel == InputWater)
      {
        value = OUTPUT_WATER;
      }
      else if (centerPixel == InputUrban || centerPixel == InputOther)
      {
        //gets the pixels surrounding pixels that intersects the given radious
        std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster, currentRow, currentColumn, radius, mask);

        double permUrb = 0.;
        value = calculateUrbanizedArea((short)centerPixel, inputClassesMap, vecPixels, permUrb);
      }

      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);

      task.pulse();
    }
  }

  return outputRaster;
}

std::auto_ptr<te::rst::Raster> te::urban::classifyUrbanFootprint(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius)
{
  assert(inputRaster.get());

  std::auto_ptr<te::rst::Raster> outputRaster = cloneRasterIntoMem(inputRaster, false);

  assert(outputRaster.get());

  const short InputWater = inputClassesMap.find(INPUT_WATER)->second;
  const short InputUrban = inputClassesMap.find(INPUT_URBAN)->second;
  const short InputOther = inputClassesMap.find(INPUT_OTHER)->second;

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  boost::numeric::ublas::matrix<bool> mask = createRadiusMask(resX, radius);

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
      if (centerPixel == InputWater)
      {
        value = OUTPUT_WATER;
      }
      else if (centerPixel == InputOther)
      {
        value = OUTPUT_URBANIZED_OS;
      }
      else if (centerPixel == InputUrban)
      {
        //gets the pixels surrounding pixels that intersects the given radious
        std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster, currentRow, currentColumn, radius, mask);

        double permUrb = 0.;
        value = calculateUrbanFootprint((short)centerPixel, inputClassesMap, vecPixels, permUrb);
      }

      outputRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);

      task.pulse();
    }
  }

  return outputRaster;
}

void te::urban::classifyUrbanOpenArea(te::rst::Raster* urbanFootprintRaster, double radius)
{
  assert(urbanFootprintRaster);

  unsigned int numRows = urbanFootprintRaster->getNumberOfRows();
  unsigned int numColumns = urbanFootprintRaster->getNumberOfColumns();
  double resX = urbanFootprintRaster->getResolutionX();
  double resY = urbanFootprintRaster->getResolutionY();

  boost::numeric::ublas::matrix<bool> mask = createRadiusMask(resX, radius);

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
      urbanFootprintRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      double value = centerPixel;
      if (centerPixel == OUTPUT_URBANIZED_OS)
      {
        //gets the pixels surrounding pixels that intersects the given radious
        std::vector<short> vecPixels = getPixelsWithinRadious(urbanFootprintRaster, currentRow, currentColumn, radius, mask);

        value = calculateUrbanOpenArea((short)centerPixel, vecPixels);

        urbanFootprintRaster->setValue((unsigned int)currentColumn, (unsigned int)currentRow, value, 0);
      }

      task.pulse();
    }
  }
}

std::auto_ptr<te::rst::Raster> te::urban::identifyIsolatedOpenPatches(te::rst::Raster* raster)
{
  //we first need to create a binary image containing only the urban pixels
  std::auto_ptr<te::rst::Raster> binaryUrbanRaster = filterUrbanPixels(raster);

  //then we vectorize the result
  std::vector<te::gm::Geometry*> vecGeometries;
  binaryUrbanRaster->vectorize(vecGeometries, 0);

  te::rst::FillRaster(binaryUrbanRaster.get(), 0.);

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
    for (std::size_t column = 0; column < numColumns; ++column)
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
      if (isolatedOpenPatchesRasterValue == 1.)
      {
        urbanRaster->setValue((unsigned int)column, (unsigned int)row, OUTPUT_SUBURBAN_ZONE_OPEN_AREA);
      }
    }
  }
}

void te::urban::classifyIsolatedOpenPatches(te::rst::Raster* raster)
{
  std::auto_ptr<te::rst::Raster> isolatedOpenPatchesRaster = identifyIsolatedOpenPatches(raster);

  addIsolatedOpenPatches(raster, isolatedOpenPatchesRaster.get());
}

void te::urban::calculateUrbanIndexes(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius, UrbanIndexes& urbanIndexes)
{
  assert(inputRaster);

  const short InputUrban = inputClassesMap.find(INPUT_URBAN)->second;
  const short InputOther = inputClassesMap.find(INPUT_OTHER)->second;

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  boost::numeric::ublas::matrix<bool> mask = createRadiusMask(resX, radius);

  int numPix = 0;
  int edgeCount = 0; //edge index
  double sumPerUrb = 0;

  te::common::TaskProgress task("Calculating indexes");
  task.setTotalSteps((int)(numRows * numColumns));
  task.useTimer(true);

  //TODO: we must consider the given study area

  for (size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      //gets the value of the current center pixel
      double centerPixel = 0;
      inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

      task.pulse();

      if (centerPixel != InputUrban && centerPixel != InputOther)
      {
        continue;
      }

      //gets the pixels surrounding pixels that intersects the given radious
      std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster, currentRow, currentColumn, radius, mask);

      double permUrb = 0.;
      double value = calculateUrbanizedArea((short)centerPixel, inputClassesMap, vecPixels, permUrb);

      //sum the perviousness
      //TODO: we need to check if the pixel is in the study area
      if (centerPixel == InputOther)
      {
        sumPerUrb += permUrb;
        ++numPix;

        bool hasEdge = calculateEdge(inputRaster, inputClassesMap, currentColumn, currentRow);
        if (hasEdge == true)
        {
          ++edgeCount;
        }
      }
    }
  }

  double openness = 1 - (sumPerUrb / numPix);
  double edgeIndex = double(edgeCount) / numPix;

  urbanIndexes["openness"] = openness;
  urbanIndexes["edgeIndex"] = edgeIndex;
}

te::urban::UrbanRasters te::urban::prepareRaster(te::rst::Raster* inputRaster, const InputClassesMap& inputClassesMap, double radius, const std::string& outputPath, const std::string& outputPrefix)
{
  assert(inputRaster);

  //define the prefix to the file names
  std::string urbanizedPrefix = outputPrefix + "_urbanized";
  std::string footprintPrefix = outputPrefix + "_footprint";
  std::string footprintOpenAreaPrefix = outputPrefix + "_footprint_open_area";

  //define the file names
  std::string urbanizedAreaFileName = outputPath + "/" + urbanizedPrefix + ".tif";
  std::string urbanFootprintsFileName = outputPath + "/" + footprintPrefix + ".tif";
  std::string urbanFootprintsOpenAreaFileName = outputPath + "/" + footprintOpenAreaPrefix + ".tif";

  std::string urbanizedIsolatedOpenPatchesFileName = outputPath + "/" + urbanizedPrefix + "_isolated_open_patches.tif";
  std::string urbanFootprintsIsolatedOpenPatchesFileName = outputPath + "/" + footprintPrefix + "_isolated_open_patches.tif";

  UrbanRasters urbanRaster;

  //step 1 - classify the urbanized areas
  urbanRaster .m_urbanizedAreaRaster = classifyUrbanizedArea(inputRaster, inputClassesMap, radius);
  saveRaster(urbanizedAreaFileName, urbanRaster.m_urbanizedAreaRaster.get());

  //step 2 - classify the urban footprints
  urbanRaster.m_urbanFootprintRaster = classifyUrbanFootprint(inputRaster, inputClassesMap, radius);
  saveRaster(urbanFootprintsFileName, urbanRaster.m_urbanFootprintRaster.get());

  //step 3 - classify fringe open areas
  classifyUrbanOpenArea(urbanRaster.m_urbanFootprintRaster.get(), radius);
  saveRaster(urbanFootprintsOpenAreaFileName, urbanRaster.m_urbanFootprintRaster.get());

  //step 4 and 5- identify isolated patches and classify them into the given raster
  classifyIsolatedOpenPatches(urbanRaster.m_urbanizedAreaRaster.get());
  classifyIsolatedOpenPatches(urbanRaster.m_urbanFootprintRaster.get());

  saveRaster(urbanizedIsolatedOpenPatchesFileName, urbanRaster.m_urbanizedAreaRaster.get());
  saveRaster(urbanFootprintsIsolatedOpenPatchesFileName, urbanRaster.m_urbanFootprintRaster.get());

  return urbanRaster;
}

std::auto_ptr<te::rst::Raster> te::urban::compareRasterPeriods(const UrbanRasters& t1, const UrbanRasters& t2, const std::string& outputPath, const std::string& outputPrefix)
{
  assert(t1.m_urbanizedAreaRaster.get());
  assert(t2.m_urbanizedAreaRaster.get());

  std::string infillPrefix = outputPrefix + "_infill";
  std::string otherNewDevPrefix = outputPrefix + "_otherNewDev";
  std::string otherNewDevGroupedPrefix = outputPrefix + "_otherNewDevGrouped";

  std::string infillRasterFileName = outputPath + "/" + infillPrefix + ".tif";
  std::string otherNewDevRasterFileName = outputPath + "/" + otherNewDevPrefix + ".tif";
  std::string otherNewDevGroupedRasterFileName = outputPath + "/" + otherNewDevGroupedPrefix + ".tif";

  //1 - we first generate the infill raster and the other dev raster
  generateInfillOtherDevRasters(t1.m_urbanFootprintRaster.get(), t2.m_urbanFootprintRaster.get(), infillRasterFileName, otherNewDevRasterFileName);

  std::auto_ptr<te::rst::Raster> infillRaster = openRaster(infillRasterFileName);
  std::auto_ptr<te::rst::Raster> otherDevRaster = openRaster(otherNewDevRasterFileName);

  //2 - then we create distinct groups for each region of the other dev raster
  std::auto_ptr<te::rst::Raster> otherDevGroupedRaster = createDistinctGroups(otherDevRaster.get());
  saveRaster(otherNewDevGroupedRasterFileName, otherDevGroupedRaster.get());

  //3 - determine edge area groups
  std::set<double> setEdgeOpenAreaGroups = detectEdgeOpenAreaGroups(otherDevRaster.get(), otherDevGroupedRaster.get(), t1.m_urbanFootprintRaster.get());

  //4 - then we calculate the new development classification
  std::auto_ptr<te::rst::Raster> newDevelopmentRaster = classifyNewDevelopment(infillRaster.get(), otherDevGroupedRaster.get(), setEdgeOpenAreaGroups);
  return newDevelopmentRaster;
}