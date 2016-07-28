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

#ifndef __URBANANALYSIS_INTERNAL_GROWTH_UTILS_H
#define __URBANANALYSIS_INTERNAL_GROWTH_UTILS_H

#include "Config.h"

#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/raster/Raster.h>

#include <boost/numeric/ublas/matrix.hpp>

#include <ctime>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace te
{
  namespace da
  {
    class DataSetType;
  }

  namespace gm
  {
    struct Coord2D;
    class Geometry;
  }

  namespace mem
  {
    class DataSet;
  }

  namespace urban
  {
    enum InputUrbanClasses
    {
      INPUT_NODATA = 0, INPUT_WATER = 1, INPUT_URBAN = 2, INPUT_OTHER = 3
    };

    enum OutputUrbanClasses
    {
      OUTPUT_NO_DATA = 0, OUTPUT_URBAN = 1, OUTPUT_SUB_URBAN = 2, OUTPUT_RURAL = 3, OUTPUT_URBANIZED_OS = 4, OUTPUT_SUBURBAN_ZONE_OPEN_AREA = 5, OUTPUT_RURAL_OS = 6, OUTPUT_WATER = 7
    };

    typedef std::map<InputUrbanClasses, short> InputClassesMap;

    struct UrbanRasters
    {
      UrbanRasters()
        : m_urbanizedAreaRaster(0)
        , m_urbanFootprintRaster(0)
      {

      }

      std::auto_ptr<te::rst::Raster> m_urbanizedAreaRaster;
      std::auto_ptr<te::rst::Raster> m_urbanFootprintRaster;
    };

    struct Timer
    {
      Timer()
        : m_startTime(clock())
      {
      }

      double getElapsedTimeInSeconds()
      {
        clock_t endTime = clock();
        double elapsedTime = double(endTime - m_startTime) / CLOCKS_PER_SEC;

        return elapsedTime;
      }

      double getElapsedTimeMinutes()
      {
        double timeInSeconds = getElapsedTimeInSeconds();
        double timeInMinutes = timeInSeconds / 60.;
        return timeInMinutes;
      }

      clock_t m_startTime;
    };

    TEGROWTHEXPORT void init();

    TEGROWTHEXPORT void finalize();

    TEGROWTHEXPORT void initLogger(const std::string& logFileName);

    TEGROWTHEXPORT void logInfo(const std::string& message);

    TEGROWTHEXPORT void logWarning(const std::string& message);

    TEGROWTHEXPORT void logError(const std::string& message);

    TEGROWTHEXPORT void removeAllLoggers();

    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> openRaster(const std::string& fileName);

    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> cloneRasterIntoMem(te::rst::Raster* raster, bool copyData);

    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> createRaster(const std::string& fileName, te::rst::Raster* raster);
    
    TEGROWTHEXPORT std::auto_ptr<te::da::DataSource> createDataSourceOGR(const std::string& fileName);

    TEGROWTHEXPORT void saveRaster(const std::string& fileName, te::rst::Raster* raster);

    TEGROWTHEXPORT void saveVector(const std::string& fileName, const std::string& filePath, const std::vector<te::gm::Geometry*>& vecGeometries, const int& srid);

    TEGROWTHEXPORT te::gm::Geometry* createGeometryCollection(const std::vector<te::gm::Geometry*> vecGeometries);

    TEGROWTHEXPORT boost::numeric::ublas::matrix<bool> createRadiusMask(double resolution, double radius);

    TEGROWTHEXPORT bool needNormalization(te::rst::Raster* inputRaster, te::rst::Raster* referenceRaster);

    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> normalizeRaster(te::rst::Raster* inputRaster, te::rst::Raster* referenceRaster);

    //!< Returns all the pixels within the given radius
    TEGROWTHEXPORT std::vector<short> getPixelsWithinRadious(te::rst::Raster* raster, std::size_t referenceRow, std::size_t referenceColumn, double radius, const boost::numeric::ublas::matrix<bool>& mask);

    //!< Returns all the adjacent pixels
    TEGROWTHEXPORT std::vector<short> getAdjacentPixels(te::rst::Raster* raster, std::size_t referenceRow, std::size_t referenceColumn);

    //Calculate the urbanized area value based in the value of center pixel and in the value of the adjacent pixels
    TEGROWTHEXPORT double calculateUrbanizedArea(short centerPixelValue, const InputClassesMap& inputClassesMap, const std::vector<short>& vecPixels, double& permUrb);

    //Calculate the urban footprint value based in the value of center pixel and in the value of the adjacent pixels
    TEGROWTHEXPORT double calculateUrbanFootprint(short centerPixelValue, const InputClassesMap& inputClassesMap, const std::vector<short>& vecPixels, double& permUrb);

    //Calculate the urban open area value based in the value of center pixel and in the value of the adjacent pixels
    TEGROWTHEXPORT double calculateUrbanOpenArea(short centerPixelValue, const std::vector<short>& vecPixels);

    TEGROWTHEXPORT bool calculateEdge(te::rst::Raster* raster, const InputClassesMap& inputClassesMap, std::size_t column, std::size_t line);

    //this reclassification analyses the entire raster, where the output will be 1 if the pixel is urban and no_data if the pixel is not urban
    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> filterUrbanPixels(te::rst::Raster* raster, bool invertFilter);

    //!< Search for all the gaps (holes) that [optionally] have area smaller then the given reference area
    TEGROWTHEXPORT std::vector<te::gm::Geometry*> getGaps(const std::vector<te::gm::Geometry*>& vecCandidateGaps, double area = 0.);

    //!< For each region, creates a new group using sequential values
    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> createDistinctGroups(te::rst::Raster* inputRaster);

    //!< DETERMINE EDGE OPEN AREA (100 meter buffer around built-up)
    TEGROWTHEXPORT std::set<double> detectEdgeOpenAreaGroups(te::rst::Raster* otherNewDevRaster, te::rst::Raster* otherNewDevGroupedRaster, te::rst::Raster* footprintRaster);

    //compare the images in two diferrent times, creating two new classified images
    TEGROWTHEXPORT void generateInfillOtherDevRasters(te::rst::Raster* rasterT1, te::rst::Raster* rasterT2, const std::string& infillRasterFileName, const std::string& otherDevRasterFileName);

    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> classifyNewDevelopment(te::rst::Raster* infillRaster, te::rst::Raster* otherDevGroupedRaster, const std::set<double>& setEdgesOpenAreaGroups);

    // Mega faster distance method
    TEGROWTHEXPORT double TeDistance(const te::gm::Coord2D& c1, const te::gm::Coord2D& c2);

    //Gets a vector containing all the coordinates of urban pixels (classes = 1, 2, 4 and 5)
    TEGROWTHEXPORT void getUrbanCoordinates(te::rst::Raster* raster, std::vector<te::gm::Coord2D>& vecUrbanCoords);

    //Gets a random subset of the given coordinate vector 
    TEGROWTHEXPORT std::vector<te::gm::Coord2D> getRandomCoordSubset(const std::vector<te::gm::Coord2D>& vecUrbanCoords, std::size_t subsetSize);

    TEGROWTHEXPORT std::auto_ptr<te::rst::Raster> reclassify(te::rst::Raster* inputRaster, const std::map<int, int>& mapValues, int defaultValue);

    /*! Function used to create the output dataset type */
    TEGROWTHEXPORT std::auto_ptr<te::da::DataSetType> createDataSetType(std::string dataSetName, int srid);

    /*! Function used to create the output data */
    TEGROWTHEXPORT std::auto_ptr<te::mem::DataSet> createDataSet(te::da::DataSetType* dsType, const std::vector<te::gm::Geometry*>& geoms);

    /*! Function used to save the output dataset */
    TEGROWTHEXPORT void saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType, te::da::DataSource* ds, std::string dataSetName);

    TEGROWTHEXPORT std::vector<te::gm::Geometry*> fixGeometries(const std::vector<te::gm::Geometry*>& vecGeometries);

  }
}

#endif //__URBANANALYSIS_INTERNAL_GROWTH_UTILS_H
