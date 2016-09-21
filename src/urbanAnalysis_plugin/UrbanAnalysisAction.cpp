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
  \file fiocruz/src/fiocruz/flow/FlowClassifyAction.cpp

  \brief This file defines the Flow Classify Action class
*/

#include "../terralib_mod_growth_qt/UrbanAnalysisMainDialog.h"
#include "UrbanAnalysisAction.h"

// Terralib
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>


te::urban::UrbanAnalysisAction::UrbanAnalysisAction(QMenu* menu) 
  : te::urban::AbstractAction(menu)
{
  createAction(tr("Urban Analysis...").toStdString(), "");
}

te::urban::UrbanAnalysisAction::~UrbanAnalysisAction()
{
}

void te::urban::UrbanAnalysisAction::onActionActivated(bool checked)
{
  te::urban::qt::UrbanAnalysisMainDialog mainDlg;
  if (mainDlg.exec() == QDialog::Accepted)
  {
    
  }

  /*
  //get input layers
  std::list<te::map::AbstractLayerPtr> list = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  //show interface
  te::qt::plugins::fiocruz::FlowClassifyDialog dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  dlg.setLayerList(list);

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    //addNewLayer(dlg.getOutputLayer());
  }
  */
}
