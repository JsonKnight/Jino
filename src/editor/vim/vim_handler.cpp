#include "editor/vim/vim_handler.hpp"
#include "core/constants.hpp"
#include "editor/editor_widget.hpp"

#include <QAbstractSlider>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextCursor>

namespace Jino::Editor::Vim {

VimHandler::VimHandler(EditorWidget *editor)
    : QObject(editor), editorWidget(editor) {
  setMode(Mode::Insert);
}

Mode VimHandler::currentMode() const { return mode; }

void VimHandler::resetLeaderState() {
  waitingForFileLeaderKey = false;
  waitingForFileSaveKey = false;
}

bool VimHandler::handleKeyPress(QKeyEvent *event) {
  bool handled = true;

  if (event->key() == Qt::Key_Escape) {
    executeCommandExitToNormalMode();
    return true;
  }

  switch (mode) {
  case Mode::Insert:

    handled = false;
    break;
  case Mode::Normal:
    processNormalModeKey(event);

    break;
  case Mode::Visual:
    processVisualModeKey(event);

    break;
  default:
    handled = false;
    break;
  }
  return handled;
}

void VimHandler::processInsertModeKey(QKeyEvent *event) { Q_UNUSED(event); }

void VimHandler::processNormalModeKey(QKeyEvent *event) {
  if (!editorWidget)
    return;

  const QString keyText = event->text();
  const int key = event->key();
  const Qt::KeyboardModifiers modifiers = event->modifiers();
  bool keyProcessed = true;

  if (waitingForFileLeaderKey) {
    if (keyText == Constants::VIM_LEADER_FILE_SEQUENCE) {
      waitingForFileSaveKey = true;
      waitingForFileLeaderKey = false;

    } else {

      resetLeaderState();
      keyProcessed = false;
    }
  } else if (waitingForFileSaveKey) {
    if (keyText == QChar(Constants::VIM_LEADER_FILE_SAVE_KEY) &&
        !(modifiers & Qt::ShiftModifier)) {

      executeCommandSaveFile();
      resetLeaderState();
    } else if (keyText.toUpper() ==
                   QChar(Constants::VIM_LEADER_FILE_SAVEAS_KEY) &&
               (modifiers & Qt::ShiftModifier)) {

      executeCommandSaveFileAs();
      resetLeaderState();
    } else {

      resetLeaderState();
    }
  }

  else if (!waitingForFileLeaderKey && !waitingForFileSaveKey) {
    keyProcessed = true;

    if (editorWidget->textCursor().hasSelection()) {
      if (keyText == "d") {
        executeCommandCutSelection();
        resetLeaderState();
        return;
      }
      if (keyText == "y") {
        executeCommandCopySelection();
        resetLeaderState();
        return;
      }

      if (key != Qt::Key_Escape && key != Qt::Key_V) {
        QTextCursor cursor = editorWidget->textCursor();
        cursor.clearSelection();
        editorWidget->setTextCursor(cursor);
      }
    }

    if (keyText.length() == 1 && keyText != " ") {
      const char keyChar = keyText.at(0).toLatin1();
      switch (keyChar) {
      case Constants::VIM_KEY_INSERT_MODE:
        executeCommandEnterInsertMode();
        break;
      case Constants::VIM_KEY_UNDO:
        executeCommandUndo();
        break;
      case Constants::VIM_KEY_CUT_CHAR:
        executeCommandCutChar();
        break;
      case Constants::VIM_KEY_PASTE:
        executeCommandPaste();
        break;
      case 'v':
        executeCommandEnterVisualMode();
        break;

      case Constants::VIM_KEY_DELETE_LINE:
      case Constants::VIM_KEY_COPY_LINE:
        commandBuffer += keyChar;
        processCommandBuffer();
        break;
      default:
        keyProcessed = false;
        break;
      }
    } else if (key == Qt::Key_Space) {
      waitingForFileLeaderKey = true;
      clearCommandBuffer();

    } else {
      keyProcessed = false;
    }

    if (!keyProcessed) {
      keyProcessed = true;
      switch (key) {
      case Qt::Key_H:
      case Qt::Key_Left:
        editorWidget->moveCursor(QTextCursor::Left);
        break;
      case Qt::Key_L:
      case Qt::Key_Right:
        editorWidget->moveCursor(QTextCursor::Right);
        break;
      case Qt::Key_K:
      case Qt::Key_Up:
        editorWidget->moveCursor(QTextCursor::Up);
        break;
      case Qt::Key_J:
      case Qt::Key_Down:
        editorWidget->moveCursor(QTextCursor::Down);
        break;
      case Qt::Key_PageUp:
        editorWidget->verticalScrollBar()->triggerAction(
            QAbstractSlider::SliderPageStepSub);
        editorWidget->triggerLineNumberUpdate();
        break;
      case Qt::Key_PageDown:
        editorWidget->verticalScrollBar()->triggerAction(
            QAbstractSlider::SliderPageStepAdd);
        editorWidget->triggerLineNumberUpdate();
        break;
      case Qt::Key_Home:
        editorWidget->moveCursor(QTextCursor::StartOfLine);
        break;
      case Qt::Key_End:
        editorWidget->moveCursor(QTextCursor::EndOfLine);
        break;

      default:
        keyProcessed = false;
        break;
      }
    }

    if (keyProcessed && key != Constants::VIM_KEY_DELETE_LINE &&
        key != Constants::VIM_KEY_COPY_LINE && key != Qt::Key_Space) {
      clearCommandBuffer();
      resetLeaderState();
    }
  }

  if (keyProcessed) {

  } else {

    clearCommandBuffer();
    resetLeaderState();
  }
}

void VimHandler::processVisualModeKey(QKeyEvent *event) {
  if (!editorWidget)
    return;

  const QString keyText = event->text();
  const int key = event->key();
  const Qt::KeyboardModifiers modifiers = event->modifiers();
  bool keyProcessed = true;

  if (waitingForFileLeaderKey) {
    if (keyText == Constants::VIM_LEADER_FILE_SEQUENCE) {
      waitingForFileSaveKey = true;
      waitingForFileLeaderKey = false;
    } else {
      resetLeaderState();
      keyProcessed = false;
    }
  } else if (waitingForFileSaveKey) {
    if (keyText == QChar(Constants::VIM_LEADER_FILE_SAVE_KEY) &&
        !(modifiers & Qt::ShiftModifier)) {
      executeCommandSaveFile();
      resetLeaderState();
      executeCommandExitToNormalMode();
    } else if (keyText.toUpper() ==
                   QChar(Constants::VIM_LEADER_FILE_SAVEAS_KEY) &&
               (modifiers & Qt::ShiftModifier)) {
      executeCommandSaveFileAs();
      resetLeaderState();
      executeCommandExitToNormalMode();
    } else {
      resetLeaderState();
    }
  }

  else if (!waitingForFileLeaderKey && !waitingForFileSaveKey) {
    keyProcessed = true;
    if (keyText == "d" || keyText == "x") {
      executeCommandCutSelection();
      executeCommandExitToNormalMode();
    } else if (keyText == "y") {
      executeCommandCopySelection();
      executeCommandExitToNormalMode();
    } else if (key == Qt::Key_Space) {
      waitingForFileLeaderKey = true;

    } else {

      switch (key) {
      case Qt::Key_H:
      case Qt::Key_Left:
        editorWidget->moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
        break;
      case Qt::Key_L:
      case Qt::Key_Right:
        editorWidget->moveCursor(QTextCursor::Right, QTextCursor::KeepAnchor);
        break;
      case Qt::Key_K:
      case Qt::Key_Up:
        editorWidget->moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
        break;
      case Qt::Key_J:
      case Qt::Key_Down:
        editorWidget->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
        break;
      case Qt::Key_Home:
        editorWidget->moveCursor(QTextCursor::StartOfLine,
                                 QTextCursor::KeepAnchor);
        break;
      case Qt::Key_End:
        editorWidget->moveCursor(QTextCursor::EndOfLine,
                                 QTextCursor::KeepAnchor);
        break;

      case Qt::Key_PageUp:
        editorWidget->moveCursor(QTextCursor::StartOfBlock,
                                 QTextCursor::KeepAnchor);
        editorWidget->verticalScrollBar()->triggerAction(
            QAbstractSlider::SliderPageStepSub);
        editorWidget->triggerLineNumberUpdate();
        break;
      case Qt::Key_PageDown:
        editorWidget->moveCursor(QTextCursor::EndOfBlock,
                                 QTextCursor::KeepAnchor);
        editorWidget->verticalScrollBar()->triggerAction(
            QAbstractSlider::SliderPageStepAdd);
        editorWidget->triggerLineNumberUpdate();
        break;

      default:
        keyProcessed = false;
        break;
      }
    }

    if (key != Qt::Key_Space) {
      resetLeaderState();
    }
  }

  if (!keyProcessed) {

    resetLeaderState();
  }
}

void VimHandler::setMode(Mode newMode) {
  if (mode == newMode || !editorWidget)
    return;

  Mode oldMode = mode;
  mode = newMode;

  clearCommandBuffer();
  resetLeaderState();

  editorWidget->vimSetMode(newMode);

  if (oldMode == Mode::Visual && newMode != Mode::Visual) {
    QTextCursor cursor = editorWidget->textCursor();
    cursor.clearSelection();
    editorWidget->setTextCursor(cursor);
  } else if (newMode == Mode::Visual && oldMode != Mode::Visual &&
             !editorWidget->textCursor().hasSelection()) {

    editorWidget->vimToggleVisualCharacterMode();
  }

  emit modeChanged(mode);
}

void VimHandler::processCommandBuffer() {
  if (commandBuffer == Constants::VIM_CMD_DELETE_LINE) {
    executeCommandDeleteLine();
    clearCommandBuffer();
  } else if (commandBuffer == Constants::VIM_CMD_COPY_LINE) {
    executeCommandCopyLine();
    clearCommandBuffer();
  } else if (commandBuffer.length() >= 2) {

    clearCommandBuffer();
  }
}

void VimHandler::clearCommandBuffer() { commandBuffer.clear(); }

void VimHandler::executeCommandDeleteLine() {
  if (editorWidget)
    editorWidget->vimDeleteLine();
}
void VimHandler::executeCommandCopyLine() {
  if (editorWidget)
    editorWidget->vimCopyLine();
}
void VimHandler::executeCommandCutSelection() {
  if (editorWidget)
    editorWidget->vimCutSelection();
}
void VimHandler::executeCommandCopySelection() {
  if (editorWidget)
    editorWidget->vimCopySelection();
}
void VimHandler::executeCommandPaste() {
  if (editorWidget)
    editorWidget->vimPasteText();
}
void VimHandler::executeCommandUndo() {
  if (editorWidget)
    editorWidget->vimUndo();
}
void VimHandler::executeCommandCutChar() {
  if (editorWidget)
    editorWidget->vimCutChar();
}

void VimHandler::executeCommandEnterInsertMode() { setMode(Mode::Insert); }
void VimHandler::executeCommandEnterVisualMode() { setMode(Mode::Visual); }

void VimHandler::executeCommandExitToNormalMode() {
  if (mode == Mode::Insert && editorWidget) {
    QTextCursor cursor = editorWidget->textCursor();
    if (!cursor.atBlockStart()) {
      cursor.movePosition(QTextCursor::PreviousCharacter);
    }
    editorWidget->setTextCursor(cursor);
  }
  setMode(Mode::Normal);
}

void VimHandler::executeCommandSaveFile() { emit saveFileRequested(); }
void VimHandler::executeCommandSaveFileAs() { emit saveFileAsRequested(); }

} // namespace Jino::Editor::Vim
