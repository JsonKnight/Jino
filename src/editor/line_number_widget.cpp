#include "editor/line_number_widget.hpp"
#include "editor/editor_widget.hpp"

#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include <QTextBlock>

LineNumberWidget::LineNumberWidget(EditorWidget *editor)
    : QWidget(editor), codeEditor(editor) {
  setObjectName("LineNumberWidget");

  setAutoFillBackground(true);
}

QSize LineNumberWidget::sizeHint() const {
  int digits = 1;
  int maxLines = qMax(1, codeEditor->document()->blockCount());
  while (maxLines >= 10) {
    maxLines /= 10;
    ++digits;
  }
  constexpr int horizontalPadding = 3;
  int space =
      horizontalPadding * 2 +
      codeEditor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
  return QSize(space, 0);
}

void LineNumberWidget::paintEvent(QPaintEvent *event) {
  QPainter painter(this);

  const int currentLine = codeEditor->textCursor().blockNumber();
  const QFontMetrics editorFontMetrics = codeEditor->fontMetrics();
  const int fontHeight = editorFontMetrics.height();
  const int widgetWidth = this->width();
  const int editorScrollY = codeEditor->verticalScrollBar()->value();

  QTextBlock block = codeEditor->document()->firstBlock();
  int blockNumber = block.blockNumber();

  QColor defaultColor = palette().color(QPalette::Text);
  QColor highlightColor = palette().color(QPalette::Highlight);

  while (block.isValid()) {
    QRectF blockRect =
        codeEditor->document()->documentLayout()->blockBoundingRect(block);
    qreal blockViewportTop = blockRect.top() - editorScrollY;
    qreal blockViewportBottom = blockViewportTop + blockRect.height();

    if (blockViewportTop < event->rect().bottom() &&
        blockViewportBottom > event->rect().top()) {
      const QString number = QString::number(blockNumber + 1);

      const QColor textColor =
          (blockNumber == currentLine) ? highlightColor : defaultColor;
      painter.setPen(textColor);

      painter.drawText(0, qRound(blockViewportTop), widgetWidth - 3, fontHeight,
                       Qt::AlignRight, number);
    }

    if (blockViewportTop > event->rect().bottom()) {
      break;
    }

    block = block.next();
    if (!block.isValid())
      break;
    blockNumber = block.blockNumber();
  }
}
