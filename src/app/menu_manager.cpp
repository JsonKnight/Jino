#include "app/menu_manager.hpp"
#include "QtAwesome.h"
#include "core/constants.hpp"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QStyle>
#include <QTabWidget>
#include <QToolButton>
#include <QWidget>
#include <QWidgetAction>

namespace Jino::App {

MenuManager::MenuManager(QMainWindow *parentWindow, fa::QtAwesome *awesomePtr)
    : QObject(parentWindow), mainWindow(parentWindow), awesome(awesomePtr) {}

void MenuManager::createBaseActions() {
  using namespace fa;

  newTabAction = new QAction(
      awesome ? awesome->icon(fa_solid, fa_file_circle_plus) : QIcon(),
      "&New Tab", mainWindow);
  openAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_folder_open) : QIcon(),
                  "&Open...", mainWindow);
  saveAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_floppy_disk) : QIcon(),
                  "&Save", mainWindow);
  saveAsAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_floppy_disk) : QIcon(),
                  "Save &As...", mainWindow);
  closeTabAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_xmark) : QIcon(),
                  "&Close Tab", mainWindow);
  quitAction = new QAction(
      awesome ? awesome->icon(fa_solid, fa_right_from_bracket) : QIcon(),
      "&Quit", mainWindow);
  undoAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_rotate_left) : QIcon(),
                  "&Undo", mainWindow);
  redoAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_rotate_right) : QIcon(),
                  "&Redo", mainWindow);
  cutAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_scissors) : QIcon(),
                  "Cu&t", mainWindow);
  copyAction = new QAction(awesome ? awesome->icon(fa_solid, fa_copy) : QIcon(),
                           "&Copy", mainWindow);
  pasteAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_paste) : QIcon(),
                  "&Paste", mainWindow);
  selectAllAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_object_group) : QIcon(),
                  "Select &All", mainWindow);
  clearRecentAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_trash_can) : QIcon(),
                  "&Clear Recent List", mainWindow);
  showBuffersAction =
      new QAction(awesome ? awesome->icon(fa_solid, fa_list_ul) : QIcon(),
                  "Show &Buffers", mainWindow);
  showRecentAction = new QAction(
      awesome ? awesome->icon(fa_solid, fa_clock_rotate_left) : QIcon(),
      "Show &Recent", mainWindow);

  newTabAction->setShortcut(Jino::Constants::KB_ALT_T);
  openAction->setShortcuts(
      {Jino::Constants::KB_ALT_O, Jino::Constants::KeyOpen()});
  saveAction->setShortcuts(
      {Jino::Constants::KB_ALT_S, Jino::Constants::KeySave()});
  saveAsAction->setShortcuts(
      {Jino::Constants::KB_ALT_SHIFT_S, Jino::Constants::KeySaveAs()});
  closeTabAction->setShortcut(Jino::Constants::KB_ALT_D);
  quitAction->setShortcuts(
      {Jino::Constants::KB_ALT_Q, Jino::Constants::KeyQuit()});
  undoAction->setShortcut(Jino::Constants::KeyUndo());
  redoAction->setShortcuts(
      {Jino::Constants::KeyRedo(), Jino::Constants::KB_CTRL_R});
  cutAction->setShortcut(Jino::Constants::KeyCut());
  copyAction->setShortcut(Jino::Constants::KeyCopy());
  pasteAction->setShortcut(Jino::Constants::KeyPaste());
  selectAllAction->setShortcut(Jino::Constants::KeySelectAll());
  showBuffersAction->setShortcut(Jino::Constants::KB_ALT_B);
  showRecentAction->setShortcut(Jino::Constants::KB_ALT_R);

  updateActionStates(false, false, false, false, false);
  clearRecentAction->setEnabled(false);
}

