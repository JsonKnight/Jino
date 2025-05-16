// src/editor/org_syntax_highlighter.hpp
#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

namespace Jino::Editor {

class OrgSyntaxHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit OrgSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
    int captureGroup = 0;
  };
  QVector<HighlightingRule> highlightingRules;

  QTextCharFormat headlineFormat[4];
  QTextCharFormat boldFormat;
  QTextCharFormat italicFormat;
  QTextCharFormat codeFormat;
  QTextCharFormat linkFormat;
};

} // namespace Jino::Editor
