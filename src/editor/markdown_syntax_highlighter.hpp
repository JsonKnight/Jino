// src/editor/markdown_syntax_highlighter.hpp
#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

namespace Jino::Editor {

class MarkdownSyntaxHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit MarkdownSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
    int captureGroup = 0;
  };
  QVector<HighlightingRule> highlightingRules;

  QTextCharFormat headingFormat;
  QTextCharFormat boldFormat;
  QTextCharFormat italicFormat;
  QTextCharFormat boldItalicFormat;
  QTextCharFormat strikethroughFormat;
  QTextCharFormat inlineCodeFormat;
  QTextCharFormat linkFormat;
  QTextCharFormat imageFormat;
  QTextCharFormat blockQuoteFormat;

  QRegularExpression codeBlockStartExpression;
  QRegularExpression codeBlockEndExpression;
  QTextCharFormat codeBlockFormat;
};

} // namespace Jino::Editor