void MenuManager::setupMenusAndActions(QMenuBar *menuBar,
                                       QTabWidget *tabWidget) {
  mainTabWidget = tabWidget;
  createBaseActions();

  using namespace fa;
  QIcon fileIcon = awesome ? awesome->icon(fa_solid, fa_folder) : QIcon();
  QIcon editIcon =
      awesome ? awesome->icon(fa_solid, fa_pen_to_square) : QIcon();
  QIcon buffersIcon = awesome ? awesome->icon(fa_solid, fa_list_ul) : QIcon();
  QIcon recentIcon =
      awesome ? awesome->icon(fa_solid, fa_clock_rotate_left) : QIcon();

  fileMenu = menuBar->addMenu(fileIcon, "&File");
  fileMenu->addAction(newTabAction);
  fileMenu->addAction(openAction);
  fileMenu->addAction(saveAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addAction(closeTabAction);
  fileMenu->addSeparator();
  fileMenu->addAction(quitAction);

  editMenu = menuBar->addMenu(editIcon, "&Edit");
  editMenu->addAction(undoAction);
  editMenu->addAction(redoAction);
  editMenu->addSeparator();
  editMenu->addAction(cutAction);
  editMenu->addAction(copyAction);
  editMenu->addAction(pasteAction);
  editMenu->addSeparator();
  editMenu->addAction(selectAllAction);

  buffersMenu = menuBar->addMenu(buffersIcon, "&Buffers");

  recentMenu = menuBar->addMenu(recentIcon, "&Recent");
  connect(clearRecentAction, &QAction::triggered, this,
          &MenuManager::handleClearRecentTriggered);
}

void MenuManager::updateActionStates(bool editorAvailable, bool hasSelection,
                                     bool undoAvailable, bool redoAvailable,
                                     bool pasteAvailable) {

  bool modified = editorAvailable && undoAvailable;
  saveAction->setEnabled(modified);
  saveAsAction->setEnabled(editorAvailable);
  closeTabAction->setEnabled(mainTabWidget && mainTabWidget->count() > 0);

  undoAction->setEnabled(undoAvailable);
  redoAction->setEnabled(redoAvailable);
  cutAction->setEnabled(hasSelection);
  copyAction->setEnabled(hasSelection);
  pasteAction->setEnabled(pasteAvailable);
  selectAllAction->setEnabled(editorAvailable);

  newTabAction->setEnabled(true);
  openAction->setEnabled(true);
  quitAction->setEnabled(true);
}

QWidget *MenuManager::createMenuItemWidget(const QString &text,
                                           const QVariant &data, bool isBuffer,
                                           const QIcon &icon) {
  QWidget *widget = new QWidget();
  QHBoxLayout *layout = new QHBoxLayout(widget);
  layout->setContentsMargins(5, 2, 2, 2);
  layout->setSpacing(5);

  QPushButton *textButton = new QPushButton(icon, text);
  textButton->setFlat(true);
  textButton->setCursor(Qt::PointingHandCursor);

  textButton->setStyleSheet(
      "QPushButton { text-align: left; padding: 2px; border: none; "
      "background-color: transparent; color: palette(text); } "
      "QPushButton:hover { background-color: palette(highlight); color: "
      "palette(highlighted-text); }");
  textButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  textButton->setProperty("itemData", data);

  QToolButton *removeButton = new QToolButton();
  removeButton->setObjectName("MenuItemRemoveButton");
  removeButton->setAutoRaise(true);
  removeButton->setFixedSize(16, 16);
  removeButton->setIconSize(QSize(8, 8));
  removeButton->setFocusPolicy(Qt::NoFocus);
  removeButton->setToolTip(isBuffer ? "Close Buffer" : "Remove From Recent");
  removeButton->setProperty("itemData", data);
  if (awesome) {
    removeButton->setIcon(awesome->icon(fa::fa_solid, fa::fa_xmark));
  } else {
    removeButton->setIcon(
        QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));
  }

  removeButton->setStyleSheet(
      "QToolButton { border: none; background-color: transparent; } "
      "QToolButton:hover { background-color: rgba(255, 0, 0, 0.2); }");

  layout->addWidget(textButton, 1);
  layout->addWidget(removeButton, 0, Qt::AlignRight);

  widget->setLayout(layout);
  widget->setCursor(Qt::PointingHandCursor);

  if (isBuffer) {
    connect(textButton, &QPushButton::clicked, this,
            &MenuManager::handleBufferWidgetClicked);
    connect(removeButton, &QToolButton::clicked, this,
            &MenuManager::handleBufferWidgetRemoveClicked);
  } else {
    connect(textButton, &QPushButton::clicked, this,
            &MenuManager::handleRecentWidgetClicked);
    connect(removeButton, &QToolButton::clicked, this,
            &MenuManager::handleRecentWidgetRemoveClicked);
  }

  return widget;
}

