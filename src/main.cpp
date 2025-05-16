#include "QtAwesome.h"
#include "app/jino_editor.hpp"
#include "core/constants.hpp"

#include <QApplication>
#include <QColor>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPalette>
#include <QSettings>
#include <QStringList>
#include <QStyleFactory>
#include <QTextStream>

QString loadStyleSheetFromResource(const QString &themeName) {
  QString resourcePath = QString(":/themes/%1.qss").arg(themeName);
  QFile f(resourcePath);
  if (!f.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << "Could not load theme file:" << resourcePath;
    return QString();
  }
  QTextStream ts(&f);
  return ts.readAll();
}

int main(int argc, char *argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  QApplication app(argc, argv);
  QApplication::setOrganizationName(Jino::Constants::APP_ORGANIZATION_NAME);
  QApplication::setApplicationName(Jino::Constants::APP_APPLICATION_NAME);
  QApplication::setApplicationVersion(Jino::Constants::APP_VERSION);
  QApplication::setApplicationDisplayName(Jino::Constants::APP_NAME);

  fa::QtAwesome *awesome = new fa::QtAwesome(&app);
  awesome->initFontAwesome();
  QColor everforestText("#d3c6aa");
  QColor everforestSubtle("#859289");
  awesome->setDefaultOption("color", everforestText);
  awesome->setDefaultOption("color-disabled", everforestSubtle);
  awesome->setDefaultOption("color-active", everforestText);
  awesome->setDefaultOption("color-selected", everforestText);

  QDir defaultDir;
  if (!defaultDir.mkpath(Jino::Constants::DEFAULT_NOTES_DIR)) {
    qWarning() << "Could not create default directory:"
               << Jino::Constants::DEFAULT_NOTES_DIR;
  }

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "Jino Text Editor - Minimalist editor with Vim modes.");
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption nameOption(QStringList() << "n" << "name", "Program name.",
                                "name");
  parser.addOption(nameOption);
  QCommandLineOption classOption(QStringList() << "c" << "class",
                                 "Program class.", "class");
  parser.addOption(classOption);
  parser.addPositionalArgument("files", "Files to open.", "[files...]");
  parser.process(app);

  QString windowTitleOverride = parser.value(nameOption);
  QString windowClassHint = parser.value(classOption);
  QStringList filesToOpen = parser.positionalArguments();

  QSettings settings;
  int lastWorkspaceIndex =
      settings.value(Jino::Constants::SETTINGS_KEY_WORKSPACE_INDEX, -1).toInt();
  int nextWorkspaceIndex =
      (lastWorkspaceIndex + 1) % Jino::Constants::WORKSPACE_SUFFIXES.size();
  QString currentWorkspaceName =
      Jino::Constants::WORKSPACE_PREFIX +
      Jino::Constants::WORKSPACE_SUFFIXES[nextWorkspaceIndex];
  settings.setValue(Jino::Constants::SETTINGS_KEY_WORKSPACE_INDEX,
                    nextWorkspaceIndex);

  QString selectedTheme = Jino::Constants::DEFAULT_THEME;
  QString styleSheet = loadStyleSheetFromResource(selectedTheme);
  app.setStyleSheet(styleSheet);

  QPalette palette = QApplication::palette();
  palette.setColor(QPalette::Window, QColor("#2f383e"));
  palette.setColor(QPalette::WindowText, everforestText);
  palette.setColor(QPalette::Base, QColor("#2f383e"));
  palette.setColor(QPalette::AlternateBase, QColor("#272e33"));
  palette.setColor(QPalette::Text, everforestText);
  palette.setColor(QPalette::Button, QColor("#424d55"));
  palette.setColor(QPalette::ButtonText, everforestText);
  palette.setColor(QPalette::Highlight, QColor("#a7c080"));
  palette.setColor(QPalette::HighlightedText, QColor("#2f383e"));
  palette.setColor(QPalette::ToolTipBase, QColor("#272e33"));
  palette.setColor(QPalette::ToolTipText, everforestText);
  QApplication::setPalette(palette);

  QString desktopBaseName = windowClassHint.isEmpty()
                                ? QFileInfo(argv[0]).baseName()
                                : windowClassHint;
  QGuiApplication::setDesktopFileName(desktopBaseName + ".desktop");

  Jino::App::JinoEditor window(nullptr, currentWorkspaceName, awesome);

  if (!windowTitleOverride.isEmpty()) {
    window.setWindowTitle(windowTitleOverride);
  }

  window.openFilesFromCli(filesToOpen);

  window.show();

  int exitCode = app.exec();
  return exitCode;
}
