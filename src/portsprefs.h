#pragma once

#include "ui_uiportsprefs.h"

#include <QVector>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>

class PortsPrefs : public PrefWidget, private Ui::UIPortsPrefs
{
  Q_OBJECT
public:
  PortsPrefs(QWidget *parent = Q_NULLPTR);
  ~PortsPrefs();

  QStringList customPortList();
  QString sigrokExecutable() { return ui_sigrokExe->text().trimmed(); }

public Q_SLOTS:
  virtual void   defaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   factoryDefaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   applySLOT() Q_DECL_OVERRIDE;

protected Q_SLOTS:
  void           on_ui_sigrokExeButton_clicked();

private:
  QVector<QLineEdit*> m_portEdits;
  QVector<QComboBox*> m_portTypes;
};
