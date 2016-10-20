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

#include "../terralib_mod_growth/SprawlMetrics.h"
#include "../terralib_mod_growth/UrbanGrowth.h"
#include "../terralib_mod_growth/Utils.h"
#include "SprawlMetricsWidget.h"
#include "Utils.h"
#include "ui_SprawlMetricsWidgetForm.h"

//Terralib
#include <terralib/common/StringUtils.h>
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/Utils.h>
#include <terralib/raster/Band.h>
#include <terralib/raster/BandProperty.h>
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

  m_ui->m_reclassRadiusLineEdit->setValidator(new QDoubleValidator(this));

  if (m_startAsPlugin)
  {
    m_ui->m_addImageToolButton->setIcon(QIcon::fromTheme("list-add"));
    m_ui->m_removeImageToolButton->setIcon(QIcon::fromTheme("list-remove"));
    m_ui->m_slopeRasterToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_cbdVecToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_studyAreaVecToolButton->setIcon(QIcon::fromTheme("folder"));
    m_ui->m_exportMetricsInfoToolButton->setIcon(QIcon::fromTheme("document-save"));
    m_ui->m_addThresholdToolButton->setIcon(QIcon::fromTheme("list-add"));
    m_ui->m_removeThresholdToolButton->setIcon(QIcon::fromTheme("list-remove"));
  }
  else
  {
    m_ui->m_addImageToolButton->setIcon(QIcon(":/images/list-add.svg"));
    m_ui->m_removeImageToolButton->setIcon(QIcon(":/images/list-remove.svg"));
    m_ui->m_slopeRasterToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_cbdVecToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_studyAreaVecToolButton->setIcon(QIcon(":/images/folder.svg"));
    m_ui->m_exportMetricsInfoToolButton->setIcon(QIcon(":/images/document-save.svg"));
    m_ui->m_addThresholdToolButton->setIcon(QIcon(":/images/list-add.svg"));
    m_ui->m_removeThresholdToolButton->setIcon(QIcon(":/images/list-remove.svg"));
  }

  //connects
  connect(m_ui->m_addImageToolButton, SIGNAL(clicked()), this, SLOT(onAddImageToolButtonClicked()));
  connect(m_ui->m_removeImageToolButton, SIGNAL(clicked()), this, SLOT(onRemoveImageToolButtonClicked()));
  connect(m_ui->m_slopeRasterToolButton, SIGNAL(clicked()), this, SLOT(onSlopeRasterToolButtonClicked()));
  connect(m_ui->m_cbdVecToolButton, SIGNAL(clicked()), this, SLOT(onCBDVectorialDataToolButtonClicked()));
  connect(m_ui->m_studyAreaVecToolButton, SIGNAL(clicked()), this, SLOT(onStudyAreaVectorialDataToolButtonClicked()));
  connect(m_ui->m_exportMetricsInfoToolButton, SIGNAL(clicked()), this, SLOT(onExportMetricsInfoToolButton()));
  connect(m_ui->m_addThresholdToolButton, SIGNAL(clicked()), this, SLOT(onAddThresholdToolButtonClicked()));
  connect(m_ui->m_removeThresholdToolButton, SIGNAL(clicked()), this, SLOT(onRemoveThresholdToolButtonClicked()));
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

