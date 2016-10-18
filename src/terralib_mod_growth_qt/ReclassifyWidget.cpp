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
#include "Utils.h"
#include "ui_ReclassifyWidgetForm.h"

#include "../terralib_mod_growth/UrbanGrowth.h"

//Terralib
#include <terralib/common/STLUtils.h>
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

te::urban::qt::ReclassifyWidget::ReclassifyWidget(bool startAsPlugin, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_ui(new Ui::ReclassifyWidgetForm),
  m_startAsPlugin(startAsPlugin)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_reclassRadiusLineEdit->setValidator(new QDoubleValidator(this));

  if (m_startAsPlugin)
  {
    m_ui->m_reclassAddImageToolButton->setIcon(QIcon::fromTheme("list-add"));
    m_ui->m_reclassRemoveImageToolButton->setIcon(QIcon::fromTheme("list-remove"));
    m_ui->m_reclassAddVecToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_reclassOutputRepoToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_exportIndexInfoToolButton->setIcon(QIcon::fromTheme("document-save"));
  }
  else
  {
    m_ui->m_reclassAddImageToolButton->setIcon(QIcon(":/images/list-add.svg"));
    m_ui->m_reclassRemoveImageToolButton->setIcon(QIcon(":/images/list-remove.svg"));
    m_ui->m_reclassAddVecToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_reclassOutputRepoToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_exportIndexInfoToolButton->setIcon(QIcon(":/images/document-save.svg"));
  }

  //connects
  connect(m_ui->m_reclassAddImageToolButton, SIGNAL(clicked()), this, SLOT(onReclassAddImageToolButtonClicked()));
  connect(m_ui->m_reclassRemoveImageToolButton, SIGNAL(clicked()), this, SLOT(onReclassRemoveImageToolButtonClicked()));
  connect(m_ui->m_reclassAddVecToolButton, SIGNAL(clicked()), this, SLOT(onReclassAddVecToolButtonClicked()));
  connect(m_ui->m_reclassOutputRepoToolButton, SIGNAL(clicked()), this, SLOT(onReclassOutputRepoToolButtonClicked()));
  connect(m_ui->m_exportIndexInfoToolButton, SIGNAL(clicked()), this, SLOT(onExportIndexInfoToolButton()));
  connect(m_ui->m_remapCheckBox, SIGNAL(clicked(bool)), this, SLOT(onRemapCheckBoxClicked(bool)));
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

void te::urban::qt::ReclassifyWidget::onRemapCheckBoxClicked(bool flag)
{
  if (!m_ui->m_remapCheckBox->isChecked())
    return;

  m_ui->m_tabWidget->setCurrentIndex(1);

  std::map<double, unsigned int> values;

  if (m_ui->m_imgFilesListWidget->count() == 0)
  {
	  return;
  }

  std::string inputFileName = m_ui->m_imgFilesListWidget->item(0)->text().toStdString();
  
  try
  {
    std::auto_ptr<te::rst::Raster> inputRaster = openRaster(inputFileName);

    values = inputRaster->getBand(0)->getHistogramR();
  }
  catch (const std::exception& e)
  {
    QString message = tr("Error in the execution.");
    if (e.what() != 0)
    {
      message += QString("\n");
      message += QString(e.what());
    }

    QMessageBox::information(this, tr("Urban Analysis"), message);

    return;
  }

  te::qt::widgets::ScopedCursor c(Qt::WaitCursor);
  
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
    cmbBox->addItem("No Data", INPUT_NODATA);
    cmbBox->addItem("Other", INPUT_OTHER);
    cmbBox->addItem("Water", INPUT_WATER);
    cmbBox->addItem("Urban", INPUT_URBAN);

    m_ui->m_remapTableWidget->setCellWidget(newrow, 2, cmbBox);
  }

  m_ui->m_remapTableWidget->resizeColumnToContents(0);
}

