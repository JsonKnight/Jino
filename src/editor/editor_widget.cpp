#include "editor/editor_widget.hpp"
#include "core/constants.hpp"
#include "editor/line_number_widget.hpp"
#include "editor/markdown_syntax_highlighter.hpp"
#include "editor/org_syntax_highlighter.hpp"
#include "editor/vim/vim_handler.hpp"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMimeData>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QWheelEvent>

EditorWidget::EditorWidget(QWidget *parent)
    : QTextEdit(parent), vimHandler(new Jino::Editor::Vim::VimHandler(this)),
      lineNumberWidget(new LineNumberWidget(this)) {

  defaultCursorWidth = 1;
  connect(this->document(), &QTextDocument::blockCountChanged, this,
          &EditorWidget::updateLineNumberAreaWidth);
  connect(this, &EditorWidget::cursorPositionChanged, this,
          &EditorWidget::updateLineNumberArea);
  connect(vimHandler, &Jino::Editor::Vim::VimHandler::modeChanged, this,
          &EditorWidget::vimModeChanged);
  connect(vimHandler, &Jino::Editor::Vim::VimHandler::saveFileRequested, this,
          &EditorWidget::saveFileRequested);
  connect(vimHandler, &Jino::Editor::Vim::VimHandler::saveFileAsRequested, this,
          &EditorWidget::saveFileAsRequested);
  updateLineNumberAreaWidth();
  vimSetMode(vimHandler->currentMode());
  setEditorMode(Jino::Constants::EditorFileType::Text);
  setZoom(currentZoomLevelPercent);
}

EditorWidget::~EditorWidget() {}

Jino::Editor::Vim::Mode EditorWidget::currentVimMode() const {
  return vimHandler ? vimHandler->currentMode()
                    : Jino::Editor::Vim::Mode::Insert;
}

void EditorWidget::setEditorMode(Jino::Constants::EditorFileType mode) {
  if (currentEditorMode == mode) {
    if ((mode != Jino::Constants::EditorFileType::Text && !syntaxHighlighter) ||
        (mode == Jino::Constants::EditorFileType::Text && syntaxHighlighter)) {
    } else {
      return;
    }
  }
  currentEditorMode = mode;
  setupSyntaxHighlighter(currentEditorMode);
  if (syntaxHighlighter)
    syntaxHighlighter->rehighlight();
  else {
    this->document()->markContentsDirty(
        0, this->document()->toPlainText().length());
  }
}
Jino::Constants::EditorFileType EditorWidget::editorMode() const {
  return currentEditorMode;
}
void EditorWidget::setupSyntaxHighlighter(
    Jino::Constants::EditorFileType mode) {
  if (syntaxHighlighter) {
    delete syntaxHighlighter;
    syntaxHighlighter = nullptr;
  }
  switch (mode) {
  case Jino::Constants::EditorFileType::Org:
    syntaxHighlighter =
        new Jino::Editor::OrgSyntaxHighlighter(this->document());
    break;
  case Jino::Constants::EditorFileType::Markdown:
    syntaxHighlighter =
        new Jino::Editor::MarkdownSyntaxHighlighter(this->document());
    break;
  case Jino::Constants::EditorFileType::Text:
  default:
    syntaxHighlighter = nullptr;
    break;
  }
}

void EditorWidget::keyPressEvent(QKeyEvent *event) {

  if (currentVimMode() == Jino::Editor::Vim::Mode::Insert) {
    if (event->matches(QKeySequence::Cut) ||
        (event->key() == Qt::Key_D &&
         event->modifiers() & Qt::ControlModifier)) {
      deleteCurrentLine();
      event->accept();
      return;
    }
  }
  const bool handledByVim = vimHandler->handleKeyPress(event);
  if (!handledByVim && currentVimMode() != Jino::Editor::Vim::Mode::Normal) {
    QTextEdit::keyPressEvent(event);
  }
}

