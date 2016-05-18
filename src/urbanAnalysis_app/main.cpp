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

// UrbanAnalysis
#include "../terralib_mod_growth_qt/UrbanAnalysisMainDialog.h"

// Terralib
#include <terralib/common/progress/ConsoleProgressViewer.h>
#include <terralib/common/progress/ProgressManager.h>

// Qt
#include <QApplication>

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  qApp->setApplicationName("UrbanAnalysis");
  qApp->setOrganizationName("TE_3RDPARTY");

  int waitVal = EXIT_FAILURE;

  //add console progress viewer
  te::common::ConsoleProgressViewer cpv;
  int cpvId = te::common::ProgressManager::getInstance().addViewer(&cpv);

  //start main dlg
  te::urban::qt::UrbanAnalysisMainDialog mainDlg;

  if (mainDlg.exec() == QDialog::Accepted)
  {
    waitVal = EXIT_SUCCESS;
  }

  //remove progress viewer
  te::common::ProgressManager::getInstance().removeViewer(cpvId);

  return waitVal;
}