// src/app/status_bar_manager.hpp
#pragma once

#include "core/constants.hpp"
#include "editor/vim/vim_modes.hpp"

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QWidget>

class QMainWindow;
class QToolBar;
class QLabel;
class QStatusBar;
class EditorWidget;
class QToolButton;
class QHBoxLayout;

namespace fa {
class QtAwesome;
enum icon_enum : int;
} // namespace fa

namespace Jino::App {

fa::icon_enum getIconForVimMode(Jino::Editor::Vim::Mode mode);
fa::icon_enum getIconForFileType(Jino::Constants::EditorFileType type);

class StatusBarManager : public QObject {
  Q_OBJECT

public:
  explicit StatusBarManager(QMainWindow *parentWindow,
                            const QString &workspaceName,
                            fa::QtAwesome *awesome);
  ~StatusBarManager() override = default;

  void setupUI();
  void updateTimeDisplay(const QString &timeString);
  QWidget *getEditorModeWidget() const;

public slots:
  void updateTopStatusBar(EditorWidget *currentEditor);
  void updateEditorStats(EditorWidget *currentEditor);
  void updateZoomDisplay(int zoomPercent);

signals:
  void editorModeChangeRequested();
  void closeCurrentTabRequested();
  void resetZoomRequested();
  void goToLineRequested();
  void goToColumnRequested();
  void saveRequested();
  void fileInfoRequested();
  void goToTopRequested();
  void goToCenterRequested();
  void goToBottomRequested();
  void pasteAndReplaceRequested();
  void clearFileRequested();

private slots:
  void handleZoomWidgetClicked();

private:
  void createTopStatusBar();
  void setupMainStatusBar();

  QMainWindow *mainWindow;
  QString currentWorkspaceName;
  fa::QtAwesome *awesome = nullptr;
  QIcon workspaceIcon;

  QToolBar *topStatusBar = nullptr;
  QPointer<QLabel> vimStatusLabel;
  QPointer<QToolButton> editorModeButton;
  QPointer<QWidget> positionActionWidget;
  QPointer<QToolButton> lineButton;
  QPointer<QToolButton> columnButton;
  QPointer<QToolButton> closeButton;
  QPointer<QToolButton> saveButton;
  QPointer<QToolButton> infoButton;
  QPointer<QToolButton> goToTopButton;
  QPointer<QToolButton> goToCenterButton;
  QPointer<QToolButton> goToBottomButton;
  QPointer<QToolButton> pasteReplaceButton;
  QPointer<QToolButton> clearFileButton;
  QPointer<QLabel> statsCharsLabel;
  QPointer<QLabel> statsWordsLabel;
  QPointer<QLabel> timeLabel;
  QPointer<QToolButton> zoomWidget;

  QPointer<QLabel> statusBarWorkspaceLabel;
};

} // namespace Jino::App
