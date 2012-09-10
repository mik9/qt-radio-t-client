#ifndef CUSTOMSTREAMWIDGET_H
#define CUSTOMSTREAMWIDGET_H

#include <QLabel>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPropertyAnimation>

#define ARC_RADIUS 5

class CustomStreamWidget : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
public:
    explicit CustomStreamWidget(QString _s, QString label, QWidget *parent = 0);
    ~CustomStreamWidget();

    void setNotActive();
    void setActive();
    bool equals(QString stream);

signals:
    void change_sream(QString);

private:
    QString streamUrl;
    QPropertyAnimation animation;
    enum StateFlags {
        StateActive = 0x1,
        StateClicked = 0x2,
        StateHovered = 0x4
    };
    quint8 currentFlags;

    QColor _backgroundColor;
    void setBackgroundColor(QColor c) {
        _backgroundColor = c;
    }
    QColor backgroundColor() const { return _backgroundColor;}
    void updateAnimation();

    static QColor normalColor;
    static QColor highlightedColor;
    static QColor clickedColor;
    static QColor activeColor;

protected:
    void mousePressEvent(QMouseEvent *ev);
    bool event(QEvent *e);
    void paintEvent(QPaintEvent *);
};

#endif // CUSTOMSTREAMWIDGET_H
