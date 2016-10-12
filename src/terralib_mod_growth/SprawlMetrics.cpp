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

#include "SprawlMetrics.h"

#include "Utils.h"
#include "Statistics.h"

#include <terralib/common.h>
#include <terralib/common/TerraLib.h>
#include <terralib/geometry/Coord2D.h>
#include <terralib/geometry/Point.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/geometry/Utils.h>
#include <terralib/memory/CachedRaster.h>
#include <terralib/plugin.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/RasterSummary.h>
#include <terralib/raster/RasterSummaryManager.h>
#include <terralib/raster/Utils.h>

void te::urban::calculateUrbanCentroid(te::rst::Raster* urbanRaster, double& urbanArea, te::gm::Coord2D& centroid)
{
  assert(urbanRaster);

  double resX = urbanRaster->getResolutionX();
  double resY = urbanRaster->getResolutionY();
  double onePixelArea = resX * resY;

  std::vector<te::gm::Coord2D> vecUrbanCoordinates;
  getUrbanCoordinates(urbanRaster, vecUrbanCoordinates);

  urbanArea = 0.;

  double sumX = 0.;
  double sumY = 0.;

  size_t size = vecUrbanCoordinates.size();
  for (std::size_t i = 0; i < size; ++i)
  {
    te::gm::Coord2D coord = vecUrbanCoordinates[i];
    sumX += coord.getX();
    sumY += coord.getY();
  }

  centroid.x = sumX / size;
  centroid.y = sumY / size;
  urbanArea = size * onePixelArea;
}

te::urban::UrbanIndexes te::urban::calculateProximityIndex(te::rst::Raster* urbanRaster, te::rst::Raster* landCoverRaster, te::rst::Raster* slopeRaster, const te::gm::Coord2D& centroidCBD, const te::gm::Coord2D& centroidUrban, double radius, double urbanAreaHA)
{
  double in_EAC = 0.;
  double in_nEAC = 0.;

  unsigned int numRows = urbanRaster->getNumberOfRows();
  unsigned int numColumns = urbanRaster->getNumberOfColumns();
  double resX = urbanRaster->getResolutionX();
  double resY = urbanRaster->getResolutionY();
  double onePixelArea = resX * resY;

  double sumDistance = 0.;
  double sumDistanceSquare = 0.;
  double count = 0;
  std::vector<double> urbanAreaDistanceVec;
  std::vector<double> nonUrbanAreaDistanceVec;

  for (std::size_t currentRow = 0; currentRow < numRows; ++currentRow)
  {
    for (std::size_t currentColumn = 0; currentColumn < numColumns; ++currentColumn)
    {
      te::gm::Coord2D currentCoord = urbanRaster->getGrid()->gridToGeo((double)currentColumn, (double)currentRow);
      double distanceToCBD = TeDistance(currentCoord, centroidCBD);
      double distanceToCentroidUrban = TeDistance(currentCoord, centroidUrban);

      double urbanValue = 0.;
      urbanRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, urbanValue);

      double landCoverValue = 0.;
      landCoverRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, landCoverValue);

      double slopeValue = 0.;
      slopeRaster->getValue((unsigned int)currentColumn, (unsigned int)currentRow, landCoverValue);

      if (urbanValue == OUTPUT_URBAN || urbanValue == OUTPUT_SUB_URBAN, urbanValue == OUTPUT_URBANIZED_OS || urbanValue == OUTPUT_SUBURBAN_ZONE_OPEN_AREA)
      {
        if (slopeValue == 0.)
        {
          urbanAreaDistanceVec.push_back(distanceToCBD);
        }

        sumDistance += distanceToCBD;
        sumDistanceSquare += (distanceToCentroidUrban * distanceToCentroidUrban);
        ++count;

        if (distanceToCBD <= radius)
          ++in_EAC;
      }
      else if (landCoverValue == INPUT_WATER || landCoverValue == INPUT_OTHER)
      {
        if (slopeValue == 0.)
        {
          nonUrbanAreaDistanceVec.push_back(distanceToCBD);
        }
      }
    }
  }

  //finalizes the proximity calculation
  double distanceToCenter = sumDistance / count;
  double distanceToCenterSquare = sumDistanceSquare / count;

  //net exchange index
  std::set<double> fullDistanceListSorted;
  fullDistanceListSorted.insert(urbanAreaDistanceVec.begin(), urbanAreaDistanceVec.end());
  fullDistanceListSorted.insert(nonUrbanAreaDistanceVec.begin(), nonUrbanAreaDistanceVec.end());

  int areaPix = 0;
  double referenceDistance = 0.;
  std::set<double>::iterator itFullList = fullDistanceListSorted.begin();
  while (itFullList != fullDistanceListSorted.end())
  {
    referenceDistance = (*itFullList);

    ++areaPix;
    if (areaPix >= count)
    {
      break;
    }

    ++itFullList;
  }

  double nEAC_r = referenceDistance;

  for (std::size_t i = 0; i < urbanAreaDistanceVec.size(); ++i)
  {
    double currentDistance = urbanAreaDistanceVec[i];
    if (currentDistance <= nEAC_r)
    {
      ++in_nEAC;
    }
  }

  // proximity index (circle / shape)...
  double circleDistance = radius * (2.0 / 3.0); //avg distance to center for equal area circle...
  double proximityIndex = circleDistance / distanceToCenter;

  // Spin index(circle / shape)...
  double circleMOI = .5 * (radius * radius); // moment of inertia for equal area circle...
  double spinIndex = circleMOI / distanceToCenterSquare;

  // Exchange index...
  in_EAC *= (onePixelArea * onePixelArea) / 10000; // class area in hectares
  double exchangeIndex = in_EAC / urbanAreaHA;

  // Net Exchange index...
  in_nEAC *= (onePixelArea * onePixelArea) / 10000.;
  double nExchangeIndex = in_nEAC / urbanAreaHA;

  UrbanIndexes mapIndexes;
  mapIndexes["proximity.ProximityIndex"] = proximityIndex;
  mapIndexes["proximity.SpinIndex"] = spinIndex;
  mapIndexes["proximity.ExchangeIndex"] = exchangeIndex;
  mapIndexes["proximity.NetExchangeIndex"] = nExchangeIndex;

  return mapIndexes;
}

