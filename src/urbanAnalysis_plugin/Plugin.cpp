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
  /*
  m_flowMenu = new QMenu(m_menu);
  m_flowMenu->setTitle(TE_TR("Flow"));
  m_menu->addMenu(m_flowMenu);

  m_regMenu = new QMenu(m_menu);
  m_regMenu->setTitle(TE_TR("Regionalization"));
  m_menu->addMenu(m_regMenu);
  */
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

// unregister actions
  unRegisterActions();

  TE_LOG_TRACE(TE_TR("UrbanAnalysis Plugin shutdown!"));

  m_initialized = false;
}

void te::urban::Plugin::registerActions()
{
  /*
#ifdef FIOCRUZ_HAVE_FLOWCLASSIFY
  m_flowClassify = new te::qt::plugins::fiocruz::FlowClassifyAction(m_flowMenu);
  connect(m_flowClassify, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef FIOCRUZ_HAVE_FLOWDIAGRAM
  m_flowDiagram = new te::qt::plugins::fiocruz::FlowDiagramAction(m_flowMenu);
  connect(m_flowDiagram, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));

  te::qt::plugins::fiocruz::FlowNetworkRendererFactory::initialize();
#endif

#ifdef FIOCRUZ_HAVE_FLOWNETWORK
  m_flowNetwork = new te::qt::plugins::fiocruz::FlowNetworkAction(m_flowMenu);
  connect(m_flowNetwork, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONRASTER
  m_regRaster = new te::qt::plugins::fiocruz::RegionalizationRasterAction(m_regMenu);
  connect(m_regRaster, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONVECTOR
  m_regVector = new te::qt::plugins::fiocruz::RegionalizationVectorAction(m_regMenu);
  connect(m_regVector, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif
  */
}

void  te::urban::Plugin::unRegisterActions()
{
  /*
#ifdef FIOCRUZ_HAVE_FLOWCLASSIFY
    delete m_flowClassify;
#endif

#ifdef FIOCRUZ_HAVE_FLOWDIAGRAM
    delete m_flowDiagram;

    te::qt::plugins::fiocruz::FlowNetworkRendererFactory::finalize();
#endif

#ifdef FIOCRUZ_HAVE_FLOWNETWORK
    delete m_flowNetwork;
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONRASTER
    delete m_regRaster;
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONVECTOR
    delete m_regVector;
#endif
    */
}

PLUGIN_CALL_BACK_IMPL(te::urban::Plugin)
