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

  //create dataset type
  std::auto_ptr<te::da::DataSetType> outDsType = createStatisticsDataSetType(outDataSetName, dsType.get(), calculateArea, calculateCount);

  //fill dataset
  std::auto_ptr<te::da::DataSet> inDataSet = ds->getDataSet(dataSetName);

  std::auto_ptr<te::mem::DataSet> outDataSet = createStatisticsDataSet(raster, outDsType.get(), inDataSet.get(), calculateArea, calculateCount);

  //save dataset
  std::auto_ptr<te::da::DataSource> outDs = te::urban::createDataSourceOGR(outPath);

  saveStatisticsDataSet(outDataSet.get(), outDsType.get(), outDs.get(), outDataSetName);
}

std::auto_ptr<te::da::DataSetType> te::urban::createStatisticsDataSetType(std::string dataSetName, te::da::DataSetType* inputDsType, 
                                                                          const bool& calculateArea, const bool& calculateCount)
{
  assert(inputDsType);

  std::auto_ptr<te::da::DataSetType> dsType(new te::da::DataSetType(dataSetName));

  for (std::size_t t = 0; t < inputDsType->getProperties().size(); ++t)
  {
    if (inputDsType->getProperty(t)->getName() == "FID" ||
        inputDsType->getProperty(t)->getName() == "fid")
      continue;

    te::dt::Property* p = inputDsType->getProperty(t)->clone();

    dsType->add(p);
  }

  if (calculateArea)
  {
    te::dt::SimpleProperty* infillProperty = new te::dt::SimpleProperty("if_area", te::dt::DOUBLE_TYPE);
    dsType->add(infillProperty);

    te::dt::SimpleProperty* leapfrogProperty = new te::dt::SimpleProperty("lf_area", te::dt::DOUBLE_TYPE);
    dsType->add(leapfrogProperty);

    te::dt::SimpleProperty* extensionProperty = new te::dt::SimpleProperty("ext_area", te::dt::DOUBLE_TYPE);
    dsType->add(extensionProperty);
  }

  if (calculateCount)
  {
    te::dt::SimpleProperty* infillProperty = new te::dt::SimpleProperty("if_count", te::dt::INT32_TYPE);
    dsType->add(infillProperty);

    te::dt::SimpleProperty* leapfrogProperty = new te::dt::SimpleProperty("lf_count", te::dt::INT32_TYPE);
    dsType->add(leapfrogProperty);

    te::dt::SimpleProperty* extensionProperty = new te::dt::SimpleProperty("ext_count", te::dt::INT32_TYPE);
    dsType->add(extensionProperty);
  }

  return dsType;
}

std::auto_ptr<te::mem::DataSet> te::urban::createStatisticsDataSet(te::rst::Raster* raster, te::da::DataSetType* dsType, te::da::DataSet* inputDs,
                                                                   const bool& calculateArea, const bool& calculateCount)
{
  int rasterSRID = raster->getSRID();
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

    //reproject geometry if its necessary
    if (geom->getSRID() != rasterSRID)
      geom->transform(rasterSRID);

    int if_count, lf_count, ext_count;
    double if_area, lf_area, ext_area;

    //calculate
    calculateStatisticsDataSet(raster, geom.get(), if_count, lf_count, ext_count, if_area, lf_area, ext_area, calculateArea, calculateCount);

    if (calculateArea)
    {
      item->setDouble("if_area", if_area);
      item->setDouble("lf_area", lf_area);
      item->setDouble("ext_area", ext_area);
    }

    if (calculateCount)
    {
      item->setInt32("if_count", if_count);
      item->setInt32("lf_count", lf_count);
      item->setInt32("ext_count", ext_count);
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

void te::urban::calculateStatisticsDataSet(te::rst::Raster* raster, te::gm::Geometry* geom,
                                           int& if_count, int& lf_count, int& ext_count,
                                           double& if_area, double& lf_area, double& ext_area,
                                           const bool& calculateArea, const bool& calculateCount)
{
  //infill       outputValue = 1;
  if_count = 0;
  if_area = 0.;

  //extensio     outputValue = 2;
  ext_count = 0;
  ext_area = 0.;

  //leapfrog     outputValue = 3;
  lf_count = 0;
  lf_area = 0.;

  //raster pixel area
  double pixelArea = raster->getResolutionX() * raster->getResolutionY();

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

      if (value == 1)
        if_count++;
      else if (value == 2)
        ext_count++;
      else if (value == 3)
        lf_count++;

      ++it;
    }
  }

  if_area  = if_count  * pixelArea;
  ext_area = ext_count * pixelArea;
  lf_area  = lf_count  * pixelArea;
}
