#pragma once

#include <QPaintEvent>
#include <QWidget>

class EditorWidget;

class LineNumberWidget : public QWidget {
  Q_OBJECT

public:
  explicit LineNumberWidget(EditorWidget *editor);

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  EditorWidget *codeEditor;
};
