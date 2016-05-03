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
\file urban_analysis/src/growth/qt/ReclassifyWidget.cpp

\brief This class represents the Reclassify Widget class.
*/

#include "ReclassifyWidget.h"
#include "ui_ReclassifyWidgetForm.h"

#include "../terralib_mod_growth/UrbanGrowth.h"

//Qt
#include <QMessageBox>

te::urban::qt::ReclassifyWidget::ReclassifyWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_ui(new Ui::ReclassifyWidgetForm)
{
  // add controls
  m_ui->setupUi(this);
}

te::urban::qt::ReclassifyWidget::~ReclassifyWidget()
{

}

void te::urban::qt::ReclassifyWidget::execute()
{
  /*
  if (m_ui->m_reclassInputImageLineEdit->text().isEmpty() == true)
  {
    return;
  }
  if (m_ui->m_reclassRadiusLineEdit->text().isEmpty() == true)
  {
    return;
  }

  bool converted = false;
  double radius = m_ui->m_reclassRadiusLineEdit->text().toDouble(&converted);

  if (converted == false)
  {
    return;
  }

  if (m_ui->m_reclassOutputImageLineEdit->text().isEmpty() == true)
  {
    return;
  }

  std::string inputFileName = m_ui->m_reclassInputImageLineEdit->text().toStdString();
  std::string outputFileName = m_ui->m_reclassOutputImageLineEdit->text().toStdString();
  */

  std::string inputFileName = "C:\\Users\\Mário\\Google Drive\\Pessoal\\Projetos\\Miguel_Fred\\Dados\\belem_aug92_t90_final1.tif";
  double radius = 564.;
  std::string outputPath = "D:\\temp\\miguel_fred";
  std::string outputPrefix = "t1";

  UrbanRasters outputRaster = prepareRaster(inputFileName, radius, outputPath, outputPrefix);
}