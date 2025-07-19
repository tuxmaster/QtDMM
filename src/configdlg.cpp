//======================================================================
// File:		configdlg.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 14:54:29 CEST 2002
//----------------------------------------------------------------------
// This file is part of QtDMM.
//
// QtDMM is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// QtDMM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------
// Copyright (c) 2002 Matthias Toussaint
//======================================================================


#include <QtWidgets>
#include <QtPrintSupport>

#include "configdlg.h"
#include "configitem.h"
#include "dmmprefs.h"
#include "executeprefs.h"
#include "graphprefs.h"
#include "guiprefs.h"
#include "integrationprefs.h"
#include "recorderprefs.h"
#include "scaleprefs.h"
#include "settings.h"

#include <iostream>


ConfigDlg::ConfigDlg(QWidget *parent) :  QDialog(parent)
{
  m_buttonBox_OK = false;
  setupUi(this);

  QString path = QString("%1/.qtdmmrc").arg(QDir::homePath());

  m_settings = new Settings(this, path);

  // Check if configuration file exists. If not welcome user

  if (!m_settings->fileExists())
  {
    QMessageBox welcome;
    welcome.setWindowTitle(tr("QtDMM: Welcome!"));
    welcome.setText(tr("<font size=+2><b>Welcome!</b></font><p>"
                       "This seems to be your first invocation of QtDMM "
                       "(Or you have deleted its configuration file).<p>QtDMM "
                       "has created the file %1 in your home directory "
                       "to save its settings.").arg(m_settings->fileName()));
    welcome.setIcon(QMessageBox::Information);
    welcome.setStandardButtons(QMessageBox::Yes);
    welcome.setDefaultButton(QMessageBox::Yes);
    welcome.setIconPixmap(QPixmap(":/Symbols/icon.xpm"));

    QAbstractButton *yesButton = welcome.button(QMessageBox::Yes);
    if (yesButton)
      yesButton->setText(tr("Continue"));

    welcome.exec();
  }
  else
  {
    int version = m_settings->getInt("QtDMM/version");
    int revision = m_settings->getInt("QtDMM/revision");

    if ((version <= 0 && revision < 84) || version >= 7)
    {
      QMessageBox welcome;
      welcome.setWindowTitle(tr("QtDMM: Welcome!"));
      welcome.setText(tr("<font size=+2><b>Welcome!</b></font><p>"
                         "You seem to have upgraded <b>QtDMM</b> from a version prior to 0.8.4. "
                         "Please check your configuration. There are some new parameters to be "
                         "configured."
                         "<p>Thank you for choosing <b>QtDMM</b>.<p><i>Matthias Toussaint</i>"));
      welcome.setIcon(QMessageBox::Information);
      welcome.setStandardButtons(QMessageBox::Yes);
      welcome.setDefaultButton(QMessageBox::Yes);
      welcome.setIconPixmap(QPixmap(":/Symbols/icon.xpm"));

      QAbstractButton *yesButton = welcome.button(QMessageBox::Yes);
      if (yesButton)
        yesButton->setText(tr("Continue"));

      welcome.exec();
    }

    if (m_settings->fileConverted())
    {
      QMessageBox::information(nullptr,
                               tr("QtDMM: Welcome!"),
                               tr("Your config file has been converted to the new format.\n"
                                  "Please check your color settings, because they couldn't be converted automatically.\n"
                                  "Your old config ~/.qtdmmrc was renamed to ~/.qtdmmrc.old."));
    }
  }

  // CREATE PAGES (Top page last)

  m_recorder = new RecorderPrefs(ui_stack);
  m_recorder->setId(ConfigDlg::Recorder);
  new ConfigItem(m_recorder->id(),
                 m_recorder->pixmap(),
                 m_recorder->label(),
                 ui_list);
  m_recorder->setCfg(m_settings);
  ui_stack->insertWidget(m_recorder->id(), m_recorder);

  m_scale = new ScalePrefs(ui_stack);
  m_scale->setId(ConfigDlg::Scale);
  new ConfigItem(m_scale->id(),
                 m_scale->pixmap(),
                 m_scale->label(),
                 ui_list);
  m_scale->setCfg(m_settings);
  ui_stack->insertWidget(m_scale->id(), m_scale);

  m_dmm = new DmmPrefs(ui_stack);
  m_dmm->setId(ConfigDlg::DMM);
  new ConfigItem(m_dmm->id(),
                 m_dmm->pixmap(),
                 m_dmm->label(),
                 ui_list);
  m_dmm->setCfg(m_settings);
  ui_stack->insertWidget(m_dmm->id(), m_dmm);

  m_gui = new GuiPrefs(ui_stack);
  m_gui->setId(ConfigDlg::GUI);
  new ConfigItem(m_gui->id(),
                 m_gui->pixmap(),
                 m_gui->label(),
                 ui_list);
  m_gui->setCfg(m_settings);
  ui_stack->insertWidget(m_gui->id(), m_gui);

  m_graph = new GraphPrefs(ui_stack);
  m_graph->setId(ConfigDlg::Graph);
  new ConfigItem(m_graph->id(),
                 m_graph->pixmap(),
                 m_graph->label(),
                 ui_list);
  m_graph->setCfg(m_settings);
  ui_stack->insertWidget(m_graph->id(), m_graph);

  m_integration = new IntegrationPrefs(ui_stack);
  m_integration->setId(ConfigDlg::Integration);
  new ConfigItem(m_integration->id(),
                 m_integration->pixmap(),
                 m_integration->label(),
                 ui_list);
  m_integration->setCfg(m_settings);
  ui_stack->insertWidget(m_integration->id(), m_integration);

  m_execute = new ExecutePrefs(ui_stack);
  m_execute->setId(ConfigDlg::External);
  new ConfigItem(m_execute->id(),
                 m_execute->pixmap(),
                 m_execute->label(),
                 ui_list);
  m_execute->setCfg(m_settings);
  ui_stack->insertWidget(m_execute->id(), m_execute);


  // init stuff
  //
  on_ui_buttonBox_rejected();
  showPage(static_cast<ConfigDlg::PageType>(m_settings->getInt("Config/LastItem", DMM)));
  ui_undo->hide();
  adjustSize();
}

