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
\file urban_analysis/src/growth/qt/ReclassifyWidget.h

\brief This class represents the Reclassify Widget class.
*/

#ifndef __URBANANALYSIS_INTERNAL_URBANALYSIS_RECLASSIFY_QT_WIDGET_H
#define __URBANANALYSIS_INTERNAL_URBANALYSIS_RECLASSIFY_QT_WIDGET_H

#include "Config.h"

// STL
#include <memory>

// Qt
#include <QWidget>

namespace Ui { class ReclassifyWidgetForm; }

namespace te
{
  namespace urban
  {
    namespace qt
    {
      class TEGROWTHQTEXPORT ReclassifyWidget : public QWidget
      {
        Q_OBJECT

        public:

          ReclassifyWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

          ~ReclassifyWidget();

        public slots:

          void onReclassAddImageToolButtonClicked();

          void onReclassRemoveImageToolButtonClicked();

          void onReclassAddVecToolButtonClicked();

          void onReclassOutputRepoToolButtonClicked();

          void onRemapCheckBoxClicked(bool flag);

          void execute();

        private:

          std::auto_ptr<Ui::ReclassifyWidgetForm> m_ui;
      };
    }
  }
}

#endif //__URBANANALYSIS_INTERNAL_URBANALYSIS_RECLASSIFY_QT_WIDGET_H
