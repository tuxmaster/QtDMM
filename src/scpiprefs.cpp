// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "scpiprefs.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHostInfo>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include "settings.h"

ScpiPrefs::ScpiPrefs(QWidget *parent) : PrefWidget(parent)
{
  m_label = tr("SCPI server");
  m_description = tr("<b>Lets other programs read the meter over the network:</b> QtDMM answers "
                     "SCPI queries (*IDN?, READ?, ...) on a TCP port, like a bench instrument.");
  m_pixmap = new QPixmap(":/Symbols/scpi.xpm");

  auto *layout = new QVBoxLayout(this);
  auto *intro = new QLabel(tr("With the server on, lxi-tools, LabVIEW, PyVISA or a "
                              "few lines of Python can read the current value as if the multimeter "
                              "were a bench instrument with a LAN port. The server only reports; "
                              "it never sends anything to the meter. INITiate/ABORt start and stop "
                              "the recorder, INPut ON/OFF connect and disconnect."), this);
  intro->setWordWrap(true);
  layout->addWidget(intro);

  auto *group = new QGroupBox(tr("Server"), this);
  auto *form = new QFormLayout(group);
  m_enabled = new QCheckBox(tr("&Enable the SCPI server"), group);
  form->addRow(m_enabled);
  m_port = new QSpinBox(group);
  m_port->setRange(1, 65535);
  m_port->setValue(5025);
  m_port->setToolTip(tr("5025 is the usual raw-socket SCPI port. When it is taken (another QtDMM "
                        "instance), the next free one is used and shown in the status bar."));
  auto *portLabel = new QLabel(tr("&Port:"), group);
  portLabel->setBuddy(m_port);
  form->addRow(portLabel, m_port);
  m_bind = new QComboBox(group);
  m_bind->addItem(tr("This computer only (localhost)"));
  m_bind->addItem(tr("All network interfaces"));
  m_bind->setToolTip(tr("There is no authentication: anyone who can reach the port can read the "
                        "meter and start the recorder. Open it to the network only where you trust it."));
  auto *bindLabel = new QLabel(tr("&Listen on:"), group);
  bindLabel->setBuddy(m_bind);
  form->addRow(bindLabel, m_bind);
  m_mdns = new QCheckBox(tr("&Announce by mDNS (_scpi-raw._tcp), so lxi discover finds it"), group);
  form->addRow(m_mdns);
  layout->addWidget(group);

  m_hint = new QLabel(this);
  m_hint->setWordWrap(true);
  m_hint->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(m_hint);

  m_status = new QLabel(this);
  m_status->setWordWrap(true);
  layout->addWidget(m_status);
  layout->addStretch(1);

  connect(m_enabled, &QCheckBox::toggled, this, &ScpiPrefs::updateHint);
  connect(m_bind, &QComboBox::currentIndexChanged, this, &ScpiPrefs::updateHint);
  connect(m_port, &QSpinBox::valueChanged, this, &ScpiPrefs::updateHint);
  connect(m_enabled, &QCheckBox::toggled, group, [this](bool on)
  {
    m_port->setEnabled(on);
    m_bind->setEnabled(on);
    m_mdns->setEnabled(on && m_bind->currentIndex() == 1);
  });
  connect(m_bind, &QComboBox::currentIndexChanged, this, [this](int idx)
  {
    m_mdns->setEnabled(m_enabled->isChecked() && idx == 1);
  });
  updateHint();
}

bool ScpiPrefs::enabled() const { return m_enabled->isChecked(); }
int ScpiPrefs::port() const { return m_port->value(); }
bool ScpiPrefs::allInterfaces() const { return m_bind->currentIndex() == 1; }
bool ScpiPrefs::mdns() const { return m_mdns->isChecked() && allInterfaces(); }

void ScpiPrefs::setStatus(const QString &text)
{
  m_status->setText(text);
}

void ScpiPrefs::updateHint()
{
  if (!m_enabled->isChecked())
  {
    m_hint->clear();
    return;
  }
  const QString host = m_bind->currentIndex() == 1 ? QHostInfo::localHostName() : QString("localhost");
  m_hint->setText(tr("Try it: <code>lxi scpi -a %1 -p %2 \"*IDN?\"</code> or "
                     "<code>printf 'READ?\\n' | nc %1 %2</code>").arg(host).arg(m_port->value()));
}

void ScpiPrefs::defaultsSLOT()
{
  m_enabled->setChecked(m_cfg->getBool("Scpi/enabled", false));
  m_port->setValue(m_cfg->getInt("Scpi/port", 5025));
  m_bind->setCurrentIndex(m_cfg->getBool("Scpi/all-interfaces", false) ? 1 : 0);
  m_mdns->setChecked(m_cfg->getBool("Scpi/mdns", true));
  m_enabled->toggled(m_enabled->isChecked());
  updateHint();
}

void ScpiPrefs::factoryDefaultsSLOT()
{
  m_enabled->setChecked(false);
  m_port->setValue(5025);
  m_bind->setCurrentIndex(0);
  m_mdns->setChecked(true);
  updateHint();
}

void ScpiPrefs::applySLOT()
{
  m_cfg->setBool("Scpi/enabled", m_enabled->isChecked());
  m_cfg->setInt("Scpi/port", m_port->value());
  m_cfg->setBool("Scpi/all-interfaces", m_bind->currentIndex() == 1);
  m_cfg->setBool("Scpi/mdns", m_mdns->isChecked());
}
