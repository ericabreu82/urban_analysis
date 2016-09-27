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
\file urban_analysis/src/growth/qt/SlopeWidget.h

\brief This class represents the Slope Widget class.
*/

#ifndef __URBANANALYSIS_INTERNAL_URBANALYSIS_SLOPE_QT_WIDGET_H
#define __URBANANALYSIS_INTERNAL_URBANALYSIS_SLOPE_QT_WIDGET_H

#include "Config.h"

// Terralib
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <memory>

// Qt
#include <QWidget>

namespace Ui { class SlopeWidgetForm; }

namespace te
{
  namespace urban
  {
    namespace qt
    {
      class TEGROWTHQTEXPORT SlopeWidget : public QWidget
      {
        Q_OBJECT

        public:

          SlopeWidget(bool startAsPlugin, QWidget* parent = 0, Qt::WindowFlags f = 0);

          ~SlopeWidget();

        public slots:

          void onAddImageToolButtonClicked();

          void onOutputRepoToolButtonClicked();

          void execute();

        signals:

          void layerCreated(te::map::AbstractLayerPtr layer);

        private:

          std::auto_ptr<Ui::SlopeWidgetForm> m_ui;

          bool m_startAsPlugin;
      };
    }
  }
}

#endif //__URBANANALYSIS_INTERNAL_URBANALYSIS_SLOPE_QT_WIDGET_H
