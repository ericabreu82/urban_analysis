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
\file urban_analysis/src/growth/qt/UrbanAnalysisMainDialog.h

\brief This class represents the Urban Analysis Main Dialog 
*/

#ifndef __URBANANALYSIS_INTERNAL_URBANALYSIS_MAINDLG_QT_DIALOG_H
#define __URBANANALYSIS_INTERNAL_URBANALYSIS_MAINDLG_QT_DIALOG_H

#include "Config.h"

// Terralib
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <memory>

// Qt
#include <QDialog>
#include <QGridLayout>
#include <QToolButton>

namespace Ui { class UrbanAnalysisMainDialogForm; }

namespace te
{
  namespace urban
  {
    namespace qt
    {
      class TEGROWTHQTEXPORT UrbanAnalysisMainDialog : public QDialog
      {
        Q_OBJECT

        public:

          UrbanAnalysisMainDialog(bool startAsPlugin, QWidget* parent = 0, Qt::WindowFlags f = 0);

          ~UrbanAnalysisMainDialog();

        public:

          std::list<te::map::AbstractLayerPtr> getLayers();

        protected:

          void buildDialog();

          QToolButton* createButton(std::string name);

          void createSpacer();

        protected slots:

          void onReclassifyToolButtonClicked();

          void onSprawlMetricsToolButtonClicked();

          void onRemapClassToolButtonClicked();

          void onStatisticsToolButtonClicked();

        private:

          std::auto_ptr<Ui::UrbanAnalysisMainDialogForm> m_ui;

          QWidget* m_currentWidget;

          QGridLayout* m_layout;

          int m_opButtonPos;

          bool m_startAsPlugin;

          std::list<te::map::AbstractLayerPtr> m_layers;
      };
    }
  }
}

#endif //__URBANANALYSIS_INTERNAL_URBANALYSIS_MAINDLG_QT_DIALOG_H
