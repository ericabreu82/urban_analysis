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

#include "../terralib_mod_growth/Statistics.h"
#include "../terralib_mod_growth/Utils.h"
#include "StatisticsWidget.h"
#include "ui_StatisticsWidgetForm.h"

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


te::urban::qt::StatisticsWidget::StatisticsWidget(bool startAsPlugin, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_ui(new Ui::StatisticsWidgetForm),
  m_startAsPlugin(startAsPlugin)
{
  // add controls
  m_ui->setupUi(this);

  if (m_startAsPlugin)
  {
    m_ui->m_addImageToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_addVectorToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_outputRepoToolButton->setIcon(QIcon::fromTheme("folder"));
  }
  else
  {
    m_ui->m_addImageToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_addVectorToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_outputRepoToolButton->setIcon(QIcon(":/images/folder.svg"));
  }

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
  QString fileName = QFileDialog::getSaveFileName(this, tr("Select output data location"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_outDir"), tr("Shape Files (*.shp *.SHP)"));

  if (!fileName.isEmpty())
  {
    QFileInfo file(fileName);

    if (file.suffix().isEmpty())
      fileName.append(".shp");

    m_ui->m_outputRepoLineEdit->setText(fileName);

    te::qt::widgets::AddFilePathToSettings(file.absolutePath(), "urbanAnalysis_outDir");
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
  std::string rasterFileName = m_ui->m_imageLineEdit->text().toStdString();

  if (m_ui->m_vectorLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select the input vector data."));
    return;
  }
  std::string ogrFileName = m_ui->m_vectorLineEdit->text().toStdString();

  if (m_ui->m_outputRepoLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select the output repository location."));
    return;
  }
  std::string outPath = m_ui->m_outputRepoLineEdit->text().toStdString();
  
  QFileInfo file(outPath.c_str());

  bool calculateArea = m_ui->m_areaCheckBox->isChecked();

  bool calculateCount = m_ui->m_countCheckBox->isChecked();

  //add task viewer
  te::qt::widgets::ProgressViewerDialog* dlgViewer = new te::qt::widgets::ProgressViewerDialog(this);
  int dlgViewerId = te::common::ProgressManager::getInstance().addViewer(dlgViewer);

  try
  {
    //get raster
    std::auto_ptr<te::rst::Raster> raster = te::urban::openRaster(rasterFileName);

    //get datasource
    std::auto_ptr<te::da::DataSource> ds = te::urban::createDataSourceOGR(ogrFileName);
    if (ds->getDataSetNames().empty())
    {
      QMessageBox::warning(this, tr("Urban Analysis"), tr("Error getting data set from data source."));
      return;
    }
    std::string dataSetName = ds->getDataSetNames()[0];

    te::urban::CalculateStatistics(raster.get(), ds.get(), dataSetName, calculateArea, calculateCount, outPath, file.baseName().toStdString());
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