QString ConfigDlg::deviceListText() const
{
  return m_dmm->deviceListText();
}

void ConfigDlg::showPage(ConfigDlg::PageType page)
{
  PrefWidget *wid = Q_NULLPTR;
  for (int entry = 0; entry <= static_cast<int>(ui_list->count()); entry++)
  {
    if (dynamic_cast<ConfigItem *>(ui_list->item(entry))->id() == page)
    {
      ui_list->setCurrentRow(entry);
      ui_stack->setCurrentIndex(page);
      wid = dynamic_cast<PrefWidget *>(ui_stack->widget(page));
      break;
    }
  }
  if (wid)
  {
    ui_helpText->setText(wid->description());
    ui_helpPixmap->setPixmap(wid->pixmap());
  }
}

void ConfigDlg::on_ui_factoryDefaults_clicked()
{
  dynamic_cast<PrefWidget *>(ui_stack->currentWidget())->factoryDefaultsSLOT();
}

void ConfigDlg::zoomInSLOT(double fac)
{
  m_scale->zoomInSLOT(fac);
  Q_EMIT zoomed();
}

void ConfigDlg::zoomOutSLOT(double fac)
{
  m_scale->zoomOutSLOT(fac);
  Q_EMIT zoomed();
}

void ConfigDlg::setSampleTimeSLOT(int sampleTime)
{
  m_recorder->setSampleTimeSLOT(sampleTime);
  on_ui_buttonBox_accepted();
}

void ConfigDlg::setGraphSizeSLOT(int size, int length)
{
  m_scale->setGraphSizeSLOT(size, length);
  on_ui_buttonBox_accepted();
}

void ConfigDlg::connectSLOT(bool /*connected*/)
{

}

QRect ConfigDlg::winRect() const
{
  QRect rect(m_settings->getInt("Position/x"),
             m_settings->getInt("Position/y"),
             m_settings->getInt("Position/width", 500),
             m_settings->getInt("Position/height", 350));

  return rect;
}

void ConfigDlg::setWinRect(const QRect &rect)
{
  m_winRect = rect;
}

void ConfigDlg::on_ui_buttonBox_rejected()
{
  m_settings->clear();
  int count = m_settings->getInt("Custom colors/count");
  for (int i = 0; i < count; i++)
    QColorDialog::setCustomColor(i, m_settings->getColor(QString("Custom colors/color_%1").arg(i)));
  for (int i = 0; i < NumItems; ++i)
    dynamic_cast<PrefWidget *>(ui_stack->widget(i))->defaultsSLOT();
  hide();
}

void ConfigDlg::setCurrentTipSLOT(int id)
{
  m_settings->setInt("QtDMM/tip-id", id);
  m_settings->save();
}

int ConfigDlg::currentTipId() const
{
  return m_settings->getInt("QtDMM/tip-id");
}

