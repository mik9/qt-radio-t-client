#include "customstreamwidget.h"

QColor CustomStreamWidget::normalColor(Qt::white);
QColor CustomStreamWidget::highlightedColor(176, 177, 255);
QColor CustomStreamWidget::clickedColor(Qt::gray);
QColor CustomStreamWidget::activeColor(189, 217, 197);

CustomStreamWidget::CustomStreamWidget(QString _s, QString label, QWidget *parent) :
    QLabel(parent),
    streamUrl(_s)
{
    setText(label);
    setAlignment(Qt::AlignHCenter);
    currentFlags = 0x0;
    connect(&animation, SIGNAL(valueChanged(QVariant)), this, SLOT(update()));
    animation.setTargetObject(this);
    animation.setPropertyName("backgroundColor");
    animation.setDuration(100);
    _backgroundColor = normalColor;
    setMinimumHeight(fontMetrics().height() + 2);
}

CustomStreamWidget::~CustomStreamWidget()
{
}

void CustomStreamWidget::mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        emit change_sream(streamUrl);
        currentFlags |= StateActive;
        updateAnimation();
    }
}

void CustomStreamWidget::paintEvent(QPaintEvent *e) {
    QPainterPath roundRectPath;
    roundRectPath.moveTo(0, 0);
    roundRectPath.lineTo(0, height() - ARC_RADIUS - 1);
    roundRectPath.arcTo(0, height() - ARC_RADIUS * 2 - 1, ARC_RADIUS * 2, ARC_RADIUS * 2, 180, 90);
    roundRectPath.lineTo(width() - ARC_RADIUS - 1, height() - 1);
    roundRectPath.arcTo(width() - ARC_RADIUS * 2 - 1, height() - ARC_RADIUS * 2 - 1, ARC_RADIUS * 2, ARC_RADIUS * 2, 270, 90);
    roundRectPath.lineTo(width() - 1, 0);

    QPainter p(this);
    p.setPen(QColor(Qt::black));
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillPath(roundRectPath, QBrush(_backgroundColor));
    p.drawPath(roundRectPath);

    QLabel::paintEvent(e);
}

bool CustomStreamWidget::event(QEvent *e) {
    if (e->type() == QEvent::Enter) {
        currentFlags |= StateHovered;
        updateAnimation();
    } else if (e->type() == QEvent::Leave) {
        currentFlags ^= StateHovered;
        updateAnimation();
    }
    return QLabel::event(e);
}

void CustomStreamWidget::setNotActive() {
    currentFlags ^= StateActive;
    updateAnimation();
}

void CustomStreamWidget::setActive() {
    currentFlags |= StateActive;
    updateAnimation();
}

bool CustomStreamWidget::equals(QString stream) {
    return streamUrl == stream;
}

void CustomStreamWidget::updateAnimation() {
    QColor _b = normalColor;
    if (currentFlags & StateClicked) {
        _b = clickedColor;
    } else if (currentFlags & StateHovered) {
        _b = highlightedColor;
    } else if (currentFlags & StateActive) {
        _b = activeColor;
    }
    if(animation.state() == QPropertyAnimation::Running) {
        animation.pause();
    }
    animation.setEndValue(_b);
    animation.start();
}
