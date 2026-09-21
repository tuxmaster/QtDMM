// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Instances dialog without a running second QtDMM: the list is built from the
// config files in a temporary config directory, delete mode removes the
// selected instance's file, and a calculated instance is created with the
// keys DMM and MainWin rely on.

#include <QtWidgets>
#include <QTemporaryDir>

#include "instancesdlg.h"
#include "settings.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAIL:" << what;
    ++failed;
  }
}

static QStringList instanceButtons(QDialog &dlg)
{
  // QListWidget::clear() deletes the row widgets deferred
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  QStringList names;
  for (QPushButton *b : dlg.findChildren<QPushButton *>())
    if (b->property("instanceId").isValid())
      names << b->property("instanceId").toString();
  return names;
}

int main(int argc, char **argv)
{
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);
  app.setApplicationName("qtdmm_test");

  QTemporaryDir dir;
  check(dir.isValid(), "temporary config dir");

  // --- 1. two instance configs on disk -> both listed ---
  Settings settings("default", dir.path());
  settings.setString("DMM/model", "UT61E");
  settings.save();
  QString probeFile;
  {
    Settings probe("probe", dir.path());
    probe.setString("DMM/model", "UT803");
    probe.save();
    probeFile = probe.fileName();
  }
  check(QFile::exists(probeFile), "probe config written: " + probeFile);

  InstancesDlg dlg(&settings, "default", dir.path());
  dlg.setInstancesOnline({"default"});
  QStringList listed = instanceButtons(dlg);
  check(listed.contains("default") && listed.contains("probe"),
        "both instances listed: " + listed.join(','));

  // --- 2. delete mode: check "probe", press "-" again -> file gone ---
  auto *del = dlg.findChild<QToolButton *>("ui_instance_del");
  check(del != nullptr, "delete button found");
  if (del)
  {
    check(del->isCheckable() && !del->isChecked(), "delete button is a toggle");
    del->click();          // enter delete mode; buttons become checkable
    QPushButton *probeBtn = nullptr;
    for (QPushButton *b : dlg.findChildren<QPushButton *>())
      if (b->property("instanceId").toString() == "probe")
        probeBtn = b;
    check(probeBtn && probeBtn->isCheckable(), "probe button checkable in delete mode");
    if (probeBtn)
      probeBtn->setChecked(true);
    del->click();          // leave delete mode: delete what is checked
    check(!QFile::exists(probeFile), "probe config removed");
    check(!instanceButtons(dlg).contains("probe"), "probe no longer listed");
  }

  // --- 3. a calculated instance gets the keys the new process needs ---
  const QString calcFile = InstancesDlg::createCalculatedInstance("p", dir.path(), "W", "u * i", false);
  check(QFile::exists(calcFile), "calc config created: " + calcFile);
  {
    Settings calc("p", dir.path());
    check(calc.getString("DMM/model") == "QtDMM Calculated value", "model key");
    check(calc.getBool("DMM/configured"), "configured flag set (auto-connect at first start)");
    check(calc.getString("Port settings/device") == "calc W u * i", "device string");
    check(calc.getString("DMM/calc-expression") == "u * i", "formula key");
  }

  if (failed == 0)
    qInfo() << "All instances dialog tests passed.";
  else
    qWarning() << failed << "instances dialog test(s) failed.";
  return failed == 0 ? 0 : 1;
}
