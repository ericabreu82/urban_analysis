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

//Terralib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/qt/widgets/Utils.h>


//Boost
#include <boost/lexical_cast.hpp>

//Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QValidator>

te::urban::qt::ReclassifyWidget::ReclassifyWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_ui(new Ui::ReclassifyWidgetForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_reclassRadiusLineEdit->setValidator(new QDoubleValidator(this));

  m_ui->m_reclassAddImageToolButton->setIcon(QIcon(":/images/list-add.svg"));
  m_ui->m_reclassRemoveImageToolButton->setIcon(QIcon(":/images/list-remove.svg"));
  m_ui->m_reclassAddVecToolButton->setIcon(QIcon(":/images/folder.svg"));
  m_ui->m_reclassOutputRepoToolButton->setIcon(QIcon(":/images/folder.svg"));

  //connects
  connect(m_ui->m_reclassAddImageToolButton, SIGNAL(clicked()), this, SLOT(onReclassAddImageToolButtonClicked()));
  connect(m_ui->m_reclassRemoveImageToolButton, SIGNAL(clicked()), this, SLOT(onReclassRemoveImageToolButtonClicked()));
  connect(m_ui->m_reclassAddVecToolButton, SIGNAL(clicked()), this, SLOT(onReclassAddVecToolButtonClicked()));
  connect(m_ui->m_reclassOutputRepoToolButton, SIGNAL(clicked()), this, SLOT(onReclassOutputRepoToolButtonClicked()));
}

te::urban::qt::ReclassifyWidget::~ReclassifyWidget()
{

}

void te::urban::qt::ReclassifyWidget::onReclassAddImageToolButtonClicked()
{
  QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Multiple Raster Files"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_img"), te::qt::widgets::GetDiskRasterFileSelFilter());

  if (!fileNames.isEmpty())
  {
    QFileInfo info(fileNames.value(0));

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_img");

    m_ui->m_imgFilesListWidget->addItems(fileNames);
  }
}

void te::urban::qt::ReclassifyWidget::onReclassRemoveImageToolButtonClicked()
{
  qDeleteAll(m_ui->m_imgFilesListWidget->selectedItems());
}

void  te::urban::qt::ReclassifyWidget::onReclassAddVecToolButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Vector File"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_vec"), tr("Esri Shapefile (*.shp *.SHP);; Mapinfo File (*.mif *.MIF);; GeoJSON (*.geojson *.GeoJSON);; GML (*.gml *.GML);; KML (*.kml *.KML);; All Files (*.*)"));

  if (!fileName.isEmpty())
  {
    QFileInfo info(fileName);

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_vec");

    m_ui->m_vecFileLineEdit->setText(fileName);
  }
}

