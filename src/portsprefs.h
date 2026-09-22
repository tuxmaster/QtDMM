#pragma once

#include "ui_uiportsprefs.h"

#include <QVector>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>

#include "mdnsbrowser.h"

/// Settings page "Special ports": user-defined port entries (type + address,
/// e.g. RFC2217 host:port or a sigrok driver string) added to the port list
/// of the multimeter page, and the path of sigrok-cli.
class PortsPrefs : public PrefWidget, private Ui::UIPortsPrefs
{
  Q_OBJECT
public:
  PortsPrefs(QWidget *parent = Q_NULLPTR);
  ~PortsPrefs();

  /// The non-empty entries as "<type> <address>" strings.
  QStringList customPortList();
  QString sigrokExecutable() { return ui_sigrokExe->text().trimmed(); }

public Q_SLOTS:
  virtual void   defaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   factoryDefaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   applySLOT() Q_DECL_OVERRIDE;

protected Q_SLOTS:
  void           on_ui_sigrokExeButton_clicked();
  /// mDNS browse for qtdmm-bridge announcements, fills the list.
  void           on_ui_bridgeSearch_clicked();
  /// The selected bridge port into the next free custom port line.
  void           on_ui_bridgeAdd_clicked();

private:

  MdnsBrowser    *m_browser = nullptr;
  QVector<QLineEdit*> m_portEdits;
  QVector<QComboBox*> m_portTypes;
};