void ConfigDlg::on_ui_buttonBox_accepted()
{
  m_settings->setInt("Custom colors/count", QColorDialog::customCount());

  for (int i = 0; i < QColorDialog::customCount(); i++)
    m_settings->setColor(QString("Custom colors/color_%1").arg(i), QColorDialog::customColor(i));
  m_settings->setInt("Position/x", m_winRect.x());
  m_settings->setInt("Position/y", m_winRect.y());
  m_settings->setInt("Position/width", m_winRect.width());
  m_settings->setInt("Position/height", m_winRect.height());

  m_settings->setInt("Printer/page-size", static_cast<int>(m_printer->pageLayout().pageSize().id()));
  m_settings->setInt("Printer/page-orientation", static_cast<int>(m_printer->pageLayout().orientation()));
  m_settings->setInt("Printer/color", static_cast<int>(m_printer->colorMode()));
  m_settings->setString("Printer/name", m_printer->printerName());
  m_settings->setString("Printer/filename", m_printer->outputFileName());
  m_settings->setBool("Printer/print-file", (m_printer->outputFormat() == QPrinter::PdfFormat) ? true : false);
  m_settings->setInt("Config/LastItem", dynamic_cast<ConfigItem *>(ui_list->currentItem())->id());

  for (int i = 0; i < NumItems; ++i)
    dynamic_cast<PrefWidget *>(ui_stack->widget(i))->applySLOT();
  m_settings->save();
  Q_EMIT accepted();

  if ((sender() == ui_buttonBox) && m_buttonBox_OK)
    hide();
}
void ConfigDlg::on_ui_buttonBox_clicked(QAbstractButton *button)
{
  if (ui_buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    m_buttonBox_OK = true;
  else
    m_buttonBox_OK = false;
}

void ConfigDlg::writePrinter(QPrinter *printer)
{
  m_printer = printer;
  on_ui_buttonBox_accepted();
}

void ConfigDlg::readPrinter(QPrinter *printer)
{
  m_printer = printer;

  m_printer->setPageSize(QPageSize(static_cast<QPageSize::PageSizeId>(m_settings->getInt("Printer/page-size"))));
  m_printer->setPageOrientation(static_cast<QPageLayout::Orientation>(m_settings->getInt("Printer/page-orientation")));
  m_printer->setColorMode(static_cast<QPrinter::ColorMode>(m_settings->getInt("Printer/color", 1)));
  m_printer->setPrinterName(m_settings->getString("Printer/name", "lp"));
  m_printer->setOutputFileName(m_settings->getString("Printer/filename"));
  m_printer->setOutputFormat((m_settings->getBool("Printer/print-file")) ? QPrinter::PdfFormat : QPrinter::NativeFormat);
}

void ConfigDlg::on_ui_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *)
{
  int id = dynamic_cast<ConfigItem *>(current)->id();
  PrefWidget *wid = dynamic_cast<PrefWidget *>(ui_stack->widget(id));
  ui_stack->setCurrentWidget(wid);

  ui_helpText->setText(wid->description());
  ui_helpPixmap->setPixmap(wid->pixmap());
}

void ConfigDlg::thresholdChangedSLOT(DMMGraph::CursorMode mode, double value)
{
  switch (mode)
  {
    case DMMGraph::Trigger:
      m_recorder->setThreshold(value);
      break;
    case DMMGraph::External:
      m_execute->setThreshold(value);
      break;
    case DMMGraph::Integration:
      m_integration->setThreshold(value);
      break;
    default:
      std::cerr << "Unexpected CursorMode in configdlg.cpp:418" << std::endl;
      break;
  }
}

/////////////////////////////////////////////////////////////////
// RECORDER
//
DMMGraph::SampleMode ConfigDlg::sampleMode() const
{
  return m_recorder->sampleMode();
}

int ConfigDlg::sampleStep() const
{
  return m_recorder->sampleStep();
}

int ConfigDlg::sampleLength() const
{
  return m_recorder->sampleLength();
}

double ConfigDlg::fallingThreshold() const
{
  return m_recorder->fallingThreshold();
}

double ConfigDlg::raisingThreshold() const
{
  return m_recorder->raisingThreshold();
}

QTime ConfigDlg::startTime() const
{
  return m_recorder->startTime();
}

/////////////////////////////////////////////////////////////////
// EXECUTE
//
bool ConfigDlg::startExternal() const
{
  return m_execute->startExternal();
}

bool ConfigDlg::externalFalling() const
{
  return m_execute->externalFalling();
}

double ConfigDlg::externalThreshold() const
{
  return m_execute->externalThreshold();
}

bool ConfigDlg::disconnectExternal() const
{
  return m_execute->disconnectExternal();
}

QString ConfigDlg::externalCommand() const
{
  return m_execute->externalCommand();
}

/////////////////////////////////////////////////////////////////
// GUI
//
bool ConfigDlg::showTip() const
{
  return m_gui->showTip();
}

bool ConfigDlg::showBar() const
{
  return m_gui->showBar();
}

bool ConfigDlg::showMinMax() const
{
  return m_gui->showMinMax();
}

bool ConfigDlg::alertUnsavedData() const
{
  return m_gui->alertUnsavedData();
}