void te::urban::qt::ReclassifyWidget::onExportIndexInfoToolButton()
{
  //check output location
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
  std::string outputPrefix = m_ui->m_reclassOutputNameLineEdit->text().toStdString();
  std::string logFile = outputPath + "/" + outputPrefix + "_indexes.txt";

  //check index table
  if (m_ui->m_indexTableWidget->rowCount() == 0)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Indexes not calculated."));
    return;
  }

  //create output file
  FILE* indexFile;
  indexFile = fopen(logFile.c_str(), "a+"); // a+ (create + append)

  if (indexFile == NULL)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Error creating Index File."));
    return;
  }
  
  fprintf(indexFile, "\t\tUrban Analysis - INDEX LOG INFORMATION \n\n");

  for (int i = 0; i < m_ui->m_indexTableWidget->rowCount(); ++i)
  {
    std::string msg  = "Image: " + m_ui->m_indexTableWidget->item(i, 0)->text().toStdString();
                msg += "\t\t Opennes: " + m_ui->m_indexTableWidget->item(i, 1)->text().toStdString();
                msg += "\t\t Edge: " + m_ui->m_indexTableWidget->item(i, 2)->text().toStdString();
                msg += "\n";

    fprintf(indexFile, msg.c_str());
  }

  fprintf(indexFile, "-------------------------------------------------------------------");

  fclose(indexFile);
}

