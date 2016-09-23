/*  Copyright (C) 2008-2013 National Institute For Space Research (INPE) - Brazil.

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
  \file Plugin.cpp

  \brief Plugin implementation for the UrbanAnalysis Qt Plugin.
*/

#include "Plugin.h"
#include "UrbanAnalysisAction.h"

// TerraLib
#include <terralib/core/translator/Translator.h>
#include <terralib/core/logger/Logger.h>
#include <terralib/qt/af/ApplicationController.h>

// QT
#include <QMenu>
#include <QMenuBar>

te::urban::Plugin::Plugin(const te::plugin::PluginInfo& pluginInfo)
  : QObject()
  , te::plugin::Plugin(pluginInfo)
  , m_menu(0)
  , m_urbanAnalysisAction(0)
{
}

te::urban::Plugin::~Plugin()
{

}

void te::urban::Plugin::startup()
{
  if(m_initialized)
    return;

  te::qt::af::AppCtrlSingleton::getInstance().addListener(this, te::qt::af::SENDER);

  TE_LOG_TRACE(TE_TR("UrbanAnalysis Plugin startup!"));

// add plugin menu
  m_menu = te::qt::af::AppCtrlSingleton::getInstance().getMenu("UrbanAnalysis");
  m_menu->setTitle(TE_TR("UrbanAnalysis"));

// register actions
  registerActions();

  m_initialized = true;
}

void te::urban::Plugin::shutdown()
{
  if(!m_initialized)
    return;

// remove menu
  delete m_menu;
  m_menu = 0;

// unregister actions
  unRegisterActions();

  TE_LOG_TRACE(TE_TR("UrbanAnalysis Plugin shutdown!"));

  m_initialized = false;
}

void te::urban::Plugin::registerActions()
{
  m_urbanAnalysisAction = new te::urban::UrbanAnalysisAction(m_menu);
  connect(m_urbanAnalysisAction, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
}

void  te::urban::Plugin::unRegisterActions()
{
  delete m_urbanAnalysisAction;
  m_urbanAnalysisAction = 0;
}

PLUGIN_CALL_BACK_IMPL(te::urban::Plugin)