void EditorWidget::insertFromMimeData(const QMimeData *source) {
  if (currentEditorMode == Jino::Constants::EditorFileType::Text &&
      source->hasText()) {
    QMimeData *plainTextData = new QMimeData();
    plainTextData->setText(source->text());
    QTextEdit::insertFromMimeData(plainTextData);
    delete plainTextData;
  } else {
    QTextEdit::insertFromMimeData(source);
  }
}

void EditorWidget::wheelEvent(QWheelEvent *e) {
  if (e->modifiers() & Qt::ControlModifier) {
    const int delta = e->angleDelta().y();
    if (delta > 0)
      zoomIn();
    else if (delta < 0)
      zoomOut();
    e->accept();
  } else {
    QTextEdit::wheelEvent(e);
  }
}

int EditorWidget::currentZoomPercent() const { return currentZoomLevelPercent; }
void EditorWidget::setZoom(int percent) {
  int cP = qBound(Jino::Constants::EDITOR_MIN_ZOOM_PERCENT, percent,
                  Jino::Constants::EDITOR_MAX_ZOOM_PERCENT);
  if (currentZoomLevelPercent == cP)
    return;
  currentZoomLevelPercent = cP;
  qreal f = (qreal)cP / 100.0;
  QFont cF = font();
  qreal nS = Jino::Constants::FONT_SIZE_PT * f;
  cF.setPointSizeF(nS);
  setFont(cF);
  emit zoomPercentChanged(currentZoomLevelPercent);
  updateLineNumberAreaWidth();
}
void EditorWidget::zoomIn(int range) {
  int s = range * Jino::Constants::EDITOR_ZOOM_STEP;
  setZoom(currentZoomLevelPercent + s);
}
void EditorWidget::zoomOut(int range) {
  int s = range * Jino::Constants::EDITOR_ZOOM_STEP;
  setZoom(currentZoomLevelPercent - s);
}
void EditorWidget::resetZoom() {
  setZoom(Jino::Constants::EDITOR_DEFAULT_ZOOM_PERCENT);
}

int EditorWidget::calculateLineNumberWidth() const {
  int d = 1;
  int m = qMax(1, document()->blockCount());
  while (m >= 10) {
    m /= 10;
    ++d;
  }
  int s = 3 * 2 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * d;
  return s;
}
void EditorWidget::updateLineNumberAreaWidth() {
  setViewportMargins(calculateLineNumberWidth(), 0, 0, 0);
}
void EditorWidget::triggerLineNumberUpdate() const { updateLineNumberArea(); }
void EditorWidget::updateLineNumberArea() const {
  if (lineNumberWidget)
    lineNumberWidget->update();
}
void EditorWidget::resizeEvent(QResizeEvent *e) {
  QTextEdit::resizeEvent(e);
  if (lineNumberWidget) {
    QRect cr = contentsRect();
    lineNumberWidget->setGeometry(
        QRect(cr.left(), cr.top(), calculateLineNumberWidth(), cr.height()));
  }
}
void EditorWidget::scrollContentsBy(int dx, int dy) {
  QTextEdit::scrollContentsBy(dx, dy);
  updateLineNumberArea();
}

void EditorWidget::goToLine(int lineNum) {
  if (lineNum < 1 || lineNum > document()->blockCount()) {
    qWarning() << "Go To Line: Invalid line number" << lineNum;
    return;
  }
  QTextCursor cursor(document()->findBlockByNumber(lineNum - 1));
  setTextCursor(cursor);
  ensureCursorVisible();
}
void EditorWidget::goToColumn(int colNum) {
  if (colNum < 1) {
    qWarning() << "Go To Column: Invalid column number" << colNum;
    return;
  }
  QTextCursor cursor = textCursor();
  cursor.movePosition(QTextCursor::StartOfLine);
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, colNum - 1);
  setTextCursor(cursor);
  ensureCursorVisible();
}
void EditorWidget::goToTop() {
  QTextCursor cursor = textCursor();
  cursor.movePosition(QTextCursor::Start);
  setTextCursor(cursor);
  ensureCursorVisible();
}
void EditorWidget::goToCenter() {
  int totalBlocks = document()->blockCount();
  int centerBlock = qMax(0, (totalBlocks / 2) - 1);
  QTextCursor cursor(document()->findBlockByNumber(centerBlock));
  setTextCursor(cursor);
  ensureCursorVisible();
}
void EditorWidget::goToBottom() {
  QTextCursor cursor = textCursor();
  cursor.movePosition(QTextCursor::End);
  setTextCursor(cursor);
  ensureCursorVisible();
}

