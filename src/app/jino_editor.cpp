#include "app/jino_editor.hpp"
#include "QtAwesome.h"
#include "core/constants.hpp"
#include "editor/editor_widget.hpp"
#include "editor/vim/vim_handler.hpp"
#include "editor/vim/vim_modes.hpp"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontInfo>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPoint>
#include <QSettings>
#include <QShortcut>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTextCodec>
#include <QTextDocument>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <chrono>
#include <random>
#include <vector>

namespace Jino::App {

namespace {
QString getRandomAngelNameInternal() {
  if (Constants::ANGEL_NAMES.empty())
    return QStringLiteral("DefaultName");
  thread_local static std::mt19937 generator(
      std::chrono::system_clock::now().time_since_epoch().count());
  thread_local static std::uniform_int_distribution<size_t> distribution(
      0, Constants::ANGEL_NAMES.size() - 1);
  return Constants::ANGEL_NAMES[distribution(generator)];
}
} // namespace

JinoEditor::JinoEditor(QWidget *parent, const QString &workspaceName,
                       fa::QtAwesome *awesomePtr)
    : QMainWindow(parent), currentWorkspaceName(workspaceName),
      awesome(awesomePtr) {
  setupInitialUi();
}

JinoEditor::~JinoEditor() {
  delete statusBarManager;
  delete menuManager;
}

void JinoEditor::setupInitialUi() {

  if (awesome) {
    using namespace fa;
    setWindowIcon(awesome->icon(fa_solid, fa_feather_pointed));
  } else {
    setWindowIcon(QIcon(":/icons/jino_icon_256x256.png"));
  }
  tabWidget = new QTabWidget(this);
  tabWidget->setTabsClosable(false);
  tabWidget->setMovable(true);
  tabWidget->setTabPosition(QTabWidget::South);
  setCentralWidget(tabWidget);
  tabWidget->tabBar()->setExpanding(false);
  resize(Constants::DEFAULT_WINDOW_WIDTH, Constants::DEFAULT_WINDOW_HEIGHT);
  sessionTimer.start();
  elapsedTimerClock = new QTimer(this);
  loadFont();
  menuManager = new MenuManager(this, awesome);
  menuManager->setupMenusAndActions(menuBar(), tabWidget);
  statusBarManager = new StatusBarManager(this, currentWorkspaceName, awesome);
  statusBarManager->setupUI();

  connect(menuManager, &MenuManager::openRecentFileRequested, this,
          &JinoEditor::handleOpenRecentFileRequested);
  connect(menuManager, &MenuManager::clearRecentFilesRequested, this,
          &JinoEditor::handleClearRecentFilesRequested);
  connect(menuManager, &MenuManager::switchToBufferRequested, this,
          &JinoEditor::handleSwitchToBufferRequested);
  connect(menuManager, &MenuManager::showBuffersMenuRequested, this,
          &JinoEditor::showBuffersMenu);
  connect(menuManager, &MenuManager::showRecentMenuRequested, this,
          &JinoEditor::showRecentMenu);
  connect(menuManager, &MenuManager::removeRecentFileRequested, this,
          &JinoEditor::handleRemoveRecentFileRequested);
  connect(menuManager, &MenuManager::closeBufferRequested, this,
          &JinoEditor::handleCloseBufferRequested);

  connect(statusBarManager, &StatusBarManager::editorModeChangeRequested, this,
          &JinoEditor::handleStatusBarEditorModeChangeRequested);
  connect(statusBarManager, &StatusBarManager::closeCurrentTabRequested, this,
          &JinoEditor::handleStatusBarCloseRequested);
  connect(statusBarManager, &StatusBarManager::resetZoomRequested, this,
          &JinoEditor::handleResetZoomRequested);

  connect(statusBarManager, &StatusBarManager::goToLineRequested, this,
          &JinoEditor::handleGoToLineRequested);
  connect(statusBarManager, &StatusBarManager::goToColumnRequested, this,
          &JinoEditor::handleGoToColumnRequested);
  connect(statusBarManager, &StatusBarManager::saveRequested, this,
          &JinoEditor::handleSaveRequested);

  connect(statusBarManager, &StatusBarManager::goToTopRequested, this,
          &JinoEditor::handleGoToTopRequested);
  connect(statusBarManager, &StatusBarManager::goToCenterRequested, this,
          &JinoEditor::handleGoToCenterRequested);
  connect(statusBarManager, &StatusBarManager::goToBottomRequested, this,
          &JinoEditor::handleGoToBottomRequested);
  connect(statusBarManager, &StatusBarManager::pasteAndReplaceRequested, this,
          &JinoEditor::handlePasteAndReplaceRequested);
  connect(statusBarManager, &StatusBarManager::clearFileRequested, this,
          &JinoEditor::handleClearFileRequested);

  connect(menuManager->newTabAction, &QAction::triggered, this,
          &JinoEditor::onNewTabAction);
  connect(menuManager->openAction, &QAction::triggered, this,
          &JinoEditor::onOpenAction);
  connect(menuManager->saveAction, &QAction::triggered, this,
          &JinoEditor::onSaveAction);
  connect(menuManager->saveAsAction, &QAction::triggered, this,
          &JinoEditor::onSaveAsAction);
  connect(menuManager->closeTabAction, &QAction::triggered, this,
          &JinoEditor::onCloseTabAction);
  connect(menuManager->quitAction, &QAction::triggered, this, &QWidget::close);
  connect(menuManager->undoAction, &QAction::triggered, this,
          &JinoEditor::onUndoAction);
  connect(menuManager->redoAction, &QAction::triggered, this,
          &JinoEditor::onRedoAction);
  connect(menuManager->cutAction, &QAction::triggered, this,
          &JinoEditor::onCutAction);
  connect(menuManager->copyAction, &QAction::triggered, this,
          &JinoEditor::onCopyAction);
  connect(menuManager->pasteAction, &QAction::triggered, this,
          &JinoEditor::onPasteAction);
  connect(menuManager->selectAllAction, &QAction::triggered, this,
          &JinoEditor::onSelectAllAction);

  connect(tabWidget, &QTabWidget::currentChanged, this,
          &JinoEditor::handleCurrentTabChanged);
  connect(QApplication::clipboard(), &QClipboard::dataChanged, this,
          &JinoEditor::updateUiStates);
  connect(elapsedTimerClock, &QTimer::timeout, this,
          &JinoEditor::updateElapsedTime);

  setupShortcuts();
  statusBar()->showMessage(Constants::STATUS_READY, 2000);
  elapsedTimerClock->start(1000);
  loadSettings();
  initialTabCreated = false;
  updateUiStates();
  updateWindowTitle();
}

void JinoEditor::loadSettings() {
  QSettings s;
  recentFilesList =
      s.value(Constants::SETTINGS_KEY_RECENT_FILES).toStringList();
  if (menuManager)
    menuManager->updateRecentMenu(recentFilesList);
}
void JinoEditor::saveSettings() {
  QSettings s;
  s.setValue(Constants::SETTINGS_KEY_RECENT_FILES, recentFilesList);
}
void JinoEditor::loadFont() {
  const int id =
      QFontDatabase::addApplicationFont(Constants::FONT_RESOURCE_PATH);
  if (id == -1)
    statusBar()->showMessage(
        Constants::STATUS_FONT_LOAD_FAILED.arg(Constants::FONT_RESOURCE_PATH),
        5000);
}

void JinoEditor::setupEditorConnections(EditorWidget *editor) {

  if (!editor || !statusBarManager)
    return;
  connect(editor, &EditorWidget::saveFileRequested, this,
          &JinoEditor::onSaveAction);
  connect(editor, &EditorWidget::saveFileAsRequested, this,
          &JinoEditor::onSaveAsAction);
  connect(editor->document(), &QTextDocument::modificationChanged, this,
          &JinoEditor::handleModificationChanged);
  connect(editor->document(), &QTextDocument::contentsChanged, this,
          &JinoEditor::updateUiStates);
  connect(editor, &QTextEdit::copyAvailable, this, &JinoEditor::updateUiStates);
  connect(editor, &QTextEdit::undoAvailable, this, &JinoEditor::updateUiStates);
  connect(editor, &QTextEdit::redoAvailable, this, &JinoEditor::updateUiStates);
  connect(editor, &QTextEdit::selectionChanged, this,
          &JinoEditor::updateUiStates);
  connect(editor, &QTextEdit::cursorPositionChanged, this,
          &JinoEditor::updateUiStates);
  connect(editor, &EditorWidget::vimModeChanged, this,
          &JinoEditor::updateUiStates);
  connect(editor->document(), &QTextDocument::modificationChanged, this,
          [this, editor]() {
            int index = tabWidget->indexOf(editor);
            if (index != -1) {
              updateTabToolTip(index);
              updateTabTitle(index);
            }
          });
  connect(editor, &EditorWidget::zoomPercentChanged, statusBarManager,
          &StatusBarManager::updateZoomDisplay);
}

void JinoEditor::disconnectEditorSignals(EditorWidget *editor) {

  if (!editor || !statusBarManager)
    return;
  disconnect(editor, &EditorWidget::zoomPercentChanged, statusBarManager,
             &StatusBarManager::updateZoomDisplay);
}

void JinoEditor::setupShortcuts() {

  auto addS2S = [&](const QKeySequence &s, auto slot) {
    auto sc = new QShortcut(s, this);
    connect(sc, &QShortcut::activated, this, slot);
  };
  addS2S(Constants::KB_ALT_LEFT, &JinoEditor::prevTab);
  addS2S(Constants::KB_ALT_RIGHT, &JinoEditor::nextTab);
  addS2S(Constants::KB_ALT_B, &JinoEditor::showBuffersMenu);
  addS2S(Constants::KB_ALT_R, &JinoEditor::showRecentMenu);
  addS2S(Constants::KB_CTRL_PLUS, &JinoEditor::onZoomInAction);
  addS2S(Constants::KB_CTRL_EQUAL, &JinoEditor::onZoomInAction);
  addS2S(Constants::KB_CTRL_MINUS, &JinoEditor::onZoomOutAction);
}

void JinoEditor::updateUiStates() {

  EditorWidget *editor = currentEditorWidget();
  bool editorExists = (editor != nullptr);
  bool hasSelection = editorExists && editor->textCursor().hasSelection();
  bool undoAvailable = editorExists && editor->document()->isUndoAvailable();
  bool redoAvailable = editorExists && editor->document()->isRedoAvailable();
  bool pasteAvailable = QApplication::clipboard()->mimeData()->hasText();
  if (statusBarManager)
    statusBarManager->updateTopStatusBar(editor);
  if (menuManager) {
    menuManager->updateActionStates(editorExists, hasSelection, undoAvailable,
                                    redoAvailable, pasteAvailable);
    menuManager->updateBuffersMenu(tabWidget->currentIndex());
  }
  updateWindowTitle();
}

void JinoEditor::updateElapsedTime() {
  if (statusBarManager) {
    qint64 ms = sessionTimer.elapsed();
    qint64 s = (ms / 1000) % 60;
    qint64 m = (ms / (1000 * 60)) % 60;
    qint64 h = (ms / (1000 * 60 * 60));
    QString t = QString("%1:%2:%3")
                    .arg(h, 1, 10, QChar('0'))
                    .arg(m, 2, 10, QChar('0'))
                    .arg(s, 2, 10, QChar('0'));
    statusBarManager->updateTimeDisplay(t);
  }
}
void JinoEditor::handleModificationChanged(bool) { updateUiStates(); }
void JinoEditor::applyEditorFont(EditorWidget *e) {
  if (!e)
    return;
  QFont f(Constants::FONT_FAMILY, Constants::FONT_SIZE_PT);
  e->setFont(f);
}

QString JinoEditor::getRandomAngelName() const {
  return getRandomAngelNameInternal();
}
void JinoEditor::setBaseNameForEditor(EditorWidget *editor,
                                      const QString &name) {
  if (!editor)
    return;
  QWidget *ew = editor;
  editorBaseNames[ew] = name;
}
QString JinoEditor::getBaseNameForEditor(EditorWidget *editor) const {
  if (!editor)
    return QString();
  QWidget *ew = editor;
  return editorBaseNames.value(ew, getRandomAngelName());
}

void JinoEditor::addRecentFile(const QString &filePath) {
  if (filePath.isEmpty())
    return;
  QFileInfo fi(filePath);
  if (fi.fileName().startsWith("." + currentWorkspaceName + "_") &&
      fi.dir().path() == Constants::DEFAULT_NOTES_DIR)
    return;
  recentFilesList.removeAll(filePath);
  recentFilesList.prepend(filePath);
  while (recentFilesList.size() > Constants::MAX_RECENT_FILES)
    recentFilesList.removeLast();
  if (menuManager)
    menuManager->updateRecentMenu(recentFilesList);
}
void JinoEditor::handleClearRecentFilesRequested() {
  recentFilesList.clear();
  if (menuManager)
    menuManager->updateRecentMenu(recentFilesList);
  saveSettings();
}

void JinoEditor::onNewTabAction() { newTab(); }
void JinoEditor::onOpenAction() { openFile(); }
void JinoEditor::onSaveAction() { saveFile(); }
void JinoEditor::onSaveAsAction() { saveFileAs(); }
void JinoEditor::onCloseTabAction() { closeCurrentTab(); }
void JinoEditor::onUndoAction() { undoEdit(); }
void JinoEditor::onRedoAction() { redoEdit(); }
void JinoEditor::onCutAction() { cutEdit(); }
void JinoEditor::onCopyAction() { copyEdit(); }
void JinoEditor::onPasteAction() { pasteEdit(); }
void JinoEditor::onSelectAllAction() { selectAllEdit(); }

void JinoEditor::handleOpenRecentFileRequested(const QString &filePath) {
  openSingleFile(filePath);
}
void JinoEditor::handleSwitchToBufferRequested(int index) {
  if (index >= 0 && index < tabWidget->count())
    tabWidget->setCurrentIndex(index);
}
void JinoEditor::showBuffersMenu() {
  QList<QAction *> acts = menuBar()->actions();
  for (auto a : acts)
    if (a->text() == "&Buffers" && a->menu()) {
      if (menuManager)
        menuManager->updateBuffersMenu(tabWidget->currentIndex());
      auto *w = menuBar()->findChild<QWidget *>();
      QPoint p = w ? w->mapToGlobal({0, w->height()})
                   : mapToGlobal({0, menuBar()->height()});
      a->menu()->popup(p);
      break;
    }
}
void JinoEditor::showRecentMenu() {
  QList<QAction *> acts = menuBar()->actions();
  for (auto a : acts)
    if (a->text() == "&Recent" && a->menu()) {
      if (menuManager)
        menuManager->updateRecentMenu(recentFilesList);
      auto *w = menuBar()->findChild<QWidget *>();
      QPoint p = w ? w->mapToGlobal({0, w->height()})
                   : mapToGlobal({0, menuBar()->height()});
      a->menu()->popup(p);
      break;
    }
}

void JinoEditor::onZoomInAction() {
  if (EditorWidget *e = currentEditorWidget())
    e->zoomIn(Constants::EDITOR_ZOOM_STEP);
}
void JinoEditor::onZoomOutAction() {
  if (EditorWidget *e = currentEditorWidget())
    e->zoomOut(Constants::EDITOR_ZOOM_STEP);
}
void JinoEditor::handleResetZoomRequested() {
  if (EditorWidget *e = currentEditorWidget())
    e->resetZoom();
}

void JinoEditor::handleStatusBarEditorModeChangeRequested() {
  EditorWidget *e = currentEditorWidget();
  if (!e)
    return;
  QMenu mm(this);
  mm.setObjectName("EditorModeSelectionMenu");
  for (const auto &m : Constants::getAllEditorModes()) {
    QAction *a = mm.addAction(Constants::editorModeToString(m));
    a->setCheckable(true);
    a->setChecked(e->editorMode() == m);
    connect(a, &QAction::triggered, this,
            [this, m]() { this->changeEditorMode(m); });
    if (awesome)
      a->setIcon(awesome->icon(fa::fa_solid, Jino::App::getIconForFileType(m)));
  }
  QWidget *mw = statusBarManager->getEditorModeWidget();
  if (mw)
    mm.exec(mw->mapToGlobal({0, mw->height()}));
  else
    mm.exec(QCursor::pos());
}
void JinoEditor::handleStatusBarCloseRequested() { closeCurrentTab(); }
void JinoEditor::changeEditorMode(Jino::Constants::EditorFileType newMode) {
  if (EditorWidget *e = currentEditorWidget()) {
    e->setEditorMode(newMode);
    updateUiStates();
  }
}

void JinoEditor::handleGoToLineRequested() {
  EditorWidget *editor = currentEditorWidget();
  if (!editor)
    return;
  bool ok;
  int currentLine = editor->textCursor().blockNumber() + 1;
  int maxLine = editor->document()->blockCount();
  int line = QInputDialog::getInt(this, Constants::INPUT_GOTO_LINE_TITLE,
                                  Constants::INPUT_GOTO_LINE_LABEL, currentLine,
                                  1, maxLine, 1, &ok);
  if (ok) {
    editor->goToLine(line);
  }
}

void JinoEditor::handleGoToColumnRequested() {
  EditorWidget *editor = currentEditorWidget();
  if (!editor)
    return;
  bool ok;
  int currentCol = editor->textCursor().columnNumber() + 1;

  int maxCol = editor->textCursor().block().length();
  if (maxCol < 1)
    maxCol = 1;
  int col = QInputDialog::getInt(this, Constants::INPUT_GOTO_COL_TITLE,
                                 Constants::INPUT_GOTO_COL_LABEL, currentCol, 1,
                                 maxCol, 1, &ok);
  if (ok) {
    editor->goToColumn(col);
  }
}

void JinoEditor::handleSaveRequested() { saveFile(); }

void JinoEditor::handleGoToTopRequested() {
  if (EditorWidget *editor = currentEditorWidget())
    editor->goToTop();
}

void JinoEditor::handleGoToCenterRequested() {
  if (EditorWidget *editor = currentEditorWidget())
    editor->goToCenter();
}

void JinoEditor::handleGoToBottomRequested() {
  if (EditorWidget *editor = currentEditorWidget())
    editor->goToBottom();
}

void JinoEditor::handlePasteAndReplaceRequested() {
  if (EditorWidget *editor = currentEditorWidget())
    editor->pasteAndReplace();
}

void JinoEditor::handleClearFileRequested() {
  if (EditorWidget *editor = currentEditorWidget())
    editor->clearAll();
}

void JinoEditor::handleRemoveRecentFileRequested(const QString &filePath) {
  if (recentFilesList.removeOne(filePath)) {
    if (menuManager)
      menuManager->updateRecentMenu(recentFilesList);
    saveSettings();
  }
}
void JinoEditor::handleCloseBufferRequested(int index) { maybeCloseTab(index); }

void JinoEditor::handleEditorZoomChanged(int percent) {
  if (statusBarManager)
    statusBarManager->updateZoomDisplay(percent);
}

void JinoEditor::handleCurrentTabChanged(int index) {
  Q_UNUSED(index);
  if (currentlyConnectedEditor) {
    disconnectEditorSignals(currentlyConnectedEditor);
    currentlyConnectedEditor = nullptr;
  }
  EditorWidget *currentEditor = currentEditorWidget();
  if (currentEditor) {
    connect(currentEditor, &EditorWidget::zoomPercentChanged, statusBarManager,
            &StatusBarManager::updateZoomDisplay);
    currentlyConnectedEditor = currentEditor;
    if (statusBarManager)
      statusBarManager->updateTopStatusBar(currentEditor);
  } else {
    if (statusBarManager)
      statusBarManager->updateTopStatusBar(nullptr);
  }
  updateWindowTitle();
  if (menuManager)
    menuManager->updateBuffersMenu(tabWidget->currentIndex());
}

void JinoEditor::newTab() {
  if (tabWidget->count() >= Constants::MAX_TABS_PER_WORKSPACE) {
    statusBar()->showMessage(
        Constants::STATUS_MAX_TABS.arg(Constants::MAX_TABS_PER_WORKSPACE),
        3000);
    return;
  }
  auto editor = new EditorWidget;
  applyEditorFont(editor);
  editor->setEditorMode(Constants::EditorFileType::Text);
  QString baseName = getRandomAngelName();
  setBaseNameForEditor(editor, baseName);
  const int newTabIndex = tabWidget->addTab(editor, "");
  tabWidget->setCurrentIndex(newTabIndex);
  setCurrentFile(editor, QString());
  setupEditorConnections(editor);
  updateUiStates();
  initialTabCreated = true;
}
void JinoEditor::openFile() {
  const QString dir = recentFilesList.isEmpty()
                          ? Constants::DEFAULT_NOTES_DIR
                          : QFileInfo(recentFilesList.first()).path();
  const QString p = QFileDialog::getOpenFileName(this, "Open File", dir);
  if (!p.isEmpty())
    openSingleFile(p);
}
void JinoEditor::openFilesFromCli(const QStringList &filePaths) {
  bool firstFile = true;
  for (const QString &path : filePaths) {
    if (QFileInfo::exists(path)) {
      if (firstFile && !initialTabCreated && tabWidget->count() == 0) {
        newTab();
      }
      openSingleFile(path);
      firstFile = false;
    } else {
      qWarning() << "File not found (CLI argument):" << path;
    }
  }
  if (tabWidget->count() == 0 && !initialTabCreated) {
    newTab();
  }
}
void JinoEditor::openSingleFile(const QString &filePath) {
  if (filePath.isEmpty())
    return;
  for (int i = 0; i < tabWidget->count(); ++i) {
    if (editorWidgetForIndex(i) &&
        getCurrentFile(editorWidgetForIndex(i)) == filePath) {
      tabWidget->setCurrentIndex(i);
      addRecentFile(filePath);
      return;
    }
  }
  EditorWidget *ce = currentEditorWidget();
  const bool reuse = initialTabCreated && tabWidget->count() == 1 && ce &&
                     ce->document()->isEmpty() &&
                     !ce->document()->isModified() &&
                     getCurrentFile(ce).isEmpty();
  if (!reuse && tabWidget->count() >= Constants::MAX_TABS_PER_WORKSPACE) {
    statusBar()->showMessage(
        Constants::STATUS_MAX_TABS.arg(Constants::MAX_TABS_PER_WORKSPACE),
        3000);
    return;
  }
  QString content;
  if (loadFileContent(filePath, content)) {
    EditorWidget *etu;
    if (reuse) {
      etu = ce;
      qInfo() << "Reusing empty tab for:" << filePath;
    } else {
      newTab();
      etu = currentEditorWidget();
      if (!etu)
        return;
      qInfo() << "Created new tab for:" << filePath;
    }
    etu->setPlainText(content);
    applyEditorFont(etu);
    setCurrentFile(etu, filePath);
    addRecentFile(filePath);
    statusBar()->showMessage(
        Constants::STATUS_FILE_OPENED.arg(QFileInfo(filePath).fileName()),
        3000);
    updateUiStates();
  } else {
    recentFilesList.removeAll(filePath);
    if (menuManager)
      menuManager->updateRecentMenu(recentFilesList);
  }
}

bool JinoEditor::loadFileContent(const QString &path, QString &content) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(
        this, Constants::APP_NAME,
        Constants::STATUS_FILE_OPEN_FAILED.arg(QFileInfo(path).fileName()));
    statusBar()->showMessage(Constants::STATUS_FILE_OPEN_FAILED.arg(path),
                             5000);
    return false;
  }
  QTextStream in(&file);
  in.setCodec(Constants::DEFAULT_FILE_ENCODING.constData());
  QApplication::setOverrideCursor(Qt::WaitCursor);
  content = in.readAll();
  QApplication::restoreOverrideCursor();
  file.close();
  return true;
}
bool JinoEditor::saveFileLogic(const QString &p) {
  EditorWidget *e = currentEditorWidget();
  if (!e || p.isEmpty())
    return false;
  QFileInfo fi(p);
  QDir d = fi.dir();
  if (!d.exists() && !d.mkpath("."))
    return false;
  QFile f(p);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    QMessageBox::warning(this, Constants::APP_NAME,
                         Constants::STATUS_FILE_SAVE_FAILED.arg(fi.fileName()));
    statusBar()->showMessage(Constants::STATUS_FILE_SAVE_FAILED.arg(p), 5000);
    return false;
  }
  QTextStream o(&f);
  o.setCodec(Constants::DEFAULT_FILE_ENCODING.constData());
  QApplication::setOverrideCursor(Qt::WaitCursor);
  o << e->toPlainText();
  QApplication::restoreOverrideCursor();
  o.flush();
  f.close();
  if (f.error() != QFile::NoError)
    return false;
  setCurrentFile(e, p);
  addRecentFile(p);
  return true;
}
bool JinoEditor::saveFile() {
  EditorWidget *e = currentEditorWidget();
  if (!e)
    return false;
  QString cp = getCurrentFile(e);
  if (cp.isEmpty() ||
      (cp.startsWith("." + currentWorkspaceName + "_") &&
       QFileInfo(cp).dir().path() == Constants::DEFAULT_NOTES_DIR)) {
    return saveFileAs();
  } else {
    if (saveFileLogic(cp)) {
      statusBar()->showMessage(
          Constants::STATUS_FILE_SAVED.arg(QFileInfo(cp).fileName()), 3000);
      return true;
    }
    return false;
  }
}
bool JinoEditor::saveFileAs() {
  EditorWidget *e = currentEditorWidget();
  if (!e)
    return false;
  QString cp = getCurrentFile(e);
  QString ip;
  QString bn = getBaseNameForEditor(e);
  QString dsn = Constants::DEFAULT_NOTES_DIR + "/" +
                (cp.isEmpty() ? bn : QFileInfo(cp).completeBaseName()) + ".txt";
  if (cp.isEmpty()) {
    ip = dsn;
  } else {
    QFileInfo fi(cp);
    if (fi.fileName().startsWith("." + currentWorkspaceName + "_") &&
        fi.dir().path() == Constants::DEFAULT_NOTES_DIR) {
      ip = dsn;
    } else {
      ip = cp;
    }
  }
  const QString p = QFileDialog::getSaveFileName(this, "Save File As", ip);
  if (p.isEmpty())
    return false;
  if (saveFileLogic(p)) {
    statusBar()->showMessage(
        Constants::STATUS_FILE_SAVED.arg(QFileInfo(p).fileName()), 3000);
    return true;
  }
  return false;
}
bool JinoEditor::autoSaveBufferOnClose(EditorWidget *editor) {
  if (!editor || !editor->document() || !editor->document()->isModified())
    return false;
  QString cp = getCurrentFile(editor);
  QString sp;
  bool isU = cp.isEmpty();
  QFileInfo ci(cp);
  bool isH = !isU &&
             ci.fileName().startsWith("." + currentWorkspaceName + "_") &&
             ci.dir().path() == Constants::DEFAULT_NOTES_DIR;
  if (isU || isH) {
    QString bn = getBaseNameForEditor(editor);
    sp = Constants::DEFAULT_NOTES_DIR + "/." + currentWorkspaceName + "_" + bn +
         ".txt";
    qInfo() << "Autosaving:" << bn << "to" << sp;
  } else {
    sp = cp;
    qInfo() << "Autosaving:" << sp;
  }
  auto iS = [&](const QString &p, const QString &c) -> bool {
    QFileInfo fi(p);
    QDir d = fi.dir();
    if (!d.exists() && !d.mkpath("."))
      return false;
    QFile f(p);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
      return false;
    QTextStream o(&f);
    o.setCodec(Constants::DEFAULT_FILE_ENCODING.constData());
    o << c;
    o.flush();
    f.close();
    return f.error() == QFile::NoError;
  };
  if (iS(sp, editor->toPlainText())) {
    QWidget *ep = editor;
    editorFilePaths[ep] = sp;
    editor->document()->setModified(false);
    if (editor == currentEditorWidget()) {
      int idx = tabWidget->currentIndex();
      updateTabTitle(idx);
      updateTabToolTip(idx);
      updateWindowTitle();
    }
    return true;
  } else {
    qWarning() << "Autosave failed:" << sp;
    statusBar()->showMessage(
        QString("Autosave failed: %1").arg(QFileInfo(sp).fileName()), 5000);
    return false;
  }
}

