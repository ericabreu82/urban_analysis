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
\file urban_analysis/src/growth/qt/SprawlMetricsWidget.cpp

\brief This class represents the Sprawl Metrics Widget class.
*/

#include "../terralib_mod_growth/Utils.h"
#include "SprawlMetricsWidget.h"
#include "Utils.h"
#include "ui_SprawlMetricsWidgetForm.h"

//Terralib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/raster/Band.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/qt/widgets/utils/ScopedCursor.h>
#include <terralib/qt/widgets/Utils.h>

//Boost
#include <boost/lexical_cast.hpp>

//Qt
#include <QFileDialog>
#include <QMessageBox>


te::urban::qt::SprawlMetricsWidget::SprawlMetricsWidget(bool startAsPlugin, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_ui(new Ui::SprawlMetricsWidgetForm),
  m_startAsPlugin(startAsPlugin)
{
  // add controls
  m_ui->setupUi(this);

  if (m_startAsPlugin)
  {
    m_ui->m_addImageToolButton->setIcon(QIcon::fromTheme("list-add"));
    m_ui->m_removeImageToolButton->setIcon(QIcon::fromTheme("list-remove"));
    m_ui->m_slopeRasterToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_cbdVecToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_studyAreaVecToolButton->setIcon(QIcon::fromTheme("folder"));
  }
  else
  {
    m_ui->m_addImageToolButton->setIcon(QIcon(":/images/list-add.svg"));
    m_ui->m_removeImageToolButton->setIcon(QIcon(":/images/list-remove.svg"));
    m_ui->m_slopeRasterToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_cbdVecToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_studyAreaVecToolButton->setIcon(QIcon(":/images/folder.svg"));
  }

  //connects
  connect(m_ui->m_addImageToolButton, SIGNAL(clicked()), this, SLOT(onAddImageToolButtonClicked()));
  connect(m_ui->m_removeImageToolButton, SIGNAL(clicked()), this, SLOT(onRemoveImageToolButtonClicked()));
  connect(m_ui->m_slopeRasterToolButton, SIGNAL(clicked()), this, SLOT(onSlopeRasterToolButtonClicked()));
  connect(m_ui->m_cbdVecToolButton, SIGNAL(clicked()), this, SLOT(onCBDVectorialDataToolButtonClicked()));
  connect(m_ui->m_studyAreaVecToolButton, SIGNAL(clicked()), this, SLOT(onStudyAreaVectorialDataToolButtonClicked()));
}

te::urban::qt::SprawlMetricsWidget::~SprawlMetricsWidget()
{

}

void te::urban::qt::SprawlMetricsWidget::onAddImageToolButtonClicked()
{
  QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Multiple Raster Files"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_img"), te::qt::widgets::GetDiskRasterFileSelFilter());

  if (!fileNames.isEmpty())
  {
    QFileInfo info(fileNames.value(0));

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_img");

    m_ui->m_imgFilesListWidget->addItems(fileNames);
  }
}

void te::urban::qt::SprawlMetricsWidget::onRemoveImageToolButtonClicked()
{
  qDeleteAll(m_ui->m_imgFilesListWidget->selectedItems());
}

void te::urban::qt::SprawlMetricsWidget::onSlopeRasterToolButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raster Files"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_img"), te::qt::widgets::GetDiskRasterFileSelFilter());

  if (!fileName.isEmpty())
  {
    QFileInfo info(fileName);

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_img");

    m_ui->m_slopeLineEdit->setText(fileName);
  }
}

void te::urban::qt::SprawlMetricsWidget::onCBDVectorialDataToolButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Vector File"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_vec"), tr("Esri Shapefile (*.shp *.SHP);; Mapinfo File (*.mif *.MIF);; GeoJSON (*.geojson *.GeoJSON);; GML (*.gml *.GML);; KML (*.kml *.KML);; All Files (*.*)"));

  if (!fileName.isEmpty())
  {
    QFileInfo info(fileName);

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_vec");

    m_ui->m_cbdVecLineEdit->setText(fileName);
  }
}

void te::urban::qt::SprawlMetricsWidget::onStudyAreaVectorialDataToolButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Vector File"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_vec"), tr("Esri Shapefile (*.shp *.SHP);; Mapinfo File (*.mif *.MIF);; GeoJSON (*.geojson *.GeoJSON);; GML (*.gml *.GML);; KML (*.kml *.KML);; All Files (*.*)"));

  if (!fileName.isEmpty())
  {
    QFileInfo info(fileName);

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_vec");

    m_ui->m_studyAreaVecLineEdit->setText(fileName);
  }
}

void te::urban::qt::SprawlMetricsWidget::execute()
{
  //check input parameters


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

  if (m_startAsPlugin)
  {
    //te::map::AbstractLayerPtr layer = te::urban::qt::CreateLayer(outPath, "GDAL");

    //emit layerCreated(layer);
  }

  te::common::ProgressManager::getInstance().removeViewer(dlgViewerId);
  delete dlgViewer;

  QMessageBox::information(this, tr("Urban Analysis"), tr("The execution finished with success."));
}