void te::urban::qt::ReclassifyWidget::execute()
{
  //check input parameters
  if (m_ui->m_imgFilesListWidget->count() == 0)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Select at least one input image."));
    return;
  }

  if (m_ui->m_reclassRadiusLineEdit->text().isEmpty() == true)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Radius value not defined."));
    return;
  }

  bool converted = false;
  double radius = m_ui->m_reclassRadiusLineEdit->text().toDouble(&converted);

  if (converted == false)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Invalid radius value."));
    return;
  }

  std::string spatialLimitsFileName;

  if(m_ui->m_vecFileLineEdit->text().isEmpty() == false)
  {
    spatialLimitsFileName = m_ui->m_vecFileLineEdit->text().toStdString();
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

  bool calculateIndexes = m_ui->m_indexCheckBox->isChecked();

  QString qOutputIntermediatePath = m_ui->m_reclassOutputRepoLineEdit->text() + "/intermediate";
  QDir qDir(qOutputIntermediatePath);
  if (qDir.exists() == false)
  {
    qDir.mkpath(qOutputIntermediatePath);
  }

  std::string outputPath = m_ui->m_reclassOutputRepoLineEdit->text().toStdString();
  std::string outputIntermediatePath = qOutputIntermediatePath.toStdString();
  std::string outputPrefix = m_ui->m_reclassOutputNameLineEdit->text().toStdString();

  //add task viewer
  te::qt::widgets::ProgressViewerDialog* dlgViewer = new te::qt::widgets::ProgressViewerDialog(this);
  int dlgViewerId = te::common::ProgressManager::getInstance().addViewer(dlgViewer);

  std::vector<ReclassifyInfo> vecReclassifyInfo;

  if (m_ui->m_remapCheckBox->isChecked())
  {
    int nRows = m_ui->m_remapTableWidget->rowCount();

    for (int i = 0; i < nRows; ++i)
    {
      int currentPixelValue = m_ui->m_remapTableWidget->item(i, 0)->text().toInt();

      QComboBox* cmbBox = dynamic_cast<QComboBox*>(m_ui->m_remapTableWidget->cellWidget(i, 2));
      int newPixelValue = cmbBox->currentData().toInt();

      vecReclassifyInfo.push_back(ReclassifyInfo(currentPixelValue, newPixelValue));
    }
  }

  InputClassesMap inputClassesMap;
  inputClassesMap[INPUT_NODATA] = INPUT_NODATA;
  inputClassesMap[INPUT_WATER] = INPUT_WATER;
  inputClassesMap[INPUT_URBAN] = INPUT_URBAN;
  inputClassesMap[INPUT_OTHER] = INPUT_OTHER;

  //in the preprocessing step, we need to read the given ShapeFile that represents the limits of the analysed regeion.
  //then dissolve all the polygons from the ShapeFile
  std::auto_ptr<te::gm::Geometry> geometryLimit;
  if (spatialLimitsFileName.empty() == false)
  {
    try
    {
      std::auto_ptr<te::da::DataSet> dataSet = openVector(spatialLimitsFileName);
      geometryLimit = dissolveDataSet(dataSet.get());
    }
    catch (const std::exception& e)
    {
      QString message = tr("Error in the execution.");
      if (e.what() != 0)
      {
        message += QString("\n");
        message += QString(e.what());
      }
      return;
    }
  }

  //execute operation
  std::vector<te::rst::Raster*> vecInputRasters;
  std::vector<PrepareRasterParams*> vecPreparedRasters;
  std::vector<CalculateUrbanIndexesParams*> vecCalculatedIndexes;

  std::string logFileName = outputPath + "\\log\\UrbanAnalysis_" + outputPrefix + ".log";
  initLogger(logFileName);

  logInfo("Starting process");

  boost::thread_group threadGroup;

  Timer timer;

  //just a reference to be used in the raster normalization optional step
  te::rst::Raster* referenceRaster = 0;

  //we first prepared all the rasters
  try
  {
    for (int i = 0; i < m_ui->m_imgFilesListWidget->count(); ++i)
    {
      std::string inputFileName = m_ui->m_imgFilesListWidget->item(i)->text().toStdString();
      std::string currentOutputPrefix = outputPrefix + "_t" + boost::lexical_cast<std::string>(i);

      std::auto_ptr<te::rst::Raster> inputRaster = openRaster(inputFileName);

      //we reclassify the raster if necessary
      if (m_ui->m_remapCheckBox->isChecked())
      {
        inputRaster = reclassify(inputRaster.get(), vecReclassifyInfo, SET_NEW_NODATA, 0);
      }

      //we normalize the raster if necessary
      if (i == 0)
      {
        referenceRaster = inputRaster.get();
        
        if (geometryLimit.get() != 0 && geometryLimit->getSRID() != inputRaster->getSRID())
        {
          geometryLimit->transform(inputRaster->getSRID());
        }
      }
      else
      {
        if (needNormalization(inputRaster.get(), referenceRaster))
        {
          inputRaster = normalizeRaster(inputRaster.get(), referenceRaster);

          std::string normalizedPrefix = currentOutputPrefix + "_normalized";
          std::string normalizedRasterFileName = outputIntermediatePath + "/" + normalizedPrefix + ".tif";
          saveRaster(normalizedRasterFileName, inputRaster.get());
        }
      }

      //we create a thread to process the prepare raster step
      PrepareRasterParams* prepareRasterParams = new PrepareRasterParams();
      prepareRasterParams->m_inputRaster = inputRaster.get();
      prepareRasterParams->m_inputClassesMap = inputClassesMap;
      prepareRasterParams->m_radius = radius;
      prepareRasterParams->m_outputPath = outputIntermediatePath;
      prepareRasterParams->m_outputPrefix = currentOutputPrefix;

      boost::thread* prepareRasterThread = new boost::thread(&prepareRaster, prepareRasterParams);
      threadGroup.add_thread(prepareRasterThread);

      vecPreparedRasters.push_back(prepareRasterParams);

      //then we create a thread to calculate the indexes
      std::auto_ptr<CalculateUrbanIndexesParams> urbanIndexesParams;
      if (calculateIndexes)
      {
        CalculateUrbanIndexesParams* urbanIndexesParams = new CalculateUrbanIndexesParams();
        urbanIndexesParams->m_inputFileName = inputFileName;
        urbanIndexesParams->m_inputRaster = inputRaster.get();
        urbanIndexesParams->m_inputClassesMap = inputClassesMap;
        urbanIndexesParams->m_radius = radius;
        urbanIndexesParams->m_spatialLimits = geometryLimit.get();

        threadGroup.add_thread(new boost::thread(&calculateUrbanIndexes, urbanIndexesParams));

        vecCalculatedIndexes.push_back(urbanIndexesParams);
      }

      vecInputRasters.push_back(inputRaster.release());
    }
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

    logError(message.toStdString());
    logInfo("Process finished with Error in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
    removeAllLoggers();

    QMessageBox::information(this, tr("Urban Analysis"),  message);
    return;
  }

  threadGroup.join_all();

  UrbanSummary urbanSummary;
  for (std::size_t i = 0; i < vecCalculatedIndexes.size(); ++i)
  {
    urbanSummary[vecCalculatedIndexes[i]->m_inputFileName] = vecCalculatedIndexes[i]->m_urbanIndexes;
  }

  te::common::FreeContents(vecInputRasters);
  te::common::FreeContents(vecCalculatedIndexes);

  //we we compare the time periods
  try
  {
    boost::thread_group compareItmePeriodsThreadGroup;
    std::vector<CompareTimePeriodsParams*> vecCompareTimePeriodsParams;

    for (std::size_t i = 1; i < vecPreparedRasters.size(); ++i)
    {
      std::string inputFileName = m_ui->m_imgFilesListWidget->item((int)i)->text().toStdString();
      std::string currentOutputPrefix = outputPrefix + "_t" + boost::lexical_cast<std::string>(i - 1) + "_t" + boost::lexical_cast<std::string>(i);

      CompareTimePeriodsParams* params = new CompareTimePeriodsParams();
      params->m_t1 = vecPreparedRasters[i - 1]->m_result;
      params->m_t2 = vecPreparedRasters[i]->m_result;
      params->m_outputPath = outputIntermediatePath;
      params->m_outputPrefix = currentOutputPrefix;

      compareItmePeriodsThreadGroup.add_thread(new boost::thread(&compareRasterPeriods, params));

      vecCompareTimePeriodsParams.push_back(params);
    }

    compareItmePeriodsThreadGroup.join_all();

    for (std::size_t i = 0; i < vecCompareTimePeriodsParams.size(); ++i)
    {
      CompareTimePeriodsParams* params = vecCompareTimePeriodsParams[i];

      const std::string& currentOutputPrefix = params->m_outputPrefix;

      std::string newDevelopmentPrefix = currentOutputPrefix + "_newDevelopment";
      std::string newDevelopmentRasterFileName = outputPath + "/" + newDevelopmentPrefix + ".tif";
      saveRaster(newDevelopmentRasterFileName, params->m_outputRaster.get());

      if (m_startAsPlugin)
      {
        te::map::AbstractLayerPtr layer = te::urban::qt::CreateLayer(newDevelopmentRasterFileName, "GDAL");

        emit layerCreated(layer);
      }
    }

    te::common::FreeContents(vecPreparedRasters);
    te::common::FreeContents(vecCompareTimePeriodsParams);
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

    logError(message.toStdString());
    logInfo("Process finished with Error in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
    removeAllLoggers();

    QMessageBox::information(this, tr("Urban Analysis"), message);
    return;
  }

  logInfo("Process finished with success in " + boost::lexical_cast<std::string>(timer.getElapsedTimeMinutes()) + " minutes");
  removeAllLoggers();

  if (calculateIndexes)
  {
    m_ui->m_indexTableWidget->setRowCount(0);

    UrbanSummary::iterator itSummary = urbanSummary.begin();

    while (itSummary != urbanSummary.end())
    {
      UrbanIndexes ui = itSummary->second;

      std::string imageName = itSummary->first;
      double opennes = ui["openness"];
      double edge = ui["edgeIndex"];

      int newrow = m_ui->m_indexTableWidget->rowCount();
      m_ui->m_indexTableWidget->insertRow(newrow);

      QTableWidgetItem* itemName = new QTableWidgetItem(QString::fromStdString(imageName));
      itemName->setFlags(Qt::ItemIsEnabled);
      m_ui->m_indexTableWidget->setItem(newrow, 0, itemName);

      QTableWidgetItem* itemOpennes = new QTableWidgetItem(QString::number(opennes));
      itemName->setFlags(Qt::ItemIsEnabled);
      m_ui->m_indexTableWidget->setItem(newrow, 1, itemOpennes);

      QTableWidgetItem* itemEdge = new QTableWidgetItem(QString::number(edge));
      itemName->setFlags(Qt::ItemIsEnabled);
      m_ui->m_indexTableWidget->setItem(newrow, 2, itemEdge);

      ++itSummary;
    }

    m_ui->m_indexTableWidget->resizeColumnsToContents();
  }

  m_ui->m_tabWidget->setCurrentIndex(0);

  te::common::ProgressManager::getInstance().removeViewer(dlgViewerId);
  delete dlgViewer;

  QMessageBox::information(this, tr("Urban Analysis"), tr("The execution finished with success."));
}