void JinoEditor::closeCurrentTab() {
  const int ci = tabWidget->currentIndex();
  if (ci >= 0)
    maybeCloseTab(ci);
}
void JinoEditor::maybeCloseTab(int index) {
  if (index < 0 || index >= tabWidget->count())
    return;
  EditorWidget *e = editorWidgetForIndex(index);
  QWidget *w = tabWidget->widget(index);
  if (!e || !w)
    return;
  if (e->document()->isModified())
    autoSaveBufferOnClose(e);
  EditorWidget *ce = editorWidgetForIndex(index);
  cleanupEditorData(w);
  int ni = -1;
  if (tabWidget->count() > 1)
    ni = (index == tabWidget->count() - 1) ? index - 1 : index;
  tabWidget->removeTab(index);
  w->deleteLater();
  if (ce == currentlyConnectedEditor)
    currentlyConnectedEditor = nullptr;
  if (ni != -1)
    tabWidget->setCurrentIndex(ni);
  else {
    initialTabCreated = false;
    handleCurrentTabChanged(-1);
  }
  updateUiStates();
}
void JinoEditor::closeEvent(QCloseEvent *event) {
  for (int i = tabWidget->count() - 1; i >= 0; --i)
    autoSaveBufferOnClose(editorWidgetForIndex(i));
  saveSettings();
  event->accept();
}
void JinoEditor::cleanupEditorData(QWidget *editorWidget) {
  if (!editorWidget)
    return;
  editorFilePaths.remove(editorWidget);
  editorBaseNames.remove(editorWidget);
}

