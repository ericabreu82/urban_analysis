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
\file urban_analysis/src/growth/Statistics.h

\brief This class represents a list of algorithms to calculate statistics
*/

#include "Statistics.h"
#include "Utils.h"

#include <terralib/common/StringUtils.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/Utils.h>
#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/raster/PositionIterator.h>
#include <terralib/srs/Config.h>

void te::urban::CalculateStatistics(te::rst::Raster* raster, te::da::DataSource* ds, const std::string& dataSetName,
                                    const bool& calculateArea, const bool& calculateCount, const std::string& outPath, const std::string& outDataSetName)
{
  assert(raster);
  assert(ds);

  //validate SRID information
  if (raster->getSRID() == TE_UNKNOWN_SRS)
  {
    throw te::common::Exception("The SRID of the selected raster is invalid. Error in function: CalculateStatistics");
  }
  
  std::auto_ptr<te::da::DataSetType> dsType = ds->getDataSetType(dataSetName);

  if (!dsType.get())
  {
    throw te::common::Exception("Error getting data set from data source. Error in function: CalculateStatistics");
  }

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(dsType.get());

  if (!gp || gp->getSRID() == TE_UNKNOWN_SRS)
  {
    throw te::common::Exception("Invalid geometric property or SRID from data set. Error in function: CalculateStatistics");
  }

  std::auto_ptr<te::da::DataSet> inDataSet = ds->getDataSet(dataSetName);

  //create dataset type
  std::auto_ptr<te::da::DataSetType> outDsType = createStatisticsDataSetType(raster, outDataSetName, dsType.get(), calculateArea, calculateCount);

  //fill dataset
  std::auto_ptr<te::mem::DataSet> outDataSet = createStatisticsDataSet(raster, outDsType.get(), inDataSet.get(), calculateArea, calculateCount);

  //save dataset
  std::auto_ptr<te::da::DataSource> outDs = te::urban::createDataSourceOGR(outPath);

  saveStatisticsDataSet(outDataSet.get(), outDsType.get(), outDs.get(), outDataSetName);
}

std::auto_ptr<te::da::DataSetType> te::urban::createStatisticsDataSetType(te::rst::Raster* raster, std::string dataSetName, te::da::DataSetType* inputDsType,
                                                                          const bool& calculateArea, const bool& calculateCount)
{
  assert(inputDsType);

  std::map<double, unsigned int> values = raster->getBand(0)->getHistogramR();

  std::auto_ptr<te::da::DataSetType> dsType(new te::da::DataSetType(dataSetName));

  for (std::size_t t = 0; t < inputDsType->getProperties().size(); ++t)
  {
    if (inputDsType->getProperty(t)->getName() == "FID" ||
        inputDsType->getProperty(t)->getName() == "fid")
      continue;

    te::dt::Property* p = inputDsType->getProperty(t)->clone();

    dsType->add(p);
  }

  std::map<double, unsigned int>::iterator it;
  for (it = values.begin(); it != values.end(); ++it)
  {
    if (calculateCount)
    {
      std::string propName = "count_c" + te::common::Convert2String((int)it->first);
      te::dt::SimpleProperty* property = new te::dt::SimpleProperty("if_count", te::dt::INT32_TYPE);
      dsType->add(property);
    }

    if (calculateArea)
    {
      std::string propName = "area_c" + te::common::Convert2String((int)it->first);
      te::dt::SimpleProperty* property = new te::dt::SimpleProperty(propName, te::dt::DOUBLE_TYPE);
      dsType->add(property);
    }
  }

  return dsType;
}

std::auto_ptr<te::mem::DataSet> te::urban::createStatisticsDataSet(te::rst::Raster* raster, te::da::DataSetType* dsType, te::da::DataSet* inputDs,
                                                                   const bool& calculateArea, const bool& calculateCount)
{
  int rasterSRID = raster->getSRID();
  double rasterPixelArea = raster->getResolutionX() * raster->getResolutionY();

  std::auto_ptr<te::mem::DataSet> ds(new te::mem::DataSet(dsType));

  inputDs->moveBeforeFirst();

  while (inputDs->moveNext())
  {
    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(ds.get());

    std::auto_ptr<te::gm::Geometry> geom;

    for (std::size_t t = 0; t < inputDs->getNumProperties(); ++t)
    {
      if (inputDs->getPropertyName(t) == "FID" ||
        inputDs->getPropertyName(t) == "fid")
        continue;

      item->setValue(inputDs->getPropertyName(t), inputDs->getValue(inputDs->getPropertyName(t)).release());

      if (inputDs->getPropertyDataType(t) == te::dt::GEOMETRY_TYPE)
      {
        geom = inputDs->getGeometry(t);
      }
    }

    if (!geom.get())
      continue;

    std::map<int, std::size_t> pixelCountMap = computeStatistics(raster, geom.get());
    std::map<int, std::size_t>::iterator it;

    for (it = pixelCountMap.begin(); it != pixelCountMap.end(); ++it)
    {
      if (calculateCount)
      {
        std::string propName = "count_c" + te::common::Convert2String(it->first);
        item->setInt32(propName, (int)it->second);
      }

      if (calculateArea)
      {
        std::string propName = "area_c" + te::common::Convert2String(it->first);
        item->setDouble(propName, rasterPixelArea * (int)it->second);
      }
    }

    ds->add(item);
  }

  return ds;
}

void te::urban::saveStatisticsDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType, te::da::DataSource* ds, std::string dataSetName)
{
  //save dataset
  dataSet->moveBeforeFirst();

  std::map<std::string, std::string> options;

  ds->createDataSet(dsType, options);

  ds->add(dataSetName, dataSet, options);
}

std::map<int, std::size_t> te::urban::computeStatistics(te::rst::Raster* raster, te::gm::Geometry* geom)
{
  std::map<int, std::size_t> pixelCountMap;
  std::map<int, std::size_t>::iterator pixelCountMapIt;

  int rasterSRID = raster->getSRID();

  //reproject geometry if its necessary
  if (geom->getSRID() != rasterSRID)
    geom->transform(rasterSRID);

  std::vector<te::gm::Geometry*> geomVec;

  te::gm::Multi2Single(geom, geomVec);

  for (std::size_t g = 0; g < geomVec.size(); ++g)
  {
    te::gm::Polygon* polygon = dynamic_cast<te::gm::Polygon*>(geomVec[g]);

    if (!polygon)
      continue;

    te::rst::PolygonIterator<double> it = te::rst::PolygonIterator<double>::begin(raster, polygon);
    te::rst::PolygonIterator<double> itend = te::rst::PolygonIterator<double>::end(raster, polygon);

    while (it != itend)
    {
      double value = 0.;

      raster->getValue(it.getColumn(), it.getRow(), value, 0);

      pixelCountMapIt = pixelCountMap.find((int)value);

      if (pixelCountMapIt == pixelCountMap.end())
        pixelCountMap[(int)value] = 1;
      else
        pixelCountMap[(int)value] += 1;

      ++it;
    }
  }

  return pixelCountMap;
}
