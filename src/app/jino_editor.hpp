// src/app/jino_editor.hpp
#pragma once

class EditorWidget;

#include "app/menu_manager.hpp"
#include "app/status_bar_manager.hpp"
#include "core/constants.hpp"
#include "editor/vim/vim_modes.hpp"

#include <QElapsedTimer>
#include <QFile>
#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QStringList>

class QTabWidget;
class QCloseEvent;
class QWidget;
class QLabel;
class QTimer;
class QStatusBar;
class QInputDialog;

namespace fa {
class QtAwesome;
}

namespace Jino::App {

class StatusBarManager;
class MenuManager;

class JinoEditor : public QMainWindow {
  Q_OBJECT

public:
  explicit JinoEditor(QWidget *parent = nullptr,
                      const QString &workspaceName = "jino_default",
                      fa::QtAwesome *awesome = nullptr);
  ~JinoEditor() override;

  void openFilesFromCli(const QStringList &filePaths);
  QString getCurrentFile(EditorWidget *e) const;
  QString getBaseNameForEditor(EditorWidget *editor) const;
  QString formatFileInfoToolTip(const QString &filePath) const;

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void openFile();
  bool saveFile();
  bool saveFileAs();
  void newTab();
  void closeCurrentTab();
  void maybeCloseTab(int index);
  void updateWindowTitle();
  void updateUiStates();
  void updateElapsedTime();
  void handleModificationChanged(bool modified);
  void onNewTabAction();
  void onOpenAction();
  void onSaveAction();
  void onSaveAsAction();
  void onCloseTabAction();
  void onUndoAction();
  void onRedoAction();
  void onCutAction();
  void onCopyAction();
  void onPasteAction();
  void onSelectAllAction();
  void handleOpenRecentFileRequested(const QString &filePath);
  void handleClearRecentFilesRequested();
  void handleSwitchToBufferRequested(int index);
  void showBuffersMenu();
  void showRecentMenu();

  void onZoomInAction();
  void onZoomOutAction();
  void handleResetZoomRequested();

  void handleStatusBarEditorModeChangeRequested();
  void handleStatusBarCloseRequested();
  void changeEditorMode(Jino::Constants::EditorFileType newMode);
  void handleGoToLineRequested();
  void handleGoToColumnRequested();
  void handleSaveRequested();
  void handleGoToTopRequested();
  void handleGoToCenterRequested();
  void handleGoToBottomRequested();
  void handlePasteAndReplaceRequested();
  void handleClearFileRequested();

  void handleRemoveRecentFileRequested(const QString &filePath);
  void handleCloseBufferRequested(int index);

  void handleEditorZoomChanged(int percent);

  void redoEdit();
  void undoEdit();
  void cutEdit();
  void copyEdit();
  void pasteEdit();
  void selectAllEdit();
  void prevTab();
  void nextTab();

  void handleCurrentTabChanged(int index);

private:
  void loadFont();
  void loadSettings();
  void saveSettings();
  void setupEditorConnections(EditorWidget *editor);
  void disconnectEditorSignals(EditorWidget *editor);
  void setupInitialUi();
  void setupShortcuts();
  bool loadFileContent(const QString &path, QString &content);
  bool saveFileLogic(const QString &path);
  bool autoSaveBufferOnClose(EditorWidget *editor);
  void openSingleFile(const QString &filePath);
  EditorWidget *currentEditorWidget() const;
  EditorWidget *editorWidgetForIndex(int index) const;
  void applyEditorFont(EditorWidget *editor);
  void setCurrentFile(EditorWidget *editor, const QString &path);
  void updateTabTitle(int index);
  QString getRandomAngelName() const;
  void setBaseNameForEditor(EditorWidget *editor, const QString &name);
  void addRecentFile(const QString &filePath);
  void cleanupEditorData(QWidget *editorWidget);
  void updateTabToolTip(int index);

  QTabWidget *tabWidget = nullptr;
  QTimer *elapsedTimerClock = nullptr;
  QElapsedTimer sessionTimer;

  StatusBarManager *statusBarManager = nullptr;
  MenuManager *menuManager = nullptr;
  fa::QtAwesome *awesome = nullptr;

  QMap<QWidget *, QString> editorFilePaths;
  QMap<QWidget *, QString> editorBaseNames;
  QString currentWorkspaceName;
  QStringList recentFilesList;
  bool initialTabCreated = false;
  QPointer<EditorWidget> currentlyConnectedEditor;
};

} // namespace Jino::App
