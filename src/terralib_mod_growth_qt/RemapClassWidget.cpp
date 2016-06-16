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
\file urban_analysis/src/growth/qt/RemapClassWidget.cpp

  \brief This class represents the Remap Class Widget class.
*/

#include "RemapClassWidget.h"
#include "ui_RemapClassWidgetForm.h"

#include "../terralib_mod_growth/UrbanGrowth.h"

//Terralib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/raster/Band.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/qt/widgets/utils/ScopedCursor.h>
#include <terralib/qt/widgets/Utils.h>


//Boost
#include <boost/lexical_cast.hpp>

//Qt
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QValidator>

te::urban::qt::RemapClassWidget::RemapClassWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_ui(new Ui::RemapClassWidgetForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_reclassAddImageToolButton->setIcon(QIcon(":/images/list-add.svg"));
  m_ui->m_reclassRemoveImageToolButton->setIcon(QIcon(":/images/list-remove.svg"));
  m_ui->m_reclassOutputRepoToolButton->setIcon(QIcon(":/images/folder.svg"));

  //connects
  connect(m_ui->m_reclassAddImageToolButton, SIGNAL(clicked()), this, SLOT(onReclassAddImageToolButtonClicked()));
  connect(m_ui->m_reclassRemoveImageToolButton, SIGNAL(clicked()), this, SLOT(onReclassRemoveImageToolButtonClicked()));
  connect(m_ui->m_reclassOutputRepoToolButton, SIGNAL(clicked()), this, SLOT(onReclassOutputRepoToolButtonClicked()));
}

te::urban::qt::RemapClassWidget::~RemapClassWidget()
{

}

void te::urban::qt::RemapClassWidget::onReclassAddImageToolButtonClicked()
{
  QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Multiple Raster Files"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_img"), te::qt::widgets::GetDiskRasterFileSelFilter());

  if (!fileNames.isEmpty())
  {
    QFileInfo info(fileNames.value(0));

    te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_img");

    m_ui->m_imgFilesListWidget->addItems(fileNames);

    remap();
  }
}

void te::urban::qt::RemapClassWidget::onReclassRemoveImageToolButtonClicked()
{
  qDeleteAll(m_ui->m_imgFilesListWidget->selectedItems());
}

void te::urban::qt::RemapClassWidget::onReclassOutputRepoToolButtonClicked()
{
  QString dirName = QFileDialog::getExistingDirectory(this, tr("Select output data location"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_outDir"));

  if (!dirName.isEmpty())
  {
    te::qt::widgets::AddFilePathToSettings(dirName, "urbanAnalysis_outDir");

    m_ui->m_reclassOutputRepoLineEdit->setText(dirName);
  }
}

void te::urban::qt::RemapClassWidget::execute()
{
  //check input parameters
  if (m_ui->m_imgFilesListWidget->count() == 0)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select at least one input image."));
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

  std::string outputPath = m_ui->m_reclassOutputRepoLineEdit->text().toStdString();
  std::string outputSufix = m_ui->m_reclassOutputNameLineEdit->text().toStdString();

  //add task viewer
  te::qt::widgets::ProgressViewerDialog* dlgViewer = new te::qt::widgets::ProgressViewerDialog(this);
  int dlgViewerId = te::common::ProgressManager::getInstance().addViewer(dlgViewer);

  int defaultValue = 0;
  std::map<int, int> mapValues;


  //set map
  int nRows = m_ui->m_remapTableWidget->rowCount();

  for (int i = 0; i < nRows; ++i)
  {
    int pixelValue = m_ui->m_remapTableWidget->item(i, 0)->text().toInt();

    QComboBox* cmbBox = dynamic_cast<QComboBox*>(m_ui->m_remapTableWidget->cellWidget(i, 2));
    std::string inputClass = cmbBox->currentText().toStdString();

    int inputClassValue = 0;

    if (inputClass == "NoData")
    {
      inputClassValue = m_ui->m_noDataClassSpinBox->value();
    }
    else if (inputClass == "Water")
    {
      inputClassValue = m_ui->m_waterClassSpinBox->value();
    }
    else if (inputClass == "Urban")
    {
      inputClassValue = m_ui->m_urbanClassSpinBox->value();
    }
    else if (inputClass == "Other")
    {
      inputClassValue = m_ui->m_otherClassSpinBox->value();
    }

    mapValues[pixelValue] = inputClassValue;
  }

  //execute operation
  for (int i = 0; i < m_ui->m_imgFilesListWidget->count(); ++i)
  {
    QFileInfo file(m_ui->m_imgFilesListWidget->item(i)->text());
    
    std::string outputFileName = file.baseName().toStdString() + "_" + outputSufix + ".tif";
    std::string outputFilePath = outputPath + "/" + outputFileName;

    std::auto_ptr<te::rst::Raster> inputRaster = openRaster(m_ui->m_imgFilesListWidget->item(i)->text().toStdString());

    inputRaster = reclassify(inputRaster.get(), mapValues, defaultValue);

    saveRaster(outputFilePath, inputRaster.get());
  }

  te::common::ProgressManager::getInstance().removeViewer(dlgViewerId);
  delete dlgViewer;

  QMessageBox::information(this, tr("Urban Analysis"), tr("The execution finished with success."));
}

void te::urban::qt::RemapClassWidget::remap()
{
  te::qt::widgets::ScopedCursor c(Qt::WaitCursor);

  std::map<double, unsigned int> values;

  if (m_ui->m_imgFilesListWidget->count() != 0)
  {
    std::string inputFileName = m_ui->m_imgFilesListWidget->item(0)->text().toStdString();
    std::auto_ptr<te::rst::Raster> inputRaster = openRaster(inputFileName);

    values = inputRaster->getBand(0)->getHistogramR();
  }

  m_ui->m_remapTableWidget->setRowCount(0);

  std::map<double, unsigned int>::iterator it;

  for (it = values.begin(); it != values.end(); ++it)
  {
    int newrow = m_ui->m_remapTableWidget->rowCount();
    m_ui->m_remapTableWidget->insertRow(newrow);

    QTableWidgetItem* itemValue = new QTableWidgetItem(QString::number(it->first));
    itemValue->setFlags(Qt::ItemIsEnabled);
    m_ui->m_remapTableWidget->setItem(newrow, 0, itemValue);

    QTableWidgetItem* itemCount = new QTableWidgetItem(QString("%L1").arg(it->second));
    itemCount->setFlags(Qt::ItemIsEnabled);
    m_ui->m_remapTableWidget->setItem(newrow, 1, itemCount);

    QComboBox* cmbBox = new QComboBox(m_ui->m_remapTableWidget);
    cmbBox->addItem("No Data");
    cmbBox->addItem("Other");
    cmbBox->addItem("Water");
    cmbBox->addItem("Urban");

    m_ui->m_remapTableWidget->setCellWidget(newrow, 2, cmbBox);
  }

  m_ui->m_remapTableWidget->resizeColumnToContents(0);
}
