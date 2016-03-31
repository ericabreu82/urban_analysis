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
\file urban_analysis/src/growth/qt/UrbanGrowthDialog.h

\brief This class represents the urban growth dialog
*/

#ifndef __URBANANALYSIS_INTERNAL_GROWTH_URBANGROWTH_QT_DIALOG_H
#define __URBANANALYSIS_INTERNAL_GROWTH_URBANGROWTH_QT_DIALOG_H

#include "Config.h"

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class UrbanGrowthDialogForm; }

namespace te
{
  namespace urban
  {
    namespace qt
    {
      class TEGROWTHQTEXPORT UrbanGrowthDialog : public QDialog
      {
        Q_OBJECT

        public:

          UrbanGrowthDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

          ~UrbanGrowthDialog();

        private:

          std::auto_ptr<Ui::UrbanGrowthDialogForm> m_ui;
      };
    }
  }
}

#endif //__URBANANALYSIS_INTERNAL_GROWTH_URBANGROWTH_QT_DIALOG_H
