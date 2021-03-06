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
  \file fiocruz/src/fiocruz/AbstractAction.cpp

  \brief This file defines the abstract class AbstractAction
*/

#include "AbstractAction.h"

// Terralib
#include <terralib/qt/af/events/LayerEvents.h>
#include <terralib/qt/af/ApplicationController.h>

// STL
#include <cassert>

te::urban::AbstractAction::AbstractAction(QMenu* menu) :
  m_menu(menu), 
  m_action(0)
{
}

te::urban::AbstractAction::~AbstractAction()
{
  // do not delete m_action pointer because its son of fiocruz menu... and qt delete automatically
}

void te::urban::AbstractAction::createAction(std::string name, std::string pixmap)
{
  assert(m_menu);

  m_action = new QAction(m_menu);

  m_action->setText(name.c_str());

  if(pixmap.empty() == false)
    m_action->setIcon(QIcon::fromTheme(pixmap.c_str()));

  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(onActionActivated(bool)));

  m_menu->addAction(m_action);
}

void te::urban::AbstractAction::addNewLayer(te::map::AbstractLayerPtr layer)
{
  te::qt::af::evt::LayerAdded evt(layer.get());

  emit triggered(&evt);
}

std::list<te::map::AbstractLayerPtr> te::urban::AbstractAction::getLayers()
{
  te::qt::af::evt::GetAvailableLayers e;

  emit triggered(&e);

  return e.m_layers;
}
