#include <sps/qt/aspectwidget.hpp>

AspectRatioWidget::AspectRatioWidget(QWidget *widget,
                                     float width, float height,
                                     QWidget *parent) :
  QWidget(parent), arWidth(width), arHeight(height) {
  layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
  layout->setContentsMargins(0, 0, 0, 0);

  // add spacer, then your widget, then spacer
  layout->addItem(new QSpacerItem(0, 0));
  layout->addWidget(widget);
  layout->addItem(new QSpacerItem(0, 0));
}

void AspectRatioWidget::resizeEvent(QResizeEvent *event) {
  float thisAspectRatio =
    static_cast<float>(event->size().width()) / event->size().height();
  int widgetStretch, outerStretch;

  if (thisAspectRatio > (arWidth/arHeight)) {
    // too wide
    layout->setDirection(QBoxLayout::LeftToRight);
    widgetStretch = height() * (arWidth/arHeight);
    outerStretch = (width() - widgetStretch) / 2 + 0.5;
  } else {
    // too tall
    layout->setDirection(QBoxLayout::TopToBottom);
    widgetStretch = width() * (arHeight/arWidth);
    outerStretch = (height() - widgetStretch) / 2 + 0.5;
  }

  layout->setStretch(0, outerStretch);
  layout->setStretch(1, widgetStretch);
  layout->setStretch(2, outerStretch);
}