te::urban::UrbanIndexes te::urban::calculateCohesionIndex(te::rst::Raster* urbanRaster, double radius)
{
  assert(urbanRaster);

  const std::size_t sampleSize = 1000;
  const std::size_t  numSamples = 30;

  std::vector<te::gm::Coord2D> vecUrbanCoordinates;
  getUrbanCoordinates(urbanRaster, vecUrbanCoordinates);

  std::size_t count = 0;
  double sumDistance = 0.;
  double sumDistanceSquare = 0.;

  for (std::size_t s = 0; s < numSamples; ++s)
  {
    std::vector<te::gm::Coord2D> vecSubSet = getRandomCoordSubset(vecUrbanCoordinates, sampleSize);

    for (std::size_t i = 0; i < sampleSize; ++i)
    {
      te::gm::Coord2D c1 = vecSubSet[i];

      for (std::size_t j = 0; j < sampleSize; ++j)
      {
        if (i == j)
        {
          continue;
        }

        te::gm::Coord2D c2 = vecSubSet[j];

        double distance = TeDistance(c1, c2);
        //we do not consider equal coordinates
        if (distance == 0.)
        {
          continue;
        }
        
        sumDistance += distance;
        sumDistanceSquare += (distance * distance);
        ++count;
      }
    }
  }

  double averageDistance = (sumDistance / count);
  double averageDistanceSquare = (sumDistanceSquare / count);

  double circleInterDistance = radius * 0.9054;
  double cohesionIndex = circleInterDistance / averageDistance;

  double circleInterDistanceSquare = radius * radius;
  double cohesionSquareIndex = circleInterDistanceSquare / averageDistanceSquare;

  UrbanIndexes mapIndexes;
  mapIndexes["cohesion.CohesionIndex"] = cohesionIndex;
  mapIndexes["cohesion.CohesionIndexSquare"] = cohesionSquareIndex;

  return mapIndexes;
}

te::urban::UrbanIndexes te::urban::calculateDepthIndex(te::rst::Raster* urbanRaster, te::gm::Geometry* studyArea, double radius)
{
  //1 - filter the nun-urban pixels and set them to 1. Urban pixels will be set to noDataValue
  std::vector<ReclassifyInfo> vecRemapInfo;
  vecRemapInfo.push_back(ReclassifyInfo(OUTPUT_RURAL, 1));
  vecRemapInfo.push_back(ReclassifyInfo(OUTPUT_RURAL_OS, OUTPUT_WATER, 1));
  std::auto_ptr<te::rst::Raster> binaryNonUrbanRaster = reclassify(urbanRaster, vecRemapInfo, SET_NEW_NODATA, 0);

  //2 - calculates the euclidean distance between the noDataValues and the valid pixels
  std::auto_ptr<te::rst::Raster> distanceRaster = calculateEuclideanDistance(binaryNonUrbanRaster.get());
  //saveRaster("D:/temp/miguel_fred/belem/depth.tif", distanceRaster.get());

  //3 - clip the region
  distanceRaster = clipRaster(distanceRaster.get(), studyArea);

  //4 - we filter all the values that are different from 0. To do this, we change the noDataValue and set it to 0
  distanceRaster->getBand(0)->getProperty()->m_noDataValue = 0.;

  //calculates the statistics of the image (mean and maximum values)
  const te::rst::RasterSummary* rsMean = te::rst::RasterSummaryManager::getInstance().get(distanceRaster.get(), te::rst::SUMMARY_MEAN);
  const te::rst::RasterSummary* rsMax = te::rst::RasterSummaryManager::getInstance().get(distanceRaster.get(), te::rst::SUMMARY_MAX);
  const std::complex<double>* cmean = rsMean->at(0).m_meanVal;
  const std::complex<double>* cmax = rsMax->at(0).m_maxVal;
  double mean = cmean->real();
  double max = cmax->real();

  //depth for equal area circle
  double circleDepth = radius / 3.;

  //calculate depth index(shape / circle)
  double depthIndex = mean / circleDepth;

  //calculate girth index(shape / circle)
  double girthIndex = max / radius; //girth of a circle is its radius

  UrbanIndexes mapIndexes;
  mapIndexes["depth.Depth"] = depthIndex;
  mapIndexes["depth.Girth"] = girthIndex;

  return mapIndexes;
}

