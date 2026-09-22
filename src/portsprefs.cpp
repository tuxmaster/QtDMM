#include <QtGui>
#include <QtWidgets>

#include "portsprefs.h"
#include "settings.h"

PortsPrefs::PortsPrefs(QWidget *parent) : PrefWidget(parent)
{
  setupUi(this);
  m_label = tr("Special ports");
  m_description = tr("<b>Here you can configure custom ports"
                     " for RFC2217 and SIGROK.</b>");
  m_pixmap = new QPixmap(":/Symbols/connect_icon.xpm");

  m_portEdits = {
    ui_customPort0, ui_customPort1, ui_customPort2, ui_customPort3,
    ui_customPort4, ui_customPort5, ui_customPort6, ui_customPort7,
    ui_customPort8, ui_customPort9
  };

  m_portTypes = {
    ui_customPortType0, ui_customPortType1, ui_customPortType2, ui_customPortType3,
    ui_customPortType4, ui_customPortType5, ui_customPortType6, ui_customPortType7,
    ui_customPortType8, ui_customPortType9
  };

  Q_ASSERT(m_portEdits.size() == m_portTypes.size());

  // qtdmm-bridge announces its ports by mDNS; a browse fills the list,
  // a double-click takes one over
  m_browser = new MdnsBrowser(this);
  connect(m_browser, &MdnsBrowser::found, this, [this](const MdnsBrowser::Service &s)
  {
    // "dory — 192.168.178.184:4711 — UT61E": the host, where to connect,
    // and the port's name on the bridge (its device path when unnamed)
    const QString where = s.address.isNull() ? s.host : s.address.toString();
    QString hostShort = s.host;
    if (hostShort.endsWith(".local"))
      hostShort.chop(6);
    const QString device = s.txt.value("name", s.txt.value("device"));
    auto *item = new QListWidgetItem(QString("%1  —  %2:%3%4").arg(hostShort, where).arg(s.port)
                                       .arg(device.isEmpty() ? QString() : "  —  " + device), ui_bridgeList);
    item->setData(Qt::UserRole, QString("%1:%2").arg(where).arg(s.port));
    item->setToolTip(tr("Bridge %1 on %2, port %3, serving %4 (version %5)")
                       .arg(s.instance, s.host).arg(s.port).arg(s.txt.value("device"), s.txt.value("version")));
  });
  connect(m_browser, &MdnsBrowser::finished, this, [this]
  {
    ui_bridgeSearch->setEnabled(true);
    ui_bridgeHint->setText(ui_bridgeList->count() == 0
                             ? tr("No bridge found. Is qtdmm-bridge running with --mdns in this network?")
                             : tr("%n port(s) found.", "", ui_bridgeList->count()));
  });
  connect(ui_bridgeList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) { on_ui_bridgeAdd_clicked(); });
  connect(ui_bridgeList, &QListWidget::itemSelectionChanged, this,
          [this] { ui_bridgeAdd->setEnabled(!ui_bridgeList->selectedItems().isEmpty()); });
}

void PortsPrefs::on_ui_bridgeSearch_clicked()
{
  ui_bridgeList->clear();
  ui_bridgeAdd->setEnabled(false);
  ui_bridgeSearch->setEnabled(false);
  ui_bridgeHint->setText(tr("Searching (3 s)..."));
  m_browser->browse(QStringLiteral("_qtdmm-bridge._tcp.local"), 3000);
  if (!m_browser->isActive())   // no usable interface: finished() already came
    ui_bridgeHint->setText(tr("No network interface for multicast."));
}

void PortsPrefs::on_ui_bridgeAdd_clicked()
{
  const QList<QListWidgetItem *> selected = ui_bridgeList->selectedItems();
  if (selected.isEmpty())
    return;
  const QString address = selected.first()->data(Qt::UserRole).toString();
  // already there?
  for (int i = 0; i < m_portEdits.size(); ++i)
    if (m_portTypes[i]->currentText() == "RFC2217" && m_portEdits[i]->text().trimmed() == address)
    {
      ui_bridgeHint->setText(tr("%1 is already in line %2.").arg(address).arg(i + 1));
      return;
    }
  for (int i = 0; i < m_portEdits.size(); ++i)
    if (m_portEdits[i]->text().trimmed().isEmpty())
    {
      m_portTypes[i]->setCurrentIndex(qMax(0, m_portTypes[i]->findText("RFC2217")));
      m_portEdits[i]->setText(address);
      ui_bridgeHint->setText(tr("Added as custom port %1; it appears in the port list after Apply.").arg(i + 1));
      return;
    }
  ui_bridgeHint->setText(tr("All ten custom port lines are in use."));
}

PortsPrefs::~PortsPrefs()
{
  delete m_pixmap;
}

QStringList PortsPrefs::customPortList()
{
  QStringList result;
  for (int i = 0; i < m_portEdits.size(); ++i)
  {
    if (!m_portEdits[i]->text().isEmpty())
      result << (m_portTypes[i]->currentText() + " " + m_portEdits[i]->text().trimmed());
  }
  return result;
}


void PortsPrefs::defaultsSLOT()
{
  for (int i = 0; i < m_portEdits.size(); ++i)
  {
    QString setting = m_cfg->getString(QString("Port settings/custom_device%1").arg(i)).trimmed();

    if (setting.isEmpty())
    {
      m_portEdits[i]->clear();
      continue;
    }

    const int spaceIndex = setting.indexOf(' ');
    if (spaceIndex <= 0) {
      m_portEdits[i]->clear();
      continue;
    }

    QString type = setting.left(spaceIndex);
    QString port = setting.mid(spaceIndex + 1);

    int typeIndex = m_portTypes[i]->findText(type);
    if (typeIndex >= 0)
      m_portTypes[i]->setCurrentIndex(typeIndex);

    m_portEdits[i]->setText(port);
  }

  ui_sigrokExe->setText(m_cfg->getString("Port settings/sigrok_exe","sigrok-cli"));
}

void PortsPrefs::factoryDefaultsSLOT()
{
  for (int i = 0; i < m_portEdits.size(); ++i)
  {
    m_portEdits[i]->setText("");
    m_portTypes[i]->setCurrentIndex(0);
  }
  ui_sigrokExe->setText("sigrok-cli");
}


void PortsPrefs::applySLOT()
{
  QStringList ports = customPortList();

  for (int i = 0; i < m_portEdits.size(); ++i)
  {
    m_cfg->setString(QString("Port settings/custom_device%1").arg(i), (i < ports.size()) ? ports[i] : "");
  }

  m_cfg->setString("Port settings/sigrok_exe",ui_sigrokExe->text());
}

void PortsPrefs::on_ui_sigrokExeButton_clicked()
{
  QString filter = "sigrok-cli";
#ifdef Q_OS_WIN
  filter += ".exe";
#endif

  QString filename = QFileDialog::getOpenFileName(this, tr("Sigrok-cli executable"), "./",	QString("sigrok (%1)").arg(filter) );

  if (!filename.isNull())
    ui_sigrokExe->setText(filename);
}
