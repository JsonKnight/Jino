// src/app/menu_manager.hpp
#pragma once

#include <QIcon>
#include <QObject>
#include <QStringList>
#include <QVariant>

class QMainWindow;
class QMenuBar;
class QMenu;
class QAction;
class QTabWidget;
class QWidgetAction;
class QWidget;
class QLabel;
class QToolButton;
class QPushButton;

namespace fa {
class QtAwesome;
}

namespace Jino::App {

class MenuManager : public QObject {
  Q_OBJECT

public:
  explicit MenuManager(QMainWindow *parentWindow, fa::QtAwesome *awesome);
  ~MenuManager() override = default;

  void setupMenusAndActions(QMenuBar *menuBar, QTabWidget *tabWidget);

  QAction *newTabAction = nullptr;
  QAction *openAction = nullptr;
  QAction *saveAction = nullptr;
  QAction *saveAsAction = nullptr;
  QAction *closeTabAction = nullptr;
  QAction *quitAction = nullptr;
  QAction *undoAction = nullptr;
  QAction *redoAction = nullptr;
  QAction *cutAction = nullptr;
  QAction *copyAction = nullptr;
  QAction *pasteAction = nullptr;
  QAction *selectAllAction = nullptr;
  QAction *clearRecentAction = nullptr;
  QAction *showBuffersAction = nullptr;
  QAction *showRecentAction = nullptr;

public slots:
  void updateRecentMenu(const QStringList &recentFiles);
  void updateBuffersMenu(int currentTab = -1);
  void updateActionStates(bool editorAvailable, bool hasSelection,
                          bool undoAvailable, bool redoAvailable,
                          bool pasteAvailable);

signals:
  void openRecentFileRequested(const QString &filePath);
  void clearRecentFilesRequested();
  void switchToBufferRequested(int index);
  void showBuffersMenuRequested();
  void showRecentMenuRequested();
  void removeRecentFileRequested(const QString &filePath);
  void closeBufferRequested(int index);

private slots:
  void handleClearRecentTriggered();
  void handleRecentWidgetClicked();
  void handleRecentWidgetRemoveClicked();
  void handleBufferWidgetClicked();
  void handleBufferWidgetRemoveClicked();

private:
  void createBaseActions();
  QWidget *createMenuItemWidget(const QString &text, const QVariant &data,
                                bool isBuffer, const QIcon &icon = QIcon());

  QMainWindow *mainWindow;
  QTabWidget *mainTabWidget = nullptr;
  fa::QtAwesome *awesome = nullptr;

  QMenu *fileMenu = nullptr;
  QMenu *editMenu = nullptr;
  QMenu *buffersMenu = nullptr;
  QMenu *recentMenu = nullptr;
};

} // namespace Jino::App