void te::urban::qt::SprawlMetricsWidget::onExportMetricsInfoToolButton()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Set Metrics Text File"), te::qt::widgets::GetFilePathFromSettings("urbanAnalysis_txt"), tr("Text File (*.txt *.TXT)"));

  if (fileName.isEmpty())
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Metrics file not defined."));
    return;
  }

  QFileInfo info(fileName);

  te::qt::widgets::AddFilePathToSettings(info.absolutePath(), "urbanAnalysis_txt");

  std::string logFile = fileName.toUtf8().constData();

  //check index table
  if (m_ui->m_metricsTableWidget->rowCount() == 0)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Metrics not calculated."));
    return;
  }

  //create output file
  FILE* indexFile;
  indexFile = fopen(logFile.c_str(), "a+"); // a+ (create + append)

  if (indexFile == NULL)
  {
    QMessageBox::warning(this, tr("Urban Analysis"), tr("Error creating Metrics File."));
    return;
  }

  fprintf(indexFile, "\t\tUrban Analysis - METRICS LOG INFORMATION \n\n");

  for (int i = 0; i < m_ui->m_metricsTableWidget->rowCount(); ++i)
  {
    std::string msg;
    for (int j = 0; j < m_ui->m_metricsTableWidget->columnCount(); ++j)
    {
      std::string header = m_ui->m_metricsTableWidget->horizontalHeaderItem(j)->text().toStdString();
      std::string value = m_ui->m_metricsTableWidget->item(i, j)->text().toStdString();

      msg += header + ": " + value;
      msg += "\t\t";
    }
    msg += "\n";
    fprintf(indexFile, msg.c_str());
  }

  fprintf(indexFile, "-------------------------------------------------------------------");

  fclose(indexFile);
}

void te::urban::qt::SprawlMetricsWidget::onAddThresholdToolButtonClicked()
{
  int from = m_ui->m_fromSpinBox->value();
  int to = m_ui->m_toSpinBox->value();

  if (from == to || to < from)
  {
    QMessageBox::information(this, tr("Urban Analysis"), tr("Invalid threshold values."));
    return;
  }

  QString value;
  value.append(QString::number(from));
  value.append(" / ");
  value.append(QString::number(to));

  m_ui->m_thresholdListWidget->addItem(value);
}

void te::urban::qt::SprawlMetricsWidget::onRemoveThresholdToolButtonClicked()
{
  qDeleteAll(m_ui->m_thresholdListWidget->selectedItems());
}