void te::urban::qt::ReclassifyWidget::onReclassOutputRepoToolButtonClicked()
{
  QString dirName = QFileDialog::getExistingDirectory(this, tr("Select output data location"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_outDir"));

  if (!dirName.isEmpty())
  {
    te::qt::widgets::AddFilePathToSettings(dirName, "urbanAnalysis_outDir");

    m_ui->m_reclassOutputRepoLineEdit->setText(dirName);
  }
}

void te::urban::qt::ReclassifyWidget::execute()
{
  //ERIC FUNC
  //std::string inputFileName = "D:\\Workspace\\FGV\\data\\belem_aug92_t90_final1.tif";
  //double r = 564.;
  //std::string oPath = "D:\\Workspace\\FGV\\temp";
  //std::string oPrefix = "belem_aug92_t90_final1_reclass";
  
  //ERIC HOME
  //std::string inputFileName = "D:\\Projects\\FGV\\data\\belem_aug92_t90_final1.tif";
  //double r = 564.;
  //std::string oPath = "D:\\Projects\\FGV\\temp";
  //std::string oPrefix = "t1";
  
  //MARIO
  //std::string inputFileName = "D:\\temp\\miguel_fred\\entrada\\belem_aug92_t90_final1.tif";
  //double r = 564.;
  //std::string oPath = "D:\\temp\\miguel_fred";
  //std::string oPrefix = "t1";


  //UrbanRasters outputRaster = prepareRaster(inputFileName, r, oPath, oPrefix);
  //return;

  //check input parameters
  if (m_ui->m_imgFilesListWidget->count() == 0)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select at least one input image."));
    return;
  }

  if (m_ui->m_reclassRadiusLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select input vector data."));
    return;
  }

  bool converted = false;
  double radius = m_ui->m_reclassRadiusLineEdit->text().toDouble(&converted);

  if (converted == false)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Invalid radius value."));
    return;
  }

  if (m_ui->m_reclassOutputRepoLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select the output repository location."));
    return;
  }

  if (m_ui->m_reclassOutputNameLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select the output data name."));
    return;
  }

  bool calculateIndexes = m_ui->m_indexesGroupBox->isChecked();

  std::string outputPath = m_ui->m_reclassOutputRepoLineEdit->text().toStdString();
  std::string outputPrefix = m_ui->m_reclassOutputNameLineEdit->text().toStdString();

  //add task viewer
  te::qt::widgets::ProgressViewerDialog* dlgViewer = new te::qt::widgets::ProgressViewerDialog(this);
  int dlgViewerId = te::common::ProgressManager::getInstance().addViewer(dlgViewer);

  //execute operation
  UrbanRasters urbanRaster_t_n0;
  UrbanRasters urbanRaster_t_n1;
  UrbanSummary urbanSummary;

  for (int i = 0; i < m_ui->m_imgFilesListWidget->count(); ++i)
  {
    std::string inputImgName = m_ui->m_imgFilesListWidget->item(i)->text().toStdString();
    std::string currentOutputPrefix = outputPrefix + "_t" + boost::lexical_cast<std::string>(i + 1);

    urbanRaster_t_n0 = urbanRaster_t_n1;
    urbanRaster_t_n1 = prepareRaster(inputImgName, radius, outputPath, currentOutputPrefix);

    if (calculateIndexes)
    {
      UrbanIndexes urbanIndexes;
      calculateUrbanIndexes(inputImgName, radius, urbanIndexes);

      urbanSummary[inputImgName] = urbanIndexes;
    }

    if (i > 0)
    {
      std::auto_ptr<te::rst::Raster> newDevelopmentRaster = compareRasterPeriods(urbanRaster_t_n0, urbanRaster_t_n1, outputPath, currentOutputPrefix);

      std::string newDevelopmentPrefix = outputPrefix + "_newDevelopment";
      std::string newDevelopmentRasterFileName = outputPath + "/" + newDevelopmentPrefix + ".tif";
      saveRaster(newDevelopmentRasterFileName, newDevelopmentRaster.get());
    }
  }

  if (calculateIndexes)
  {
    m_ui->m_tableWidget->setRowCount(0);

    UrbanSummary::iterator itSummary = urbanSummary.begin();

    while (itSummary != urbanSummary.end())
    {
      UrbanIndexes ui = itSummary->second;

      std::string imageName = itSummary->first;
      double opennes = ui["openness"];
      double edge = ui["edgeIndex"];

      int newrow = m_ui->m_tableWidget->rowCount();
      m_ui->m_tableWidget->insertRow(newrow);

      QTableWidgetItem* itemName = new QTableWidgetItem(QString::fromStdString(imageName));
      itemName->setFlags(Qt::ItemIsEnabled);
      m_ui->m_tableWidget->setItem(newrow, 0, itemName);

      QTableWidgetItem* itemOpennes = new QTableWidgetItem(QString::number(opennes));
      itemName->setFlags(Qt::ItemIsEnabled);
      m_ui->m_tableWidget->setItem(newrow, 1, itemOpennes);

      QTableWidgetItem* itemEdge = new QTableWidgetItem(QString::number(edge));
      itemName->setFlags(Qt::ItemIsEnabled);
      m_ui->m_tableWidget->setItem(newrow, 2, itemEdge);

      ++itSummary;
    }

    m_ui->m_tableWidget->resizeColumnsToContents();
  }

  te::common::ProgressManager::getInstance().removeViewer(dlgViewerId);
  delete dlgViewer;

  QMessageBox::information(this, tr("Urban Analysis"), tr("The execution finished with success."));
}