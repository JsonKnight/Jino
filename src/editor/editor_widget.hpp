// src/editor/editor_widget.hpp
#pragma once

#include "core/constants.hpp"
#include "editor/vim/vim_modes.hpp"

#include <QMimeData>
#include <QPointer>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextEdit>

class QKeyEvent;
class QResizeEvent;
class QWidget;
class LineNumberWidget;
class QWheelEvent;

namespace Jino::Editor::Vim {
class VimHandler;
}

class EditorWidget : public QTextEdit {
  Q_OBJECT

public:
  explicit EditorWidget(QWidget *parent = nullptr);
  ~EditorWidget() override;

  Jino::Editor::Vim::Mode currentVimMode() const;

  void setEditorMode(Jino::Constants::EditorFileType mode);
  Jino::Constants::EditorFileType editorMode() const;

  void vimSetMode(Jino::Editor::Vim::Mode newMode);
  void vimDeleteLine();
  void vimCopyLine();
  void vimCutSelection();
  void vimCopySelection();
  void vimPasteText();
  void vimUndo();
  void vimCutChar();
  void vimToggleVisualCharacterMode();

  void triggerLineNumberUpdate() const;

  int currentZoomPercent() const;

  void goToLine(int lineNum);
  void goToColumn(int colNum);
  void goToTop();
  void goToCenter();
  void goToBottom();

  void pasteAndReplace();
  void clearAll();
  void deleteCurrentLine();

public slots:
  void zoomIn(int range = 1);
  void zoomOut(int range = 1);
  void resetZoom();

signals:
  void vimModeChanged(Jino::Editor::Vim::Mode newMode);
  void saveFileRequested();
  void saveFileAsRequested();
  void zoomPercentChanged(int percent);

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void scrollContentsBy(int dx, int dy) override;
  void insertFromMimeData(const QMimeData *source) override;
  void wheelEvent(QWheelEvent *e) override;

private:
  void setupSyntaxHighlighter(Jino::Constants::EditorFileType mode);
  void updateLineNumberAreaWidth();
  void updateLineNumberArea() const;
  int calculateLineNumberWidth() const;
  void setZoom(int percent);

  Jino::Editor::Vim::VimHandler *vimHandler;
  QPointer<LineNumberWidget> lineNumberWidget;
  QSyntaxHighlighter *syntaxHighlighter = nullptr;

  Jino::Constants::EditorFileType currentEditorMode =
      Jino::Constants::EditorFileType::Text;
  int defaultCursorWidth = 1;
  int currentZoomLevelPercent = Jino::Constants::EDITOR_DEFAULT_ZOOM_PERCENT;
};
