// src/core/constants.hpp
#pragma once

#include <QColor>
#include <QDir>
#include <QFont>
#include <QKeySequence>
#include <QString>
#include <QVector>
#include <vector>

namespace Jino::Constants {

enum class EditorFileType { Text, Org, Markdown };
inline QString editorModeToString(EditorFileType mode) {
  switch (mode) {
  case EditorFileType::Text:
    return QStringLiteral("Text");
  case EditorFileType::Org:
    return QStringLiteral("Org");
  case EditorFileType::Markdown:
    return QStringLiteral("MD");
  default:
    return QStringLiteral("???");
  }
}
inline QVector<EditorFileType> getAllEditorModes() {
  return {EditorFileType::Text, EditorFileType::Org, EditorFileType::Markdown};
}

const QString APP_NAME = "Jino ✨";
const QString APP_ORGANIZATION_NAME = "JinoDev";
const QString APP_APPLICATION_NAME = "JinoEditor";
const QString APP_VERSION = "0.10.0";

const QString FONT_FAMILY = "Dank Mono";
const int FONT_SIZE_PT = 14;
const QString FONT_RESOURCE_PATH = ":/fonts/DankMono.otf";
const int DEFAULT_WINDOW_WIDTH = 800;
const int DEFAULT_WINDOW_HEIGHT = 600;
const int EDITOR_ZOOM_STEP = 1;
const int EDITOR_DEFAULT_ZOOM_PERCENT = 100;
const int EDITOR_MIN_ZOOM_PERCENT = 25;
const int EDITOR_MAX_ZOOM_PERCENT = 500;

const QString DEFAULT_NOTES_DIR = QDir::homePath() + "/Notes/txt";
const QByteArray DEFAULT_FILE_ENCODING = "UTF-8";

const int MAX_TABS_PER_WORKSPACE = 13;

const QString THEME_EVERFOREST = "everforest";
const QString THEME_TOKYO_NIGHT = "tokyo_night";
const QString THEME_NIGHT_OWL = "nightowl";
const QString DEFAULT_THEME = THEME_EVERFOREST;
const QString SETTINGS_KEY_THEME = "interfaceTheme";

inline QKeySequence KeyOpen() { return QKeySequence(QKeySequence::Open); }
inline QKeySequence KeySave() { return QKeySequence(QKeySequence::Save); }
inline QKeySequence KeySaveAs() { return QKeySequence(QKeySequence::SaveAs); }
inline QKeySequence KeyQuit() { return QKeySequence(QKeySequence::Quit); }
inline QKeySequence KeyRedo() { return QKeySequence(QKeySequence::Redo); }
inline QKeySequence KeyUndo() { return QKeySequence(QKeySequence::Undo); }
inline QKeySequence KeyCut() { return QKeySequence(QKeySequence::Cut); }
inline QKeySequence KeyCopy() { return QKeySequence(QKeySequence::Copy); }
inline QKeySequence KeyPaste() { return QKeySequence(QKeySequence::Paste); }
inline QKeySequence KeySelectAll() {
  return QKeySequence(QKeySequence::SelectAll);
}
inline QKeySequence KeyDelete() { return QKeySequence(QKeySequence::Delete); }

const QKeySequence KB_ALT_S = QKeySequence(Qt::ALT | Qt::Key_S);
const QKeySequence KB_ALT_D = QKeySequence(Qt::ALT | Qt::Key_D);
const QKeySequence KB_CTRL_R = QKeySequence(Qt::CTRL | Qt::Key_R);
const QKeySequence KB_ALT_T = QKeySequence(Qt::ALT | Qt::Key_T);
const QKeySequence KB_ALT_LEFT = QKeySequence(Qt::ALT | Qt::Key_Left);
const QKeySequence KB_ALT_RIGHT = QKeySequence(Qt::ALT | Qt::Key_Right);
const QKeySequence KB_ALT_O = QKeySequence(Qt::ALT | Qt::Key_O);
const QKeySequence KB_ALT_Q = QKeySequence(Qt::ALT | Qt::Key_Q);
const QKeySequence KB_ALT_B = QKeySequence(Qt::ALT | Qt::Key_B);
const QKeySequence KB_ALT_R = QKeySequence(Qt::ALT | Qt::Key_R);
const QKeySequence KB_ALT_SHIFT_S =
    QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_S);