void MenuManager::updateRecentMenu(const QStringList &recentFiles) {
  if (!recentMenu)
    return;
  recentMenu->clear();

  if (recentFiles.isEmpty()) {
    QAction *emptyAction = recentMenu->addAction("(No Recent Files)");
    emptyAction->setEnabled(false);
  } else {
    using namespace fa;
    QIcon fileIcon = awesome ? awesome->icon(fa_solid, fa_file_lines) : QIcon();

    for (const QString &filePath : recentFiles) {
      QWidgetAction *widgetAction = new QWidgetAction(recentMenu);

      QWidget *itemWidget = createMenuItemWidget(QFileInfo(filePath).fileName(),
                                                 filePath, false, fileIcon);
      widgetAction->setDefaultWidget(itemWidget);
      recentMenu->addAction(widgetAction);
    }
  }

  if (!recentFiles.isEmpty()) {
    recentMenu->addSeparator();
    recentMenu->addAction(clearRecentAction);
    clearRecentAction->setEnabled(true);
  }
}

void MenuManager::updateBuffersMenu(int currentTab) {
  if (!buffersMenu || !mainTabWidget)
    return;
  buffersMenu->clear();

  if (mainTabWidget->count() == 0) {
    QAction *emptyAction = buffersMenu->addAction("(No Open Buffers)");
    emptyAction->setEnabled(false);
  } else {
    using namespace fa;
    QIcon bufferIcon =
        awesome ? awesome->icon(fa_solid, fa_window_restore) : QIcon();

    for (int i = 0; i < mainTabWidget->count(); ++i) {
      QString tabTitle = mainTabWidget->tabText(i);

      if (tabTitle.endsWith(" *")) {
        tabTitle.chop(2);
      }

      QWidgetAction *widgetAction = new QWidgetAction(buffersMenu);

      QWidget *itemWidget = createMenuItemWidget(tabTitle, i, true, bufferIcon);

      if (i == currentTab) {

        itemWidget->setStyleSheet(
            "QWidget { background-color: palette(highlight); }");

        QPushButton *button = itemWidget->findChild<QPushButton *>();
        if (button)
          button->setStyleSheet(button->styleSheet() +
                                " color: palette(highlighted-text);");
      }

      widgetAction->setDefaultWidget(itemWidget);
      buffersMenu->addAction(widgetAction);
    }
  }
}

void MenuManager::handleRecentWidgetClicked() {

  QPushButton *button = qobject_cast<QPushButton *>(sender());
  if (button) {

    emit openRecentFileRequested(button->property("itemData").toString());

    QWidget *parent = button->parentWidget();
    if (parent) {
      QWidgetAction *action = qobject_cast<QWidgetAction *>(parent->parent());

      QObject *menuParent = this;
      if (recentMenu)
        recentMenu->close();
    }
  }
}

void MenuManager::handleRecentWidgetRemoveClicked() {

  QToolButton *button = qobject_cast<QToolButton *>(sender());
  if (button) {

    emit removeRecentFileRequested(button->property("itemData").toString());
  }
}

void MenuManager::handleBufferWidgetClicked() {

  QPushButton *button = qobject_cast<QPushButton *>(sender());
  if (button) {
    emit switchToBufferRequested(button->property("itemData").toInt());
    if (buffersMenu)
      buffersMenu->close();
  }
}

void MenuManager::handleBufferWidgetRemoveClicked() {

  QToolButton *button = qobject_cast<QToolButton *>(sender());
  if (button) {
    emit closeBufferRequested(button->property("itemData").toInt());
  }
}

void MenuManager::handleClearRecentTriggered() {
  emit clearRecentFilesRequested();
}

} // namespace Jino::App
