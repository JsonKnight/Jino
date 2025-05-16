#pragma once

#include "editor/vim/vim_modes.hpp"

#include <QObject>
#include <QString>

class QKeyEvent;
class EditorWidget;

namespace Jino::Editor::Vim {

class VimHandler : public QObject {
  Q_OBJECT

public:
  explicit VimHandler(EditorWidget *editor);

  Mode currentMode() const;
  bool handleKeyPress(QKeyEvent *event);

signals:
  void modeChanged(Jino::Editor::Vim::Mode newMode);
  void saveFileRequested();
  void saveFileAsRequested();

private:
  void setMode(Mode newMode);
  void processNormalModeKey(QKeyEvent *event);
  void processVisualModeKey(QKeyEvent *event);
  void processInsertModeKey(QKeyEvent *event);
  void processCommandBuffer();
  void clearCommandBuffer();
  void resetLeaderState();

  void executeCommandDeleteLine();
  void executeCommandCopyLine();
  void executeCommandCutSelection();
  void executeCommandCopySelection();
  void executeCommandPaste();
  void executeCommandUndo();
  void executeCommandCutChar();
  void executeCommandEnterInsertMode();
  void executeCommandEnterVisualMode();
  void executeCommandExitToNormalMode();
  void executeCommandSaveFile();
  void executeCommandSaveFileAs();

  EditorWidget *editorWidget;
  Mode mode = Mode::Insert;
  QString commandBuffer;

  bool waitingForFileLeaderKey = false;
  bool waitingForFileSaveKey = false;
};

} // namespace Jino::Editor::Vim