const QKeySequence KB_CTRL_PLUS = QKeySequence(Qt::CTRL | Qt::Key_Plus);
const QKeySequence KB_CTRL_EQUAL = QKeySequence(Qt::CTRL | Qt::Key_Equal);
const QKeySequence KB_CTRL_MINUS = QKeySequence(Qt::CTRL | Qt::Key_Minus);
const QKeySequence KB_CTRL_D = QKeySequence(Qt::CTRL | Qt::Key_D);

const char VIM_KEY_LEADER = ' ';
const char VIM_KEY_INSERT_MODE = 'i';
const char VIM_KEY_UNDO = 'u';
const char VIM_KEY_CUT_CHAR = 'x';
const char VIM_KEY_COPY_LINE = 'y';
const char VIM_KEY_DELETE_LINE = 'd';
const char VIM_KEY_PASTE = 'p';
const QString VIM_CMD_DELETE_LINE = "dd";
const QString VIM_CMD_COPY_LINE = "yy";
const QString VIM_LEADER_FILE_SEQUENCE = "f";
const char VIM_LEADER_FILE_SAVE_KEY = 's';
const char VIM_LEADER_FILE_SAVEAS_KEY = 'S';

const QString STATUS_READY = "Ready";
const QString STATUS_FILE_OPENED = "Opened: %1";
const QString STATUS_FILE_SAVED = "Saved: %1";
const QString STATUS_FILE_SAVE_FAILED = "Failed to save: %1";
const QString STATUS_FILE_OPEN_FAILED = "Failed to open: %1";
const QString STATUS_FONT_LOAD_FAILED = "Failed to load font: %1";
const QString STATUS_VIM_MODE_FMT = "%1 %2";
const QString STATUS_EDITOR_MODE_FMT = "%1 %2";
const QString STATUS_POS_LINE_FMT = "%1";
const QString STATUS_POS_COL_FMT = "%1";
const QString STATUS_STATS_CHARS_FMT = "%1 %2";
const QString STATUS_STATS_WORDS_FMT = "%1 %2";
const QString STATUS_TIME_FMT = "%1 %2";
const QString STATUS_ZOOM_FMT = "%1 %2%";
const QString STATUS_MAX_TABS = "Maximum tab limit (%1) reached.";

const QString TOOLTIP_FILE_INFO_FMT =
    "Path: %1\\nSize: %2\\nPerms: %3\\nType: %4";
const QString TOOLTIP_BUFFER_INFO_FMT = "Buffer: %1 (Unsaved)";
const QString TOOLTIP_AUTOSAVE_INFO_FMT = "Buffer: %1 (Autosaved)\\nPath: %2";

const std::vector<QString> ANGEL_NAMES = {
    QStringLiteral("Michael"),   QStringLiteral("Gabriel"),
    QStringLiteral("Raphael"),   QStringLiteral("Uriel"),
    QStringLiteral("Seraphiel"), QStringLiteral("Jegudiel"),
    QStringLiteral("Raguel"),    QStringLiteral("Zerachiel"),
    QStringLiteral("Chamuel"),   QStringLiteral("Jophiel"),
    QStringLiteral("Zadkiel"),   QStringLiteral("Remiel"),
    QStringLiteral("Metatron"),  QStringLiteral("Sandalphon"),
    QStringLiteral("Raziel")};

const std::vector<QString> WORKSPACE_SUFFIXES = {
    QStringLiteral("alpha"), QStringLiteral("beta"), QStringLiteral("gamma"),
    QStringLiteral("delta"), QStringLiteral("epsilon")};
const QString WORKSPACE_PREFIX = "jino_";

const int MAX_RECENT_FILES = 25;
const QString SETTINGS_KEY_RECENT_FILES = "recentFiles";
const QString SETTINGS_KEY_WORKSPACE_INDEX = "workspaceIndex";

const QString INPUT_GOTO_LINE_TITLE = "Go To Line";
const QString INPUT_GOTO_LINE_LABEL = "Line number:";
const QString INPUT_GOTO_COL_TITLE = "Go To Column";
const QString INPUT_GOTO_COL_LABEL = "Column number:";

} // namespace Jino::Constants