EditorWidget *JinoEditor::currentEditorWidget() const {
  return qobject_cast<EditorWidget *>(tabWidget->currentWidget());
}
EditorWidget *JinoEditor::editorWidgetForIndex(int index) const {
  if (index < 0 || index >= tabWidget->count())
    return nullptr;
  return qobject_cast<EditorWidget *>(tabWidget->widget(index));
}

void JinoEditor::setCurrentFile(EditorWidget *e, const QString &p) {
  if (!e)
    return;
  QWidget *ew = e;
  editorFilePaths[ew] = p;
  if (!p.isEmpty()) {
    QFileInfo fi(p);
    if (!(fi.fileName().startsWith("." + currentWorkspaceName + "_") &&
          fi.dir().path() == Constants::DEFAULT_NOTES_DIR))
      editorBaseNames.remove(ew);
    else if (!editorBaseNames.contains(ew)) {
      QString bn = fi.completeBaseName();
      if (bn.startsWith("." + currentWorkspaceName + "_")) {
        bn = bn.mid(QString("." + currentWorkspaceName + "_").length());
        setBaseNameForEditor(e, bn);
      }
    }
  } else {
    if (!editorBaseNames.contains(ew))
      setBaseNameForEditor(e, getRandomAngelName());
  }
  e->document()->setModified(false);
  QString ext = QFileInfo(p).suffix().toLower();
  if (ext == "org")
    e->setEditorMode(Constants::EditorFileType::Org);
  else if (ext == "md")
    e->setEditorMode(Constants::EditorFileType::Markdown);
  else
    e->setEditorMode(Constants::EditorFileType::Text);
  const int ti = tabWidget->indexOf(ew);
  if (ti != -1) {
    updateTabTitle(ti);
    updateTabToolTip(ti);
  }
  updateUiStates();
}