te::urban::UrbanIndexes te::urban::calculateIndexes(const IndexesParams& params)
{
  double urbanArea = 0.; //the total built-up area
  te::gm::Coord2D centroidUrban; //the centroid of the built-up area
  std::map<std::string, double> mapFullIndexes;

  //we first calculate some information that will be used lately
  calculateUrbanCentroid(params.m_urbanRaster, urbanArea, centroidUrban);

  //and we also initialize some variablesthat will be used lately
  double radius = std::sqrt(urbanArea / GetConstantPI());
  double urbanAreaHA = urbanArea / 10000.;

  //here we calculate the proximity index
  if (params.m_calculateProximity)
  {
    //we calculate the slopes with a threshold of 15% and 30%
    double noDataValue = -1;

    std::vector<ReclassifyInfo> vecReclassify15;
    vecReclassify15.push_back(ReclassifyInfo(0, 15, 0));
    vecReclassify15.push_back(ReclassifyInfo(15, std::numeric_limits<double>::max(), 1));
    std::auto_ptr<te::rst::Raster> slopeRaster15 = reclassify(params.m_slopeRaster, vecReclassify15, SET_NEW_NODATA, 255);

    std::vector<ReclassifyInfo> vecReclassify30;
    vecReclassify15.push_back(ReclassifyInfo(0, 30, 0));
    vecReclassify15.push_back(ReclassifyInfo(30, std::numeric_limits<double>::max(), 1));
    std::auto_ptr<te::rst::Raster> slopeRaster30 = reclassify(params.m_slopeRaster, vecReclassify30, SET_NEW_NODATA, 255);

    std::vector<te::rst::Raster*> vecSlopeRasters;
    std::vector<std::string> vecSlopeThresholdTexts;

    vecSlopeRasters.push_back(slopeRaster15.get());
    vecSlopeThresholdTexts.push_back("15%");

    vecSlopeRasters.push_back(slopeRaster30.get());
    vecSlopeThresholdTexts.push_back("30%");

    for(std::size_t i = 0; i < vecSlopeRasters.size(); ++i)
    {
      te::rst::Raster* currentSlopeRaster = vecSlopeRasters[i];
      std::string currentThresholdText = vecSlopeThresholdTexts[i];

      UrbanIndexes mapIndexes = calculateProximityIndex(params.m_urbanRaster, params.m_landCoverRaster, currentSlopeRaster, params.m_centroidCBD, centroidUrban, radius, urbanAreaHA);

      mapFullIndexes["proximity.ProximityIndex_" + currentThresholdText] = mapIndexes["proximity.ProximityIndex"];;
      mapFullIndexes["proximity.SpinIndex_" + currentThresholdText] = mapIndexes["proximity.SpinIndex"];
      mapFullIndexes["proximity.ExchangeIndex_" + currentThresholdText] = mapIndexes["proximity.ExchangeIndex"];
      mapFullIndexes["proximity.NetExchangeIndex_" + currentThresholdText] = mapIndexes["proximity.NetExchangeIndex"];
    }
  }

  //here we calculate the cohesion index
  if (params.m_calculateCohesion)
  {
    UrbanIndexes mapIndexes = calculateCohesionIndex(params.m_urbanRaster, radius);
    mapFullIndexes.insert(mapIndexes.begin(), mapIndexes.end());
  }

  //here we calculate the depth index
  if (params.m_calculateDepth)
  {
    UrbanIndexes mapIndexes = calculateDepthIndex(params.m_urbanRaster, params.m_studyArea, radius);
    mapFullIndexes.insert(mapIndexes.begin(), mapIndexes.end());
  }

  return mapFullIndexes;
}
