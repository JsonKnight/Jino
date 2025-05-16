#include "app/status_bar_manager.hpp"
#include "QtAwesome.h"
#include "core/constants.hpp"
#include "editor/editor_widget.hpp"
#include "editor/vim/vim_modes.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QSizePolicy>
#include <QStatusBar>
#include <QStyle>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

namespace Jino::App {

fa::icon_enum getIconForVimMode(Jino::Editor::Vim::Mode mode) {
  using namespace Jino::Editor::Vim;
  using namespace fa;
  switch (mode) {
  case Mode::Normal:
    return (fa::icon_enum)fa_scroll;
  case Mode::Insert:
    return (fa::icon_enum)fa_i_cursor;
  case Mode::Visual:
    return (fa::icon_enum)fa_highlighter;
  default:
    return (fa::icon_enum)fa_question;
  }
}
fa::icon_enum getIconForFileType(Jino::Constants::EditorFileType type) {
  using namespace Jino::Constants;
  using namespace fa;
  switch (type) {
  case EditorFileType::Org:
    return (fa::icon_enum)fa_star_of_life;
  case EditorFileType::Markdown:
    return (fa::icon_enum)fa_markdown;
  case EditorFileType::Text:
  default:
    return (fa::icon_enum)fa_file_lines;
  }
}

StatusBarManager::StatusBarManager(QMainWindow *parentWindow,
                                   const QString &workspaceName,
                                   fa::QtAwesome *awesomePtr)
    : QObject(parentWindow), mainWindow(parentWindow),
      currentWorkspaceName(workspaceName), awesome(awesomePtr) {
  if (awesome) {
    using namespace fa;
    workspaceIcon = awesome->icon(fa_solid, fa_layer_group);
  }
}

QWidget *StatusBarManager::getEditorModeWidget() const {
  return editorModeButton;
}

void StatusBarManager::setupUI() {
  createTopStatusBar();
  setupMainStatusBar();
  updateTopStatusBar(nullptr);
}

void StatusBarManager::createTopStatusBar() {
  topStatusBar = mainWindow->addToolBar("Top Status Bar");
  topStatusBar->setObjectName("TopStatusBar");
  topStatusBar->setMovable(false);
  topStatusBar->setFloatable(false);
  topStatusBar->setIconSize(QSize(16, 16));

  vimStatusLabel = new QLabel(mainWindow);
  vimStatusLabel->setObjectName("StatusBarVimLabel");

  editorModeButton = new QToolButton(mainWindow);
  editorModeButton->setObjectName("StatusBarModeButton");
  editorModeButton->setPopupMode(QToolButton::InstantPopup);
  editorModeButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  editorModeButton->setFocusPolicy(Qt::NoFocus);
  editorModeButton->setStyleSheet(
      "QToolButton { border: none; padding: 2px; margin-left: 5px; "
      "margin-right: 5px; background-color: transparent; } "
      "QToolButton:menu-indicator{ image: none; }");
  editorModeButton->setCursor(Qt::PointingHandCursor);
  connect(editorModeButton, &QToolButton::clicked, this,
          &StatusBarManager::editorModeChangeRequested);

  positionActionWidget = new QWidget(mainWindow);
  QHBoxLayout *centerLayout = new QHBoxLayout(positionActionWidget);
  centerLayout->setContentsMargins(0, 0, 0, 0);
  centerLayout->setSpacing(6);

  lineButton = new QToolButton(positionActionWidget);
  lineButton->setObjectName("StatusBarLineButton");
  lineButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  lineButton->setFocusPolicy(Qt::NoFocus);
  lineButton->setStyleSheet("QToolButton { border: none; padding: 1px; "
                            "background-color: transparent; }");
  lineButton->setToolTip("Go To Line");
  lineButton->setCursor(Qt::PointingHandCursor);
  if (awesome)
    lineButton->setIcon(awesome->icon(
        fa::fa_solid, static_cast<fa::icon_enum>(fa::fa_list_ol)));
  connect(lineButton, &QToolButton::clicked, this,
          &StatusBarManager::goToLineRequested);

  columnButton = new QToolButton(positionActionWidget);
  columnButton->setObjectName("StatusBarColButton");
  columnButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  columnButton->setFocusPolicy(Qt::NoFocus);
  columnButton->setStyleSheet("QToolButton { border: none; padding: 1px; "
                              "background-color: transparent; }");
  columnButton->setToolTip("Go To Column");
  columnButton->setCursor(Qt::PointingHandCursor);
  if (awesome)
    columnButton->setIcon(awesome->icon(
        fa::fa_solid,
        static_cast<fa::icon_enum>(fa::fa_arrows_left_right_to_line)));
  connect(columnButton, &QToolButton::clicked, this,
          &StatusBarManager::goToColumnRequested);

  using namespace fa;

  closeButton = new QToolButton(positionActionWidget);
  closeButton->setObjectName("StatusBarCloseButton");
  closeButton->setFocusPolicy(Qt::NoFocus);
  closeButton->setAutoRaise(true);
  closeButton->setFixedSize(20, 20);
  closeButton->setIconSize(QSize(14, 14));
  closeButton->setToolTip("Close Current File");
  if (awesome)
    closeButton->setIcon(
        awesome->icon(fa_solid, static_cast<fa::icon_enum>(fa_xmark)));
  connect(closeButton, &QToolButton::clicked, this,
          &StatusBarManager::closeCurrentTabRequested);

  saveButton = new QToolButton(positionActionWidget);
  saveButton->setObjectName("StatusBarSaveButton");
  saveButton->setFocusPolicy(Qt::NoFocus);
  saveButton->setAutoRaise(true);
  saveButton->setFixedSize(20, 20);
  saveButton->setIconSize(QSize(14, 14));
  saveButton->setToolTip("Save Current File");
  if (awesome)
    saveButton->setIcon(
        awesome->icon(fa_solid, static_cast<fa::icon_enum>(fa_floppy_disk)));
  connect(saveButton, &QToolButton::clicked, this,
          &StatusBarManager::saveRequested);

  infoButton = new QToolButton(positionActionWidget);
  infoButton->setObjectName("StatusBarInfoButton");
  infoButton->setFocusPolicy(Qt::NoFocus);
  infoButton->setAutoRaise(true);
  infoButton->setFixedSize(20, 20);
  infoButton->setIconSize(QSize(14, 14));
  if (awesome)
    infoButton->setIcon(
        awesome->icon(fa_solid, static_cast<fa::icon_enum>(fa_circle_info)));

  goToTopButton = new QToolButton(positionActionWidget);
  goToTopButton->setObjectName("StatusBarGoTopButton");
  goToTopButton->setFocusPolicy(Qt::NoFocus);
  goToTopButton->setAutoRaise(true);
  goToTopButton->setFixedSize(20, 20);
  goToTopButton->setIconSize(QSize(14, 14));
  goToTopButton->setToolTip("Go To Top");
  if (awesome)
    goToTopButton->setIcon(
        awesome->icon(fa_solid, static_cast<fa::icon_enum>(fa_angles_up)));
  connect(goToTopButton, &QToolButton::clicked, this,
          &StatusBarManager::goToTopRequested);

  goToCenterButton = new QToolButton(positionActionWidget);
  goToCenterButton->setObjectName("StatusBarGoCenterButton");
  goToCenterButton->setFocusPolicy(Qt::NoFocus);
  goToCenterButton->setAutoRaise(true);
  goToCenterButton->setFixedSize(20, 20);
  goToCenterButton->setIconSize(QSize(14, 14));
  goToCenterButton->setToolTip("Go To Center");
  if (awesome)
    goToCenterButton->setIcon(
        awesome->icon(fa_solid, static_cast<fa::icon_enum>(fa_arrows_up_down)));
  connect(goToCenterButton, &QToolButton::clicked, this,
          &StatusBarManager::goToCenterRequested);

  goToBottomButton = new QToolButton(positionActionWidget);
  goToBottomButton->setObjectName("StatusBarGoBottomButton");
  goToBottomButton->setFocusPolicy(Qt::NoFocus);
  goToBottomButton->setAutoRaise(true);
  goToBottomButton->setFixedSize(20, 20);
  goToBottomButton->setIconSize(QSize(14, 14));
  goToBottomButton->setToolTip("Go To Bottom");
  if (awesome)
    goToBottomButton->setIcon(
        awesome->icon(fa_solid, static_cast<fa::icon_enum>(fa_angles_down)));
  connect(goToBottomButton, &QToolButton::clicked, this,
          &StatusBarManager::goToBottomRequested);

  pasteReplaceButton = new QToolButton(positionActionWidget);
  pasteReplaceButton->setObjectName("StatusBarPasteReplaceButton");
  pasteReplaceButton->setFocusPolicy(Qt::NoFocus);
  pasteReplaceButton->setAutoRaise(true);
  pasteReplaceButton->setFixedSize(20, 20);
  pasteReplaceButton->setIconSize(QSize(14, 14));
  pasteReplaceButton->setToolTip("Paste & Replace All");
  if (awesome)
    pasteReplaceButton->setIcon(
        awesome->icon(fa_solid, static_cast<fa::icon_enum>(fa_paste)));
  connect(pasteReplaceButton, &QToolButton::clicked, this,
          &StatusBarManager::pasteAndReplaceRequested);

  clearFileButton = new QToolButton(positionActionWidget);
  clearFileButton->setObjectName("StatusBarClearButton");
  clearFileButton->setFocusPolicy(Qt::NoFocus);
  clearFileButton->setAutoRaise(true);
  clearFileButton->setFixedSize(20, 20);
  clearFileButton->setIconSize(QSize(14, 14));
  clearFileButton->setToolTip("Clear File");
  if (awesome)
    clearFileButton->setIcon(awesome->icon(
        fa_solid, static_cast<fa::icon_enum>(fa_file_circle_xmark)));
  connect(clearFileButton, &QToolButton::clicked, this,
          &StatusBarManager::clearFileRequested);

  centerLayout->addWidget(lineButton);
  centerLayout->addWidget(columnButton);
  centerLayout->addSpacing(10);
  centerLayout->addWidget(closeButton);
  centerLayout->addWidget(saveButton);
  centerLayout->addWidget(infoButton);
  centerLayout->addWidget(goToTopButton);
  centerLayout->addWidget(goToCenterButton);
  centerLayout->addWidget(goToBottomButton);
  centerLayout->addWidget(pasteReplaceButton);
  centerLayout->addWidget(clearFileButton);
  positionActionWidget->setLayout(centerLayout);

  statsCharsLabel = new QLabel(mainWindow);
  statsCharsLabel->setObjectName("StatusBarStatsCharsLabel");
  statsWordsLabel = new QLabel(mainWindow);
  statsWordsLabel->setObjectName("StatusBarStatsWordsLabel");
  timeLabel = new QLabel(mainWindow);
  timeLabel->setObjectName("StatusBarTimeLabel");
  zoomWidget = new QToolButton(mainWindow);
  zoomWidget->setObjectName("StatusBarZoomWidget");
  zoomWidget->setFocusPolicy(Qt::NoFocus);
  zoomWidget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  zoomWidget->setStyleSheet(
      "QToolButton { border: none; padding: 1px; margin-left: 5px; "
      "margin-right: 5px; background-color: transparent; }");
  zoomWidget->setToolTip("Current Zoom (Click to Reset)");
  zoomWidget->setCursor(Qt::PointingHandCursor);
  if (awesome)
    zoomWidget->setIcon(awesome->icon(fa_solid, fa_search));
  connect(zoomWidget, &QToolButton::clicked, this,
          &StatusBarManager::handleZoomWidgetClicked);

  topStatusBar->addWidget(vimStatusLabel);
  topStatusBar->addWidget(editorModeButton);
  topStatusBar->addSeparator();
  topStatusBar->addWidget(positionActionWidget);
  QWidget *spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  topStatusBar->addWidget(spacer);
  topStatusBar->addWidget(statsCharsLabel);
  topStatusBar->addWidget(statsWordsLabel);
  topStatusBar->addWidget(timeLabel);
  topStatusBar->addWidget(zoomWidget);
}

void StatusBarManager::setupMainStatusBar() {
  statusBarWorkspaceLabel = new QLabel(mainWindow);
  statusBarWorkspaceLabel->setObjectName("MainStatusBarWorkspaceLabel");
  QString dName = currentWorkspaceName;
  if (dName.startsWith(Constants::WORKSPACE_PREFIX))
    dName = dName.mid(Constants::WORKSPACE_PREFIX.length());
  if (!dName.isEmpty())
    dName[0] = dName[0].toUpper();
  QString txt = QString(" %1").arg(dName);
  if (!workspaceIcon.isNull()) {
    using namespace fa;
    txt.prepend(QChar((int)fa_layer_group) + QString(" "));
  }
  statusBarWorkspaceLabel->setText(txt);
  statusBarWorkspaceLabel->setToolTip(currentWorkspaceName);
  mainWindow->statusBar()->addPermanentWidget(statusBarWorkspaceLabel);
}

void StatusBarManager::updateTopStatusBar(EditorWidget *currentEditor) {
  bool editorExists = (currentEditor != nullptr);
  bool modified = editorExists && currentEditor->document()->isModified();
  bool hasText = editorExists && !currentEditor->document()->isEmpty();
  bool canPaste = QApplication::clipboard()->mimeData()->hasText();

  if (vimStatusLabel) {
    QString iT = "", mT = "---";
    if (editorExists && awesome) {
      using namespace fa;
      icon_enum i =
          Jino::App::getIconForVimMode(currentEditor->currentVimMode());
      iT = QChar((int)i) + QString(" ");
      mT = Editor::Vim::modeToString(currentEditor->currentVimMode());
    } else if (editorExists) {
      mT = Editor::Vim::modeToString(currentEditor->currentVimMode());
    }
    vimStatusLabel->setText(Constants::STATUS_VIM_MODE_FMT.arg(iT).arg(mT));
  }
  if (editorModeButton) {
    QString mT = "---";
    QIcon mI;
    if (editorExists) {
      Constants::EditorFileType cT = currentEditor->editorMode();
      mT = Constants::editorModeToString(cT);
      if (awesome) {
        using namespace fa;
        icon_enum iE = Jino::App::getIconForFileType(cT);
        mI = awesome->icon(fa_solid, iE);
      }
    }
    editorModeButton->setText(mT);
    editorModeButton->setIcon(mI);
    editorModeButton->setEnabled(editorExists);
    editorModeButton->setToolTip(editorExists ? "Change Editor Mode" : "");
  }

  if (lineButton) {
    QString lt = "---";
    if (editorExists)
      lt = Constants::STATUS_POS_LINE_FMT.arg(
          currentEditor->textCursor().blockNumber() + 1);
    lineButton->setText(lt);
    lineButton->setEnabled(editorExists);
  }
  if (columnButton) {
    QString ct = "---";
    if (editorExists)
      ct = Constants::STATUS_POS_COL_FMT.arg(
          currentEditor->textCursor().columnNumber() + 1);
    columnButton->setText(ct);
    columnButton->setEnabled(editorExists);
  }
  if (closeButton)
    closeButton->setEnabled(editorExists);
  if (saveButton)
    saveButton->setEnabled(modified);
  if (infoButton) {
    infoButton->setEnabled(editorExists);

    infoButton->setToolTip(editorExists ? "File Information" : "");
  }
  if (goToTopButton)
    goToTopButton->setEnabled(editorExists);
  if (goToCenterButton)
    goToCenterButton->setEnabled(editorExists);
  if (goToBottomButton)
    goToBottomButton->setEnabled(editorExists);
  if (pasteReplaceButton)
    pasteReplaceButton->setEnabled(editorExists && canPaste);
  if (clearFileButton)
    clearFileButton->setEnabled(hasText);

  updateEditorStats(currentEditor);
  if (timeLabel) {
  }
  if (zoomWidget) {
    if (editorExists)
      updateZoomDisplay(currentEditor->currentZoomPercent());
    else
      updateZoomDisplay(Constants::EDITOR_DEFAULT_ZOOM_PERCENT);
    zoomWidget->setEnabled(editorExists);
  }

  positionActionWidget->setVisible(true);
  statsCharsLabel->setVisible(true);
  statsWordsLabel->setVisible(true);
  timeLabel->setVisible(true);
  zoomWidget->setVisible(true);
}

void StatusBarManager::updateEditorStats(EditorWidget *currentEditor) {
  using namespace fa;
  QString charText = "---";
  QString wordText = "---";
  QString charIconText = "";
  QString wordIconText = "";

  if (currentEditor && currentEditor->document()) {
    const int charCount = currentEditor->document()->characterCount() - 1;
    const QStringList words = currentEditor->toPlainText().split(
        QRegExp("\\\\s+"), Qt::SkipEmptyParts);
    if (awesome) {

      charIconText = QString(QChar(static_cast<int>(fa_font))) + " ";
      wordIconText = QString(QChar(static_cast<int>(fa_file_word))) + " ";
    }
    charText = Constants::STATUS_STATS_CHARS_FMT.arg(charIconText)
                   .arg(charCount)
                   .trimmed();
    wordText = Constants::STATUS_STATS_WORDS_FMT.arg(wordIconText)
                   .arg(words.count())
                   .trimmed();
  }

  if (statsCharsLabel)
    statsCharsLabel->setText(charText);
  if (statsWordsLabel)
    statsWordsLabel->setText(wordText);
}

void StatusBarManager::updateTimeDisplay(const QString &timeString) {
  if (timeLabel) {
    QString iconText = "";
    if (awesome) {

      iconText = QString(QChar(static_cast<int>(fa::fa_solid))) +
                 QString(QChar(static_cast<int>(fa::fa_clock))) + " ";
    }
    timeLabel->setText(
        Constants::STATUS_TIME_FMT.arg(iconText).arg(timeString).trimmed());
  }
}

void StatusBarManager::updateZoomDisplay(int zoomPercent) {
  if (!zoomWidget)
    return;
  using namespace fa;
  QIcon zI;
  if (awesome)
    zI = awesome->icon(fa_solid, fa_search);
  QString t = Constants::STATUS_ZOOM_FMT.arg("").arg(zoomPercent).trimmed();
  zoomWidget->setText(t);
  zoomWidget->setIcon(zI);
}
void StatusBarManager::handleZoomWidgetClicked() { emit resetZoomRequested(); }

} // namespace Jino::App
