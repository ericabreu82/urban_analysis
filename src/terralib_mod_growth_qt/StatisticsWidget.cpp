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
\file urban_analysis/src/growth/qt/StatisticsWidget.cpp

\brief This class represents the Statistics Widget class.
*/

#include "StatisticsWidget.h"
#include "ui_StatisticsWidgetForm.h"

//Terralib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/raster/Band.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/qt/widgets/utils/ScopedCursor.h>
#include <terralib/qt/widgets/Utils.h>

//Boost
#include <boost/lexical_cast.hpp>

//Qt
#include <QFileDialog>
#include <QMessageBox>


te::urban::qt::StatisticsWidget::StatisticsWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_ui(new Ui::StatisticsWidgetForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_addImageToolButton->setIcon(QIcon(":/images/folder.svg"));
  m_ui->m_addVectorToolButton->setIcon(QIcon(":/images/folder.svg"));
  m_ui->m_outputRepoToolButton->setIcon(QIcon(":/images/folder.svg"));

  //connects
  connect(m_ui->m_addImageToolButton, SIGNAL(clicked()), this, SLOT(onAddImageToolButtonClicked()));
  connect(m_ui->m_addVectorToolButton, SIGNAL(clicked()), this, SLOT(onAddVectorToolButtonClicked()));
  connect(m_ui->m_outputRepoToolButton, SIGNAL(clicked()), this, SLOT(onOutputRepoToolButtonClicked()));
}

te::urban::qt::StatisticsWidget::~StatisticsWidget()
{

}

void te::urban::qt::StatisticsWidget::onAddImageToolButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raster Files"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_img"), te::qt::widgets::GetDiskRasterFileSelFilter());

  if (!fileName.isEmpty())
  {
    QFileInfo info(fileName);

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_img");

    m_ui->m_imageLineEdit->setText(fileName);
  }
}

void te::urban::qt::StatisticsWidget::onAddVectorToolButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Vector File"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_vec"), tr("Esri Shapefile (*.shp *.SHP);; Mapinfo File (*.mif *.MIF);; GeoJSON (*.geojson *.GeoJSON);; GML (*.gml *.GML);; KML (*.kml *.KML);; All Files (*.*)"));

  if (!fileName.isEmpty())
  {
    QFileInfo info(fileName);

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_vec");

    m_ui->m_vectorLineEdit->setText(fileName);
  }
}

void te::urban::qt::StatisticsWidget::onOutputRepoToolButtonClicked()
{
  QString dirName = QFileDialog::getSaveFileName(this, tr("Select output data location"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_outDir"));

  if (!dirName.isEmpty())
  {
    te::qt::widgets::AddFilePathToSettings(dirName, "urbanAnalysis_outDir");

    m_ui->m_outputRepoLineEdit->setText(dirName);
  }
}

void te::urban::qt::StatisticsWidget::execute()
{
  //check input parameters
  if (m_ui->m_imageLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select the input image data."));
    return;
  }

  if (m_ui->m_vectorLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select the input vector data."));
    return;
  }

  if (m_ui->m_outputRepoLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select the output repository location."));
    return;
  }

  //add task viewer
  te::qt::widgets::ProgressViewerDialog* dlgViewer = new te::qt::widgets::ProgressViewerDialog(this);
  int dlgViewerId = te::common::ProgressManager::getInstance().addViewer(dlgViewer);


  try
  {
   
  }
  catch (const std::exception& e)
  {
    te::common::ProgressManager::getInstance().removeViewer(dlgViewerId);
    delete dlgViewer;

    QString message = tr("Error in the execution.");
    if (e.what() != 0)
    {
      message += QString("\n");
      message += QString(e.what());
    }

    QMessageBox::information(this, tr("Urban Analysis"), message);
    return;
  }

  te::common::ProgressManager::getInstance().removeViewer(dlgViewerId);
  delete dlgViewer;

  QMessageBox::information(this, tr("Urban Analysis"), tr("The execution finished with success."));
}
