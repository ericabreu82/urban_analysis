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

#include <terralib/common.h>
#include <terralib/common/TerraLib.h>
#include <terralib/geometry/Coord2D.h>
#include <terralib/geometry/Point.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/memory/CachedRaster.h>
#include <terralib/plugin.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/RasterSummary.h>
#include <terralib/raster/RasterSummaryManager.h>
#include <terralib/raster/Utils.h>

void te::urban::calculateUrbanCentroid(te::rst::Raster* raster, double& urbanArea, double& centroidX, double& centroidY)
{
  assert(raster);

  double resX = raster->getResolutionX();
  double resY = raster->getResolutionY();

  std::vector<te::gm::Coord2D> vecUrbanCoordinates;
  getUrbanCoordinates(raster, vecUrbanCoordinates);

  urbanArea = 0.;
  centroidX = 0.;
  centroidY = 0.;

  double sumX = 0.;
  double sumY = 0.;

  size_t size = vecUrbanCoordinates.size();
  for (std::size_t i = 0; i < size; ++i)
  {
    te::gm::Coord2D coord = vecUrbanCoordinates[i];
    sumX += coord.getX();
    sumY += coord.getY();
  }

  centroidX = sumX / size;
  centroidY = sumY / size;
  urbanArea = size * (resX * resY);
}

void te::urban::calculateCohesionIndex(te::rst::Raster* raster, double& averageDistance, double& averageDistanceSquare)
{
  assert(raster);

  averageDistance = 0.; 
  averageDistanceSquare = 0.;

  const std::size_t sampleSize = 1000;
  const std::size_t  numSamples = 30;

  std::vector<te::gm::Coord2D> vecUrbanCoordinates;
  getUrbanCoordinates(raster, vecUrbanCoordinates);

  double sumDistance = 0.;
  double sumDistanceSquare = 0.;
  std::size_t totalSamples = 0;

  for (std::size_t s = 0; s < numSamples; ++s)
  {
    std::vector<te::gm::Coord2D> vecSubSet = getRandomCoordSubset(vecUrbanCoordinates, sampleSize);

    for (std::size_t i = 0; i < sampleSize; ++i)
    {
      te::gm::Coord2D c1 = vecSubSet[i];

      for (std::size_t j = 0; j < sampleSize; ++j)
      {
        te::gm::Coord2D c2 = vecSubSet[j];

        double distance = TeDistance(c1, c2);
        //we do not consider equal coordinates
        if (distance == 0.)
        {
          continue;
        }
        
        sumDistance += distance;
        sumDistanceSquare += (distance * distance);
      }
    }
  }

  averageDistance = sumDistance / totalSamples;
  averageDistanceSquare = sumDistanceSquare / totalSamples;
}