void te::urban::qt::SprawlMetricsWidget::execute()
{
  //temporary initialization
  /*
  m_ui->m_imgFilesListWidget->addItem("D:/temp/miguel_fred/sao_paulo/entrada/Class_SP_95_FIM__ready.tif");
  m_ui->m_slopeLineEdit->setText("D:/temp/miguel_fred/sao_paulo/entrada/mosaico_sp_slope_rounded.tif");
  m_ui->m_cbdVecLineEdit->setText("D:/temp/miguel_fred/sao_paulo/entrada/area_estudo_sp.shp");
  m_ui->m_studyAreaVecLineEdit->setText("D:/temp/miguel_fred/sao_paulo/entrada/area_estudo_sp.shp");
  */
  std::vector< std::pair<int, int> > vecSlopeThresholds;

  //get radius value
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
 
  //check input 
  bool calculateProximityIndex = m_ui->m_proximityCheckBox->isChecked();
  bool calculateCohesionIndex = m_ui->m_cohesionCheckBox->isChecked();
  bool calculateDepthIndex = m_ui->m_depthCheckBox->isChecked();

  std::auto_ptr<te::rst::Raster> slopeRaster;
  std::auto_ptr<te::gm::Geometry> studyArea;
  te::gm::Point cbdCentroid;

  if (calculateProximityIndex)
  {
    if (m_ui->m_slopeLineEdit->text().isEmpty())
    {
      QMessageBox::information(this, tr("Urban Analysis"), tr("To calculate proximity index, a slope raster must be given."));
      return;
    }

    if (m_ui->m_cbdVecLineEdit->text().isEmpty())
    {
      QMessageBox::information(this, tr("Urban Analysis"), tr("To calculate proximity index, a CBD vector file must be given."));
      return;
    }

    if (m_ui->m_thresholdListWidget->count() == 0)
    {
      QMessageBox::information(this, tr("Urban Analysis"), tr("To calculate proximity index, you must enter with at least one threshold."));
      return;
    }

    QString qSlopeFileName = m_ui->m_slopeLineEdit->text();
    std::string slopeFileName = qSlopeFileName.toStdString();
    slopeRaster = openRaster(slopeFileName);

    QString qCbdFileName = m_ui->m_cbdVecLineEdit->text();
    std::string cbdFileName = qCbdFileName.toStdString();
    std::auto_ptr<te::da::DataSet> cbdDataSet = openVector(cbdFileName);
    std::auto_ptr<te::gm::Geometry> cbdGeometry = dissolveDataSet(cbdDataSet.get());

    te::gm::Coord2D centroidCoord = te::gm::GetCentroid(cbdGeometry.get());
    cbdCentroid = te::gm::Point(centroidCoord.x, centroidCoord.y, cbdGeometry->getSRID());

    for (int i = 0; i < m_ui->m_thresholdListWidget->count(); ++i)
    {
      QString qText = m_ui->m_thresholdListWidget->item(i)->text();
      QStringList qStringList = qText.split("/");
      QString initThreshold = qStringList.at(0).trimmed();
      QString endThreshold = qStringList.at(1).trimmed();
      int init = initThreshold.toInt();
      int end = endThreshold.toInt();
      std::pair<int, int> pairThreshold(init, end);

      vecSlopeThresholds.push_back(pairThreshold);
    }
  }
  if (calculateDepthIndex || calculateProximityIndex)
  {
    if (m_ui->m_studyAreaVecLineEdit->text().isEmpty())
    {
      QMessageBox::information(this, tr("Urban Analysis"), tr("To calculate proximity or cohesion indexes, a study area vector file be given."));
      return;
    }

    QString qStudyAreaFileName = m_ui->m_studyAreaVecLineEdit->text();
    std::string studyAreaFileName = qStudyAreaFileName.toStdString();
    std::auto_ptr<te::da::DataSet> studyAreaDataSet = openVector(studyAreaFileName);
    studyArea = dissolveDataSet(studyAreaDataSet.get());
  }

  InputClassesMap inputClassesMap;
  inputClassesMap[INPUT_NODATA] = INPUT_NODATA;
  inputClassesMap[INPUT_WATER] = INPUT_WATER;
  inputClassesMap[INPUT_URBAN] = INPUT_URBAN;
  inputClassesMap[INPUT_OTHER] = INPUT_OTHER;

  //add task viewer
  te::qt::widgets::ProgressViewerDialog* dlgViewer = new te::qt::widgets::ProgressViewerDialog(this);
  int dlgViewerId = te::common::ProgressManager::getInstance().addViewer(dlgViewer);

  UrbanSummary urbanSummary;
  try
  {
    for (int i = 0; i < m_ui->m_imgFilesListWidget->count(); ++i)
    {
      QString qLandCoverFileName(m_ui->m_imgFilesListWidget->item(i)->text());
      QFileInfo qLandCoverFileInfo(qLandCoverFileName);
      QString qBaseName = qLandCoverFileInfo.baseName();

      std::string landCoverFileName = qLandCoverFileName.toStdString();
      std::string baseName = qBaseName.toStdString();
      std::auto_ptr<te::rst::Raster> landCoverRaster = openRaster(landCoverFileName);

      if (i == 0)
      {
        if (slopeRaster.get() != 0 && needNormalization(slopeRaster.get(), landCoverRaster.get()))
        {
          slopeRaster = normalizeRaster(slopeRaster.get(), landCoverRaster.get());
        }
        if (cbdCentroid.getSRID() != landCoverRaster->getSRID())
        {
          cbdCentroid.transform(landCoverRaster->getSRID());
        }
        if (studyArea.get() != 0)
        {
          if (studyArea->getSRID() != landCoverRaster->getSRID())
          {
            studyArea->transform(landCoverRaster->getSRID());
          }          

          //here we clip the limit using the box of the raster          
          std::auto_ptr<te::gm::Geometry> clipArea(te::gm::GetGeomFromEnvelope(landCoverRaster->getExtent(), landCoverRaster->getSRID()));
          studyArea.reset(studyArea->intersection(clipArea.get()));
          studyArea->setSRID(landCoverRaster->getSRID());
        }
      }

      PrepareRasterParams prepareRasterParams;
      prepareRasterParams.m_inputRaster = landCoverRaster.get();
      prepareRasterParams.m_inputClassesMap = inputClassesMap;
      prepareRasterParams.m_radius = radius;
      prepareRasterParams.m_saveIntermediateFiles = false;

      prepareRaster(&prepareRasterParams);

      //we calculate the indexes for the urbanized area image
      {
        IndexesParams params;
        params.m_urbanRaster = prepareRasterParams.m_result.m_urbanizedAreaRaster.get();
        params.m_calculateCohesion = calculateCohesionIndex;
        params.m_calculateDepth = calculateDepthIndex;
        params.m_slopeRaster = slopeRaster.get();
        params.m_studyArea = studyArea.get();
        params.m_landCoverRaster = landCoverRaster.get();
        params.m_centroidCBD = cbdCentroid;
        params.m_vecSlopeThresholds = vecSlopeThresholds;

        UrbanIndexes mapIndexes = calculateIndexes(params);
        urbanSummary[baseName + "_urbanized_area"] = mapIndexes;
      }

      //and then we calculate the indexes for the urban footprint image
      {
        IndexesParams params;
        params.m_urbanRaster = prepareRasterParams.m_result.m_urbanFootprintRaster.get();
        params.m_calculateCohesion = calculateCohesionIndex;
        params.m_calculateDepth = calculateDepthIndex;
        params.m_slopeRaster = slopeRaster.get();
        params.m_studyArea = studyArea.get();
        params.m_landCoverRaster = landCoverRaster.get();
        params.m_centroidCBD = cbdCentroid;
        params.m_vecSlopeThresholds = vecSlopeThresholds;

        UrbanIndexes mapIndexes = calculateIndexes(params);
        urbanSummary[baseName + "_urban_footprint"] = mapIndexes;
      }
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

  //we show the calculated metrics
  QStringList qStringList;
  qStringList.append(tr("FileName"));

  UrbanSummary::iterator itSummary = urbanSummary.begin();
  while (itSummary != urbanSummary.end())
  {
    UrbanIndexes urbanIndexes = itSummary->second;

    UrbanIndexes::iterator itIndexes = urbanIndexes.begin();
    while (itIndexes != urbanIndexes.end())
    {
      std::string fullColumnName = itIndexes->first;
      std::vector<std::string> vecTokens;
      te::common::Tokenize(fullColumnName, vecTokens, ".");

      qStringList.append(QString::fromStdString(vecTokens[1]));
      ++itIndexes;
    }

    //we just want the the columns names at this point. So we can break the loop
    break;
  }

  m_ui->m_metricsTableWidget->setColumnCount(qStringList.size());
  m_ui->m_metricsTableWidget->setHorizontalHeaderLabels(qStringList);
  m_ui->m_metricsTableWidget->setRowCount(0);

  itSummary = urbanSummary.begin();
  while (itSummary != urbanSummary.end())
  {
    UrbanIndexes urbanIndexes = itSummary->second;
    std::string imageBaseName = itSummary->first;

    //we add a new row to the output table
    int newrow = m_ui->m_metricsTableWidget->rowCount();
    m_ui->m_metricsTableWidget->setRowCount(newrow + 1);
    QTableWidgetItem* itemName = new QTableWidgetItem(QString::fromStdString(imageBaseName));
    itemName->setFlags(Qt::ItemIsEnabled);
    m_ui->m_metricsTableWidget->setItem(newrow, 0, itemName);

    int newcolumn = 1;
    UrbanIndexes::iterator itIndexes = urbanIndexes.begin();
    while (itIndexes != urbanIndexes.end())
    {
      QTableWidgetItem* indexValueItem = new QTableWidgetItem(QString::number(itIndexes->second));
      indexValueItem->setFlags(Qt::ItemIsEnabled);
      m_ui->m_metricsTableWidget->setItem(newrow, newcolumn, indexValueItem);

      ++newcolumn;
      ++itIndexes;
    }

    ++itSummary;
  }

  m_ui->m_metricsTableWidget->resizeColumnsToContents();

  QMessageBox::information(this, tr("Urban Analysis"), tr("The execution finished with success."));
}
