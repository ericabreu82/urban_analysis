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
\file urban_analysis/src/growth/qt/Utils.cpp

\brief This class represents the Urban Analysis utils functions.
*/

#include "Utils.h"

// Terralib
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>

// BOOST
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

te::map::AbstractLayerPtr te::urban::qt::CreateLayer(const std::string& path, const std::string& type)
{
  te::map::AbstractLayerPtr layer;

  te::da::DataSourcePtr outDataSource = RegisterDataSource(path, type);

  std::string dataSetName = outDataSource->getDataSetNames()[0];

  te::qt::widgets::DataSet2Layer converter(outDataSource->getId());

  te::da::DataSetTypePtr dt(outDataSource->getDataSetType(dataSetName).release());

  layer = converter(dt);

  return layer;
}

te::da::DataSourcePtr te::urban::qt::RegisterDataSource(const std::string& path, const std::string& type)
{
  std::map<std::string, std::string> dsinfo;
  dsinfo["URI"] = path;

  te::da::DataSourceInfoPtr dsInfo(new te::da::DataSourceInfo);
  dsInfo->setConnInfo(dsinfo);
  dsInfo->setTitle(path);
  dsInfo->setAccessDriver(type);
  dsInfo->setType(type);
  dsInfo->setDescription(path);
  dsInfo->setId(GenerateRandomId());

  te::da::DataSourceInfoManager::getInstance().add(dsInfo);

  te::da::DataSourcePtr ds = te::da::DataSourceManager::getInstance().get(dsInfo->getId(), dsInfo->getType(), dsInfo->getConnInfo());

  return ds;
}

std::string te::urban::qt::GenerateRandomId()
{
  boost::uuids::basic_random_generator<boost::mt19937> gen;
  boost::uuids::uuid u = gen();
  return boost::uuids::to_string(u);
}
