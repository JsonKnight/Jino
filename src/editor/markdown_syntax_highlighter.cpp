#include "editor/markdown_syntax_highlighter.hpp"
#include <QColor>
#include <QDebug>
#include <QFont>

namespace Jino::Editor {

namespace {
const QColor MdHeadingColor = QColor("#7AA2F7");
const QColor MdLinkColor = QColor("#BB9AF7");
const QColor MdCodeColor = QColor("#7DCFFF");
const QColor MdBoldColor = QColor("#F7768E");
const QColor MdItalicColor = QColor("#E0AF68");
const QColor MdStrikeColor = QColor("#565f89");
const QColor MdQuoteColor = QColor("#9ECE6A");
} // namespace

MarkdownSyntaxHighlighter::MarkdownSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
  HighlightingRule rule;

  headingFormat.setForeground(MdHeadingColor);
  headingFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^(#+)\\\\s+.*"));
  rule.format = headingFormat;

  rule.captureGroup = 0;
  highlightingRules.append(rule);

  blockQuoteFormat.setForeground(MdQuoteColor);
  blockQuoteFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("^[> ]+"));
  rule.format = blockQuoteFormat;
  rule.captureGroup = 0;
  highlightingRules.append(rule);

  linkFormat.setForeground(MdLinkColor);
  linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
  rule.pattern = QRegularExpression(QStringLiteral(
      "!?\\\\\\\\[[^\\\\\\\\]]*\\\\\\\\]\\\\\\\\([^\\\\\\\\)]*\\\\\\\\)"));
  rule.format = linkFormat;
  rule.captureGroup = 0;
  highlightingRules.append(rule);
  imageFormat = linkFormat;

  boldItalicFormat.setForeground(MdBoldColor);
  boldItalicFormat.setFontWeight(QFont::Bold);
  boldItalicFormat.setFontItalic(true);
  rule.pattern =
      QRegularExpression(QStringLiteral("(\\\\*\\\\*\\\\*|___)(.+?)(\\\\1)"));
  rule.format = boldItalicFormat;
  rule.captureGroup = 2;
  highlightingRules.append(rule);

  boldFormat.setForeground(MdBoldColor);
  boldFormat.setFontWeight(QFont::Bold);
  rule.pattern =
      QRegularExpression(QStringLiteral("(\\\\*\\\\*|__)(.+?)(\\\\1)"));
  rule.format = boldFormat;
  rule.captureGroup = 2;
  highlightingRules.append(rule);

  italicFormat.setForeground(MdItalicColor);
  italicFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(
      QStringLiteral("(?<![*_])([*_])(?!\\\\s)(.+?)(?<!\\\\s)(\\\\1)(?![*_])"));
  rule.format = italicFormat;
  rule.captureGroup = 2;
  highlightingRules.append(rule);

  strikethroughFormat.setForeground(MdStrikeColor);
  strikethroughFormat.setFontStrikeOut(true);
  rule.pattern = QRegularExpression(QStringLiteral("(~~)(.+?)(~~)"));
  rule.format = strikethroughFormat;
  rule.captureGroup = 2;
  highlightingRules.append(rule);

  inlineCodeFormat.setForeground(MdCodeColor);
  inlineCodeFormat.setFontFamily("monospace");
  rule.pattern = QRegularExpression(QStringLiteral("(`)(.+?)(`)"));
  rule.format = inlineCodeFormat;
  rule.captureGroup = 2;
  highlightingRules.append(rule);

  codeBlockFormat.setForeground(MdCodeColor);
  codeBlockFormat.setFontFamily("monospace");
  codeBlockStartExpression = QRegularExpression(QStringLiteral("^(```|~~~)"));
  codeBlockEndExpression = QRegularExpression(QStringLiteral("^(```|~~~)"));
}

void MarkdownSyntaxHighlighter::highlightBlock(const QString &text) {

  setCurrentBlockState(0);
  int previousState = previousBlockState();
  int codeBlockStartIndex = -1;
  int scanPosition = 0;

  if (previousState == 1) {
    QRegularExpressionMatch endMatch = codeBlockEndExpression.match(text);
    if (endMatch.hasMatch() && endMatch.capturedStart() == 0) {
      setFormat(0, endMatch.capturedLength(), codeBlockFormat);
      setCurrentBlockState(0);
      scanPosition = endMatch.capturedLength();
    } else {
      setFormat(0, text.length(), codeBlockFormat);
      setCurrentBlockState(1);
      return;
    }
  }

  while (scanPosition < text.length()) {
    QRegularExpressionMatch startMatch =
        codeBlockStartExpression.match(text, scanPosition);
    if (!startMatch.hasMatch())
      break;

    codeBlockStartIndex = startMatch.capturedStart();
    QRegularExpressionMatch endMatch = codeBlockEndExpression.match(
        text, codeBlockStartIndex + startMatch.capturedLength());
    int codeBlockEndIndex = -1;
    int codeBlockEndLength = 0;

    if (endMatch.hasMatch()) {
      codeBlockEndIndex = endMatch.capturedStart();
      codeBlockEndLength = endMatch.capturedLength();
      setFormat(codeBlockStartIndex,
                (codeBlockEndIndex + codeBlockEndLength) - codeBlockStartIndex,
                codeBlockFormat);
      scanPosition = codeBlockEndIndex + codeBlockEndLength;
      setCurrentBlockState(0);
    } else {
      setFormat(codeBlockStartIndex, text.length() - codeBlockStartIndex,
                codeBlockFormat);
      setCurrentBlockState(1);
      scanPosition = text.length();
    }
  }

  if (currentBlockState() == 0) {
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
      QRegularExpressionMatchIterator matchIterator =
          rule.pattern.globalMatch(text);
      while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();

        bool inCodeBlock = false;
        for (const QTextLayout::FormatRange &range :
             currentBlock().layout()->formats()) {
          if (range.format == codeBlockFormat) {
            if (match.capturedStart() >= range.start &&
                (match.capturedStart() + match.capturedLength()) <=
                    (range.start + range.length)) {
              inCodeBlock = true;
              break;
            }
          }
        }
        if (inCodeBlock)
          continue;

        if (rule.captureGroup == 0) {
          setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        } else if (match.lastCapturedIndex() >= rule.captureGroup) {

          setFormat(match.capturedStart(rule.captureGroup),
                    match.capturedLength(rule.captureGroup), rule.format);
        }
      }
    }
  }
}

} // namespace Jino::Editor
