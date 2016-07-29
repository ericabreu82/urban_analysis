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
#include <terralib/common/STLUtils.h>
#include <terralib/raster/PositionIterator.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/Utils.h>

#include <boost/lexical_cast.hpp>


void te::urban::classifyUrbanizedArea(ClassifyParams* params)
{
  assert(params);

  Timer timer;

  te::rst::Raster* inputRaster = params->m_inputRaster;
  InputClassesMap inputClassesMap = params->m_inputClassesMap;
  double radius = params->m_radius;

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

  params->m_outputRaster.reset(outputRaster.release());

  logInfo("classifyUrbanizedArea for  " + inputRaster->getInfo()["URI"] + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
}

void te::urban::classifyUrbanFootprint(ClassifyParams* params)
{
  assert(params);

  Timer timer;

  te::rst::Raster* inputRaster = params->m_inputRaster;
  InputClassesMap inputClassesMap = params->m_inputClassesMap;
  double radius = params->m_radius;

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

  params->m_outputRaster.reset(outputRaster.release());

  logInfo("classifyUrbanFootprint for  " + inputRaster->getInfo()["URI"] + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
}

void te::urban::classifyUrbanOpenArea(te::rst::Raster* urbanFootprintRaster, double radius)
{
  Timer timer;

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

  logInfo("classifyUrbanOpenArea for  " + urbanFootprintRaster->getInfo()["URI"] + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
}

std::auto_ptr<te::rst::Raster> te::urban::identifyIsolatedOpenPatches(te::rst::Raster* raster, const std::string& outputPath, const std::string& outputPrefix)
{
  Timer timer;

  //we first need to create a binary image containing only the urban pixels
  std::auto_ptr<te::rst::Raster> binaryNonUrbanRaster = filterUrbanPixels(raster, true);
  std::string binaryInvertedFilePath = outputPath + "/" + outputPrefix + "_binary_inverted.tif";
  saveRaster(binaryInvertedFilePath, binaryNonUrbanRaster.get());

  //then we vectorize the result
  std::vector<te::gm::Geometry*> vecNonUrbanGeometries;
  binaryNonUrbanRaster->vectorize(vecNonUrbanGeometries, 0);

  std::vector<te::gm::Geometry*> vecFixedNonUrbanGeometries = te::urban::fixGeometries(vecNonUrbanGeometries);

  //export
  std::string vectorizedCandidatesFileName = outputPrefix + "_vectorized_candidates";
  std::string vectorizedCandidatesFilePath = outputPath + "/" + outputPrefix + "_vectorized_candidates.shp";
  saveVector(vectorizedCandidatesFileName, vectorizedCandidatesFilePath, vecFixedNonUrbanGeometries, raster->getSRID());

  te::rst::FillRaster(binaryNonUrbanRaster.get(), 0.);

  std::vector<te::gm::Geometry*> vecGaps = getGaps(vecFixedNonUrbanGeometries, 200.);

  te::common::FreeContents(vecNonUrbanGeometries);
  te::common::FreeContents(vecFixedNonUrbanGeometries);

  //export
  if (!vecGaps.empty())
  {
    std::string vectorizedGapsFileName = outputPrefix + "_gaps";
    std::string vectorizedGapsFilePath = outputPath + "/" + outputPrefix + "_gaps.shp";
    saveVector(vectorizedGapsFileName, vectorizedGapsFilePath, vecGaps, raster->getSRID());
  }

  std::vector<double> vecClass;
  vecClass.resize(vecGaps.size(), 1.);
  binaryNonUrbanRaster->rasterize(vecGaps, vecClass);

  te::common::FreeContents(vecGaps);

  logInfo("identifyIsolatedOpenPatches for  " + raster->getInfo()["URI"] + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");

  return binaryNonUrbanRaster;
}

void te::urban::addIsolatedOpenPatches(te::rst::Raster* urbanRaster, te::rst::Raster* isolatedOpenPatchesRaster)
{
  assert(urbanRaster);
  assert(isolatedOpenPatchesRaster);

  std::size_t numRows = urbanRaster->getNumberOfRows();
  std::size_t numColumns = urbanRaster->getNumberOfColumns();

  Timer timer;

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

  logInfo("addIsolatedOpenPatches for  " + urbanRaster->getInfo()["URI"] + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
}

void te::urban::classifyIsolatedOpenPatches(te::rst::Raster* raster, const std::string& outputPath, const std::string& outputPrefix)
{
  Timer timer;

  std::auto_ptr<te::rst::Raster> isolatedOpenPatchesRaster = identifyIsolatedOpenPatches(raster, outputPath, outputPrefix);

  addIsolatedOpenPatches(raster, isolatedOpenPatchesRaster.get());

  logInfo("classifyIsolatedOpenPatches for  " + raster->getInfo()["URI"] + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
}

void te::urban::calculateUrbanIndexes(CalculateUrbanIndexesParams* params)
{
  assert(params);

  Timer timer;

  te::rst::Raster* inputRaster = params->m_inputRaster;
  InputClassesMap inputClassesMap = params->m_inputClassesMap;
  double radius = params->m_radius;
  std::string spatialLimits = params->m_spatialLimits;

  const short InputUrban = inputClassesMap.find(INPUT_URBAN)->second;
  const short InputOther = inputClassesMap.find(INPUT_OTHER)->second;

  unsigned int numRows = inputRaster->getNumberOfRows();
  unsigned int numColumns = inputRaster->getNumberOfColumns();
  double resX = inputRaster->getResolutionX();
  double resY = inputRaster->getResolutionY();

  boost::numeric::ublas::matrix<bool> mask = createRadiusMask(resX, radius);

  int numUrbanPixels = 0;
  int edgeCount = 0; //edge index
  double sumPerUrb = 0;

  te::common::TaskProgress task("Calculating indexes");
  task.setTotalSteps((int)(numRows * numColumns));
  task.useTimer(true);

  const te::gm::Envelope* rasterEnvelope = inputRaster->getExtent();
  te::gm::Polygon* limitPolygon = (te::gm::Polygon*)te::gm::GetGeomFromEnvelope(rasterEnvelope, inputRaster->getSRID());

  te::rst::PolygonIterator<double> it = te::rst::PolygonIterator<double>::begin(inputRaster, limitPolygon);
  te::rst::PolygonIterator<double> itend = te::rst::PolygonIterator<double>::end(inputRaster, limitPolygon);

  while (it != itend)
  {
    unsigned int currentRow = it.getRow();
    unsigned int currentColumn = it.getColumn();

    //gets the value of the current center pixel
    double centerPixel = 0;
    inputRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, centerPixel);

    task.pulse();

    if (centerPixel != InputUrban)
    {
      ++it;
      continue;
    }

    //gets the pixels surrounding pixels that intersects the given radious
    std::vector<short> vecPixels = getPixelsWithinRadious(inputRaster, currentRow, currentColumn, radius, mask);

    double permUrb = 0.;
    double value = calculateUrbanizedArea((short)centerPixel, inputClassesMap, vecPixels, permUrb);

    //if the index could not be calculated, we continue to the next iteration
    if (value == OUTPUT_NO_DATA)
    {
      ++it;
      continue;
    }

    ++numUrbanPixels;

    sumPerUrb += permUrb;

    //then we check if there is at least one pixel that is not also urban in the adjacency
    bool hasEdge = calculateEdge(inputRaster, inputClassesMap, currentColumn, currentRow);
    if (hasEdge == true)
    {
      ++edgeCount;
    }

    ++it;
  }

  double openness = 1. - (sumPerUrb / numUrbanPixels);
  double edgeIndex = double(edgeCount) / numUrbanPixels;

  params->m_urbanIndexes["openness"] = openness;
  params->m_urbanIndexes["edgeIndex"] = edgeIndex;

  std::string message = "Indexes calculated for  " + inputRaster->getInfo()["URI"]  + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes";
  message += "\nopenness=" + boost::lexical_cast<std::string>(openness);
  message += "\nedgeIndex=" + boost::lexical_cast<std::string>(edgeIndex);

  logInfo(message);
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
  std::auto_ptr<ClassifyParams> urbanizedParams(new ClassifyParams());
  urbanizedParams->m_inputRaster = inputRaster;
  urbanizedParams->m_inputClassesMap = inputClassesMap;
  urbanizedParams->m_radius = radius;
  boost::thread threadStep1(&classifyUrbanizedArea, urbanizedParams.get());

  //step 2 - classify the urban footprints
  std::auto_ptr<ClassifyParams> footprintParams(new ClassifyParams());
  footprintParams->m_inputRaster = inputRaster;
  footprintParams->m_inputClassesMap = inputClassesMap;
  footprintParams->m_radius = radius;
  boost::thread threadStep2(&classifyUrbanFootprint, footprintParams.get());

  //we join step 2. step 2 normally finishes first. So we start step 3
  threadStep2.join();
  urbanRaster.m_urbanFootprintRaster = footprintParams->m_outputRaster;
  saveRaster(urbanFootprintsFileName, urbanRaster.m_urbanFootprintRaster.get());
  
  //step 3 - classify fringe open areas
  boost::thread threadStep3(&classifyUrbanOpenArea, urbanRaster.m_urbanFootprintRaster.get(), 100);
  threadStep3.join();
  saveRaster(urbanFootprintsOpenAreaFileName, urbanRaster.m_urbanFootprintRaster.get());

  //we join step 1
  threadStep1.join();
  urbanRaster.m_urbanizedAreaRaster = urbanizedParams->m_outputRaster;
  saveRaster(urbanizedAreaFileName, urbanRaster.m_urbanizedAreaRaster.get());

  //step 4 and 5- identify isolated patches and classify them into the given raster
  boost::thread threadIsolated1(&classifyIsolatedOpenPatches, urbanRaster.m_urbanizedAreaRaster.get(), outputPath, urbanizedPrefix);
  boost::thread threadIsolated2(&classifyIsolatedOpenPatches, urbanRaster.m_urbanFootprintRaster.get(), outputPath, footprintPrefix);

  threadIsolated1.join();
  threadIsolated2.join();

  saveRaster(urbanizedIsolatedOpenPatchesFileName, urbanRaster.m_urbanizedAreaRaster.get());
  saveRaster(urbanFootprintsIsolatedOpenPatchesFileName, urbanRaster.m_urbanFootprintRaster.get());

  return urbanRaster;
}

std::auto_ptr<te::rst::Raster> te::urban::compareRasterPeriods(const UrbanRasters& t1, const UrbanRasters& t2, const std::string& outputPath, const std::string& outputPrefix)
{
  Timer timer;

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

  logInfo("compareRasterPeriods for  " + newDevelopmentRaster->getInfo()["URI"] + " executed in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");

  return newDevelopmentRaster;
}