void EditorWidget::pasteAndReplace() {
  const QClipboard *cb = QApplication::clipboard();
  const QMimeData *md = cb->mimeData();
  if (md->hasText())
    this->setPlainText(md->text());
}
void EditorWidget::clearAll() {
  this->selectAll();
  this->textCursor().removeSelectedText();
}
void EditorWidget::deleteCurrentLine() {
  QTextCursor c = textCursor();
  c.beginEditBlock();
  c.select(QTextCursor::LineUnderCursor);
  if (document()->findBlockByNumber(c.blockNumber() + 1).isValid())
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  c.removeSelectedText();
  c.movePosition(QTextCursor::StartOfLine);
  c.endEditBlock();
  setTextCursor(c);
}

void EditorWidget::vimSetMode(Jino::Editor::Vim::Mode newMode) {
  const bool ro = (newMode == Jino::Editor::Vim::Mode::Normal ||
                   newMode == Jino::Editor::Vim::Mode::Visual);
  setReadOnly(ro);
  if (ro) {
    setOverwriteMode(true);
    int bw = fontMetrics().averageCharWidth();
    setCursorWidth(qMax(2, bw));
  } else {
    setOverwriteMode(false);
    setCursorWidth(defaultCursorWidth);
  }
}
void EditorWidget::vimDeleteLine() { deleteCurrentLine(); }
void EditorWidget::vimCopyLine() {
  QTextCursor c = textCursor();
  c.select(QTextCursor::LineUnderCursor);
  if (document()->findBlockByNumber(c.blockNumber() + 1).isValid())
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  const QString t = c.selectedText();
  if (!t.isEmpty())
    QApplication::clipboard()->setText(t);
  c.clearSelection();
  c.movePosition(QTextCursor::StartOfLine);
  setTextCursor(c);
}
void EditorWidget::vimCutSelection() {
  QTextCursor c = textCursor();
  if (!c.hasSelection())
    return;
  c.beginEditBlock();
  QApplication::clipboard()->setText(c.selectedText());
  c.removeSelectedText();
  c.endEditBlock();
  setTextCursor(c);
}
void EditorWidget::vimCopySelection() {
  QTextCursor c = textCursor();
  if (!c.hasSelection())
    return;
  const QString t = c.selectedText();
  if (!t.isEmpty())
    QApplication::clipboard()->setText(t);
  c.setPosition(c.selectionStart());
  setTextCursor(c);
}
void EditorWidget::vimPasteText() {
  bool ro = isReadOnly();
  if (ro)
    setReadOnly(false);
  QTextCursor c = textCursor();
  if (isReadOnly()) {
    c.movePosition(QTextCursor::NextCharacter);
    setTextCursor(c);
  }
  paste();
  if (ro)
    setReadOnly(true);
}
void EditorWidget::vimUndo() {
  bool ro = isReadOnly();
  if (ro)
    setReadOnly(false);
  undo();
  if (ro)
    setReadOnly(true);
}
void EditorWidget::vimCutChar() {
  bool ro = isReadOnly();
  if (ro)
    setReadOnly(false);
  QTextCursor c = textCursor();
  c.beginEditBlock();
  if (!c.atEnd()) {
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
    if (c.hasSelection())
      c.removeSelectedText();
  }
  c.endEditBlock();
  if (ro)
    setReadOnly(true);
  setTextCursor(c);
}
void EditorWidget::vimToggleVisualCharacterMode() {
  QTextCursor c = textCursor();
  if (!c.hasSelection()) {
    if (!c.atEnd())
      c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
    else if (!c.atStart())
      c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor,
                     1);
    if (c.hasSelection())
      setTextCursor(c);
  }
}
