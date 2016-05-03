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
\file urban_analysis/src/growth/qt/UrbanAnalysisMainDialog.cpp

\brief This class represents the Urban Analysis Main Dialog 
*/

#include "UrbanAnalysisMainDialog.h"
#include "ui_UrbanAnalysisMainDialogForm.h"

#include "ReclassifyWidget.h"

#include "terralib_mod_growth/Utils.h"

//Qt
#include <QSpacerItem>

te::urban::qt::UrbanAnalysisMainDialog::UrbanAnalysisMainDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
  m_ui(new Ui::UrbanAnalysisMainDialogForm),
  m_currentWidget(0),
  m_opButtonPos(0)
{
  // add controls
  m_ui->setupUi(this);

  //set layout
  m_layout = new QGridLayout(m_ui->m_widget);
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_ui->m_widget->setLayout(m_layout);

  //add buttons and widgets for each urban analysis operations
  buildDialog();

  te::urban::init();
}

te::urban::qt::UrbanAnalysisMainDialog::~UrbanAnalysisMainDialog()
{
  te::urban::finalize();

  if (m_currentWidget)
    delete m_currentWidget;
}

void te::urban::qt::UrbanAnalysisMainDialog::buildDialog()
{
  //reclassify
  QToolButton* reclassToolButton = createButton("Reclassify");
  connect(reclassToolButton, SIGNAL(clicked()), this, SLOT(onReclassifyToolButtonClicked()));

  //sprawl metrics
  QToolButton* sprawlMetricsToolButton = createButton("Sprawl Metrics");
  connect(sprawlMetricsToolButton, SIGNAL(clicked()), this, SLOT(onSprawlMetricsToolButtonClicked()));
  
  //leave this for last
  createSpacer();
}

QToolButton* te::urban::qt::UrbanAnalysisMainDialog::createButton(std::string name)
{
  QToolButton* button = new QToolButton(m_ui->m_frame);
  button->setText(name.c_str());
  button->setEnabled(true);
  button->setCheckable(true);
  button->setAutoExclusive(true);
  button->setAutoRaise(true);
  button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

  m_ui->m_gridLayout->addWidget(button, m_opButtonPos, 0, 1, 1);

  ++m_opButtonPos;

  return button;
}

void te::urban::qt::UrbanAnalysisMainDialog::createSpacer()
{
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

  m_ui->m_gridLayout->addItem(spacer, m_opButtonPos, 0, 1, 1);
}

void te::urban::qt::UrbanAnalysisMainDialog::onReclassifyToolButtonClicked()
{
  QToolButton* button = dynamic_cast<QToolButton*>(QObject::sender());

  if (button)
    button->setChecked(true);

  if (m_currentWidget)
    delete m_currentWidget;

  te::urban::qt::ReclassifyWidget* widget = new te::urban::qt::ReclassifyWidget(m_ui->m_widget);

  m_layout->addWidget(widget);

  widget->show();

  connect(m_ui->m_okPushButton, SIGNAL(clicked()), widget, SLOT(execute()));

  m_currentWidget = widget;
}

void te::urban::qt::UrbanAnalysisMainDialog::onSprawlMetricsToolButtonClicked()
{
  QToolButton* button = dynamic_cast<QToolButton*>(QObject::sender());

  if (button)
    button->setChecked(true);

  if (m_currentWidget)
    delete m_currentWidget;

  m_currentWidget = 0;
}
