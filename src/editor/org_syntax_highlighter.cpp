#include "editor/org_syntax_highlighter.hpp"
#include <QColor>
#include <QDebug>
#include <QFont>

namespace Jino::Editor {

namespace {
const QColor OrgHeadline1Color = QColor("#E0AF68");
const QColor OrgHeadline2Color = QColor("#7AA2F7");
const QColor OrgHeadline3Color = QColor("#9ECE6A");
const QColor OrgHeadlineOtherColor = QColor("#C0CAF5");
const QColor OrgLinkColor = QColor("#BB9AF7");
const QColor OrgCodeColor = QColor("#7DCFFF");
const QColor OrgBoldColor = QColor("#F7768E");
const QColor OrgItalicColor = QColor("#E0AF68");
} // namespace

OrgSyntaxHighlighter::OrgSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
  HighlightingRule rule;

  headlineFormat[0].setForeground(OrgHeadline1Color);
  headlineFormat[0].setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\\\*\\\\s+.*"));
  rule.format = headlineFormat[0];
  rule.captureGroup = 0;
  highlightingRules.append(rule);

  headlineFormat[1].setForeground(OrgHeadline2Color);
  headlineFormat[1].setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\\\*\\\\*\\\\s+.*"));
  rule.format = headlineFormat[1];
  rule.captureGroup = 0;
  highlightingRules.append(rule);

  headlineFormat[2].setForeground(OrgHeadline3Color);
  headlineFormat[2].setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\\\*\\\\*\\\\*\\\\s+.*"));
  rule.format = headlineFormat[2];
  rule.captureGroup = 0;
  highlightingRules.append(rule);

  headlineFormat[3].setForeground(OrgHeadlineOtherColor);
  headlineFormat[3].setFontWeight(QFont::Bold);
  rule.pattern =
      QRegularExpression(QStringLiteral("^\\\\*\\\\*\\\\*\\\\*+\\\\s+.*"));
  rule.format = headlineFormat[3];
  rule.captureGroup = 0;
  highlightingRules.append(rule);

  boldFormat.setForeground(OrgBoldColor);
  boldFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral(
      "(?<![*\\\\w])\\\\*([^\\\\s*].*?[^\\\\s*])\\\\*(?![*\\\\w])"));
  rule.format = boldFormat;
  rule.captureGroup = 1;
  highlightingRules.append(rule);

  italicFormat.setForeground(OrgItalicColor);
  italicFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral(
      "(?<![/\\\\w])\\\\/([^\\\\s/].*?[^\\\\s/])\\\\/(?![/\\\\w])"));
  rule.format = italicFormat;
  rule.captureGroup = 1;
  highlightingRules.append(rule);

  codeFormat.setForeground(OrgCodeColor);
  codeFormat.setFontFamily("monospace");
  rule.pattern =
      QRegularExpression(QStringLiteral("[=~]([^=~\\\\s].*?[^=~\\\\s])[=~]"));
  rule.format = codeFormat;
  rule.captureGroup = 1;
  highlightingRules.append(rule);

  linkFormat.setForeground(OrgLinkColor);
  linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

  rule.pattern = QRegularExpression(QStringLiteral(
      "\\\\\\\\\\\\[\\\\\\\\\\\\[([^\\\\\\\\\\\\]]+?)(\\\\\\\\\\\\]"
      "\\\\\\\\\\\\[([^\\\\\\\\\\\\]]+?))?\\\\\\\\\\\\]\\\\\\\\\\\\]"));
  rule.format = linkFormat;
  rule.captureGroup = 0;
  highlightingRules.append(rule);
}

void OrgSyntaxHighlighter::highlightBlock(const QString &text) {

  for (int i = 0; i < 4; ++i) {
    QRegularExpressionMatch match = highlightingRules[i].pattern.match(text);
    if (match.hasMatch() && match.capturedStart() == 0) {
      setFormat(0, text.length(), highlightingRules[i].format);
      return;
    }
  }

  for (const HighlightingRule &rule : qAsConst(highlightingRules).mid(4)) {
    QRegularExpressionMatchIterator matchIterator =
        rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      if (rule.captureGroup == 0) {
        setFormat(match.capturedStart(0), match.capturedLength(0), rule.format);
      } else if (match.lastCapturedIndex() >= rule.captureGroup) {
        setFormat(match.capturedStart(rule.captureGroup),
                  match.capturedLength(rule.captureGroup), rule.format);
      }
    }
  }
  setCurrentBlockState(0);
}

} // namespace Jino::Editor