bool ConfigDlg::useTextLabel() const
{
  return m_gui->useTextLabel();
}

QColor ConfigDlg::displayBgColor() const
{
  return m_gui->displayBgColor();
}

QColor ConfigDlg::displayTextColor() const
{
  return m_gui->displayTextColor();
}

bool ConfigDlg::saveWindowPosition() const
{
  return m_gui->saveWindowPosition();
}

bool ConfigDlg::saveWindowSize() const
{
  return m_gui->saveWindowSize();
}

void ConfigDlg::setShowTipsSLOT(bool on)
{
  m_gui->on_ui_tipOfTheDay_toggled(on);
}

bool ConfigDlg::showDmmToolbar() const
{
  return m_gui->showDmmToolbar();
}

bool ConfigDlg::showGraphToolbar() const
{
  return m_gui->showGraphToolbar();
}

bool ConfigDlg::showFileToolbar() const
{
  return m_gui->showFileToolbar();
}

bool ConfigDlg::showDisplay() const
{
  return m_gui->showDisplay();
}

void ConfigDlg::setToolbarVisibility(bool disp, bool dmm, bool graph, bool file)
{
  m_gui->setToolbarVisibility(disp, dmm, graph, file);
}

/////////////////////////////////////////////////////////////////
// SCALE
//
bool ConfigDlg::automaticScale() const
{
  return m_scale->automaticScale();
}

bool ConfigDlg::includeZero() const
{
  return m_scale->includeZero();
}

double ConfigDlg::scaleMin() const
{
  return m_scale->scaleMin();
}

double ConfigDlg::scaleMax() const
{
  return m_scale->scaleMax();
}

int ConfigDlg::windowSeconds() const
{
  return m_scale->windowSeconds();
}

int ConfigDlg::totalSeconds() const
{
  return m_scale->totalSeconds();
}

/////////////////////////////////////////////////////////////////
// INTEGRATION
//
double ConfigDlg::intScale() const
{
  return m_integration->intScale();
}

double ConfigDlg::intThreshold() const
{
  return m_integration->intThreshold();
}

double ConfigDlg::intOffset() const
{
  return m_integration->intOffset();
}

bool ConfigDlg::showIntegration() const
{
  return m_integration->showIntegration();
}

QColor ConfigDlg::intColor() const
{
  return m_integration->intColor();
}

QColor ConfigDlg::intThresholdColor() const
{
  return m_integration->intThresholdColor();
}

int ConfigDlg::intLineWidth() const
{
  return m_integration->intLineWidth();
}

int ConfigDlg::intLineMode() const
{
  return m_integration->intLineMode();
}

int ConfigDlg::intPointMode() const
{
  return m_integration->intPointMode();
}

/////////////////////////////////////////////////////////////////
// DMM
//
bool ConfigDlg::rts() const
{
  return m_dmm->rts();
}

bool ConfigDlg::dtr() const
{
  return m_dmm->dtr();
}

QSerialPort::Parity ConfigDlg::parity() const
{
  return m_dmm->parity();
}

bool ConfigDlg::externalSetup() const
{
  return m_dmm->externalSetup();
}

int ConfigDlg::bits() const
{
  return m_dmm->bits();
}

int ConfigDlg::stopBits() const
{
  return m_dmm->stopBits();
}

int ConfigDlg::speed() const
{
  return m_dmm->speed();
}

int ConfigDlg::numValues() const
{
  return m_dmm->numValues();
}

ReadEvent::DataFormat ConfigDlg::format() const
{
  return m_dmm->format();
}

int ConfigDlg::display() const
{
  return m_dmm->display();
}

QString ConfigDlg::dmmName() const
{
  return m_dmm->dmmName();
}

QString ConfigDlg::device() const
{
  return m_dmm->device();
}

/////////////////////////////////////////////////////////////////
// GRAPH
//
bool ConfigDlg::crosshair() const
{
  return m_graph->crosshair();
}

QColor ConfigDlg::bgColor() const
{
  return m_graph->bgColor();
}

QColor ConfigDlg::gridColor() const
{
  return m_graph->gridColor();
}

QColor ConfigDlg::dataColor() const
{
  return m_graph->dataColor();
}

QColor ConfigDlg::startColor() const
{
  return m_graph->startColor();
}

QColor ConfigDlg::externalColor() const
{
  return m_graph->externalColor();
}

QColor ConfigDlg::cursorColor() const
{
  return m_graph->cursorColor();
}

int ConfigDlg::lineWidth() const
{
  return m_graph->lineWidth();
}

int ConfigDlg::lineMode() const
{
  return m_graph->lineMode();
}

int ConfigDlg::pointMode() const
{
  return m_graph->pointMode();
}
