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
\file urbanAnalysisApp/main.cpp

\brief It contains the main routine of urbanAnalysisApp.
*/

// Terralib
#include <terralib/core/utils/Platform.h>
#include <terralib/core/logger/Logger.h>

// UrbanAnalysis
#include "../terralib_mod_growth_qt/UrbanAnalysisMainDialog.h"

// Qt
#include <QApplication>

#define MY_LOG_WARN(message) TE_CORE_LOG_WARN("mylogger", message)

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  qApp->setApplicationName("UrbanAnalysis");
  qApp->setOrganizationName("TE_3RDPARTY");

  //Initializing terralib default logger
  TE_INIT_DEFAULT_LOGGER("logs/urbanAnalysis.log");

  //Adding a logger from a configuration file.
  TE_ADD_LOGGER_FROM_FILE(te::core::FindInTerraLibPath("share/terralib/config/te-log.ini"));


  //TE_LOG_TRACE("This is a trace log.");
  //TE_LOG_INFO("This is a info log");
  //TE_LOG_ERROR("This is a error log");

  //te::core::Logger::instance().log("This is a trace log INFO.", "urbanAnalysis", boost::log::trivial::info);
  //te::core::Logger::instance().log("This is a trace log WARNING.", "urbanAnalysis", boost::log::trivial::warning);
  //te::core::Logger::instance().log("This is a trace log ERROR.", "urbanAnalysis", boost::log::trivial::error);
  //te::core::Logger::instance().log("This is a trace log FATAL.", "urbanAnalysis", boost::log::trivial::fatal);

  //Adding a new logger without configuration file.
  TE_ADD_LOGGER("mylogger", "log/mylogs.log", "[%TimeStamp%]{%ThreadID%} %Process%(%ProcessID%) <%Severity%>: %Message%");

  //MY_LOG_WARN("This is a warning log.");


  int waitVal = EXIT_FAILURE;

  //start main dlg
  te::urban::qt::UrbanAnalysisMainDialog mainDlg;

  if (mainDlg.exec() == QDialog::Accepted)
  {
    waitVal = EXIT_SUCCESS;
  }

  return waitVal;
}
