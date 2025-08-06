#include <QtGui>
#include <QtWidgets>

#include "portsprefs.h"
#include "settings.h"

PortsPrefs::PortsPrefs(QWidget *parent) : PrefWidget(parent)
{
  setupUi(this);
  m_label = tr("Custom ports");
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
#ifdef Q_OS_WINDOWS
  filter += ".exe";
#endif

  QString filename = QFileDialog::getOpenFileName(this, tr("Sigrok-cli executable"), "./",	QString("sigrok (%1)").arg(filter) );

  if (!filename.isNull())
    ui_sigrokExe->setText(filename);
}
