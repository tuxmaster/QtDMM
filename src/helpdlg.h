#pragma once

#include <QtCore>
#include "ui_uihelpdlg.h"

class Settings;


// The handbook: renders the Markdown pages from docs/user/, embedded as
// resources under :/Help/, in a QTextBrowser. The same files feed the MkDocs
// site, so there is one source for both.
class HelpDlg : public QDialog, private Ui::UIHelpDlg
{
  Q_OBJECT
public:
  explicit HelpDlg(Settings *settings, QWidget *parent = Q_NULLPTR);

  // Shows a page by its path below :/Help/, e.g. "recorder.md".
  void showPage(const QString &page);

  // Pages listed in the table of contents, as derived from index.md.
  QStringList contentPages() const { return m_contentPages; }
  QString currentPage() const;

protected:
  void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;

protected Q_SLOTS:
  void on_ui_home_clicked();
  void on_ui_close_clicked();
  void on_ui_contents_currentRowChanged(int row);
  void pageChanged(const QUrl &url);

private:
  void buildContents();
  void saveGeometry();

  Settings *m_cfg;
  QStringList m_contentPages;   // page path per list row, parallel to ui_contents
};