QString JinoEditor::getCurrentFile(EditorWidget *e) const {
  if (!e)
    return QString();
  QWidget *ew = e;
  return editorFilePaths.value(ew, QString());
}

void JinoEditor::updateTabTitle(int index) {
  if (index < 0 || index >= tabWidget->count())
    return;
  EditorWidget *e = editorWidgetForIndex(index);
  if (!e)
    return;
  const QString p = getCurrentFile(e);
  QString titleText;
  QString baseName = getBaseNameForEditor(e);
  bool isAngelBuffer = false;
  if (p.isEmpty()) {
    titleText = baseName;
    isAngelBuffer = true;
  } else {
    QFileInfo fi(p);
    QString ehn = "." + currentWorkspaceName + "_" + baseName + ".txt";
    if (!baseName.isEmpty() && fi.fileName() == ehn &&
        fi.dir().path() == Constants::DEFAULT_NOTES_DIR) {
      titleText = baseName;
      isAngelBuffer = true;
    } else {
      titleText = fi.fileName();
    }
  }

  const QString t = titleText + (e->document()->isModified() ? " *" : "");
  tabWidget->setTabText(index, t);

  if (isAngelBuffer && awesome) {
    tabWidget->setTabIcon(index, awesome->icon(fa::fa_solid, fa::fa_ghost));
  } else {

    tabWidget->setTabIcon(index, QIcon());
  }
}
void JinoEditor::updateWindowTitle() {
  QString bt = Constants::APP_NAME;
  EditorWidget *e = currentEditorWidget();
  if (e) {
    const QString cp = getCurrentFile(e);
    QString fp;
    QString bn = getBaseNameForEditor(e);
    if (cp.isEmpty()) {
      fp = bn;
    } else {
      QFileInfo fi(cp);
      QString ehn = "." + currentWorkspaceName + "_" + bn + ".txt";
      if (!bn.isEmpty() && fi.fileName() == ehn &&
          fi.dir().path() == Constants::DEFAULT_NOTES_DIR) {
        fp = bn;
      } else {
        fp = fi.fileName();
      }
    }
    setWindowTitle(QString("%1[*] - %2").arg(fp).arg(bt));
    setWindowModified(e->document()->isModified());
  } else {
    setWindowTitle(bt);
    setWindowModified(false);
  }
}
void JinoEditor::updateTabToolTip(int index) {
  if (index < 0 || index >= tabWidget->count())
    return;
  EditorWidget *ed = editorWidgetForIndex(index);
  if (!ed)
    return;
  QString fp = getCurrentFile(ed);
  QString tt;
  if (fp.isEmpty()) {
    tt = Constants::TOOLTIP_BUFFER_INFO_FMT.arg(getBaseNameForEditor(ed));
  } else {
    QFileInfo fi(fp);
    QString bn = getBaseNameForEditor(ed);
    QString ehn = "." + currentWorkspaceName + "_" + bn + ".txt";
    if (!bn.isEmpty() && fi.fileName() == ehn &&
        fi.dir().path() == Constants::DEFAULT_NOTES_DIR) {
      tt = Constants::TOOLTIP_AUTOSAVE_INFO_FMT.arg(bn).arg(fp);
    } else if (fi.exists()) {
      tt = formatFileInfoToolTip(fp);
    } else {
      tt = Constants::TOOLTIP_FILE_INFO_FMT.arg(fp).arg("N/A").arg("N/A").arg(
          "N/A");
    }
  }
  if (ed->document()->isModified())
    tt += "\\n(Modified)";
  tabWidget->setTabToolTip(index, tt);
}
QString JinoEditor::formatFileInfoToolTip(const QString &filePath) const {
  QFileInfo fi(filePath);
  qint64 sz = fi.size();
  QString ss;
  if (sz < 1024)
    ss = QString::number(sz) + " B";
  else if (sz < 1024 * 1024)
    ss = QString::number(sz / 1024.0, 'f', 1) + " KiB";
  else
    ss = QString::number(sz / (1024.0 * 1024.0), 'f', 1) + " MiB";
  QFile::Permissions ps = fi.permissions();
  QString pss;
  pss += (ps & QFile::ReadUser) ? "r" : "-";
  pss += (ps & QFile::WriteUser) ? "w" : "-";
  pss += (ps & QFile::ExeUser) ? "x" : "-";
  pss += (ps & QFile::ReadGroup) ? "r" : "-";
  pss += (ps & QFile::WriteGroup) ? "w" : "-";
  pss += (ps & QFile::ExeGroup) ? "x" : "-";
  pss += (ps & QFile::ReadOther) ? "r" : "-";
  pss += (ps & QFile::WriteOther) ? "w" : "-";
  pss += (ps & QFile::ExeOther) ? "x" : "-";
  QString ext = fi.suffix().toLower();
  QString typeStr = "Text";
  if (ext == "org")
    typeStr = "Org";
  else if (ext == "md")
    typeStr = "Markdown";
  return Constants::TOOLTIP_FILE_INFO_FMT.arg(filePath).arg(ss).arg(pss).arg(
      typeStr);
}

void JinoEditor::redoEdit() {
  if (auto e = currentEditorWidget())
    e->redo();
}
void JinoEditor::undoEdit() {
  if (auto e = currentEditorWidget())
    e->undo();
}
void JinoEditor::cutEdit() {
  if (auto e = currentEditorWidget())
    e->cut();
}
void JinoEditor::copyEdit() {
  if (auto e = currentEditorWidget())
    e->copy();
}
void JinoEditor::pasteEdit() {
  if (auto e = currentEditorWidget())
    e->paste();
}
void JinoEditor::selectAllEdit() {
  if (auto e = currentEditorWidget())
    e->selectAll();
}

void JinoEditor::prevTab() {
  const int c = tabWidget->count();
  if (c <= 1)
    return;
  const int ci = tabWidget->currentIndex();
  tabWidget->setCurrentIndex((ci - 1 + c) % c);
}
void JinoEditor::nextTab() {
  const int c = tabWidget->count();
  if (c <= 1)
    return;
  const int ci = tabWidget->currentIndex();
  tabWidget->setCurrentIndex((ci + 1) % c);
}

} // namespace Jino::App
