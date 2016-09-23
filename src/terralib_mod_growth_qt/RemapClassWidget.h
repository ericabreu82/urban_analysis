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
\file urban_analysis/src/growth/qt/RemapClassWidget.h

\brief This class represents the Remap Class Widget class.
*/

#ifndef __URBANANALYSIS_INTERNAL_URBANALYSIS_REMAPCLASS_QT_WIDGET_H
#define __URBANANALYSIS_INTERNAL_URBANALYSIS_REMAPCLASS_QT_WIDGET_H

#include "Config.h"

// STL
#include <memory>

// Qt
#include <QWidget>

namespace Ui { class RemapClassWidgetForm; }

namespace te
{
  namespace urban
  {
    namespace qt
    {
      class TEGROWTHQTEXPORT RemapClassWidget : public QWidget
      {
        Q_OBJECT

        public:

          RemapClassWidget(bool startAsPlugin, QWidget* parent = 0, Qt::WindowFlags f = 0);

          ~RemapClassWidget();

        public slots:

          void onReclassAddImageToolButtonClicked();

          void onReclassRemoveImageToolButtonClicked();

          void onReclassOutputRepoToolButtonClicked();

          void execute();

        protected:

          void remap();

        private:

          std::auto_ptr<Ui::RemapClassWidgetForm> m_ui;

          bool m_startAsPlugin;
      };
    }
  }
}

#endif //__URBANANALYSIS_INTERNAL_URBANALYSIS_REMAPCLASS_QT_WIDGET_H
