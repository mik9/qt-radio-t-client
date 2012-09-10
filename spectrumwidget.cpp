#include "spectrumwidget.h"
#include "sleeper.h"
#include <qmath.h>

SpectrumWidget::SpectrumWidget(QWidget *parent) :
    QWidget(parent)
{
    currentImage = new QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    currentImage->fill(Qt::transparent);
    bufferImage = new QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    bufferImage->fill(Qt::transparent);
    painting = true;
    dataLen = 0;
    this->setMaximumSize(400,100);
    this->setMinimumSize(400,100);
    new_bottom = bottom_color = rand() % 255;
    new_top = top_color = rand() % 255;
    data = NULL;
    connect(&change_color_timer, SIGNAL(timeout()), this, SLOT(change_color()));
    change_color_timer.start(200);
    new_color();
    __fps = FPS_NORMAL;
}

void SpectrumWidget::main_window_focus_changed(bool focused) {
    if (focused) {
        __fps = FPS_NORMAL;
    } else {
        __fps = FPS_SLOW;
    }
}

void SpectrumWidget::new_color() {
    new_bottom = rand() % 255;
    new_top = rand() % 255;
}

void SpectrumWidget::change_color() {
    if( top_color != new_top ) {
        if (new_top < top_color) {
            top_color--;
        } else {
            top_color++;
        }
    }
    if ( new_bottom != bottom_color) {
        if (new_bottom < bottom_color) {
            bottom_color--;
        } else {
            bottom_color++;
        }
    }
    if (new_bottom == bottom_color && top_color == new_top) {
        new_color();
    }
}

SpectrumWidget::~SpectrumWidget()
{
    painting = false;
    painterResult.waitForFinished();
}

void SpectrumWidget::new_data(float *_data, size_t len) {
    QMutexLocker locker(&dataMutex);
    if(data) {
        delete [] data;
    } else {
        painterResult = QtConcurrent::run(this, &SpectrumWidget::paintFunction);
    }
    data = _data;
    dataLen = len;
}

void SpectrumWidget::paintFunction() {
#ifndef QT_NO_DEBUG
    qDebug() << "Thread started.";
#endif
    QPainter p;
    QPoint lastPoint(width() - 1,height() - 1);
    QPoint mPoint;
    Sleeper::msleep(5);

    int colorS = 240;
    int colorV = 240;

    while (painting) {
        if (!dataLen) {
            Sleeper::msleep(1);
            continue;
        }

        QElapsedTimer t;t.start();

        dataMutex.lock();
#ifndef QT_NO_DEBUG
        qDebug() << "locked";
#endif
        const double data_time = (double)dataLen / _samples_per_frame / _sample_size * _time_per_frame;
        const int frame_num = qCeil(data_time * __fps);
        const double sleep_time = (data_time - t.elapsed() ) / frame_num;
        const int shift_pixels = SHIFT_PIXELS_PER_SECOND / __fps;

        const int height = bufferImage->height();
        for(int i=1;i<=frame_num;i++) {
            t.start();

            float s = 0;
#ifndef QT_NO_DEBUG
            qDebug() << "1:" << dataLen / frame_num * (i-1) << dataLen * i / frame_num;
#endif
            for(size_t j=dataLen / frame_num * (i-1); j < dataLen * i / frame_num; j++) {
                if (data[j] > 1) {
                    data[j] = 1;
                } else if (data[j] < -1) {
                    data[j] = 1;
                }
                s+=qAbs(data[j]);
            }
            s /= dataLen / 2 / frame_num;
            if (s>1) {
                s=1;
            }

            int y = lastPoint.y() + (height - height * s - lastPoint.y()) / (float)frame_num * i;
            mPoint = QPoint(width(), y);

            const uchar *oldData = currentImage->bits();
            uchar* _d = bufferImage->bits();
            const int bpl = bufferImage->bytesPerLine();
            const int bpp = bufferImage->depth() / 8;
#ifndef QT_NO_DEBUG
            qDebug() << "2:" << 0 << height;
#endif
            for(int j=0;j<height; j++) {
                memcpy(_d+j*bpl, oldData+j*bpl+bpp*shift_pixels, bpl-bpp*shift_pixels);
                memset(_d+(j+1)*bpl-bpp*shift_pixels, 0, bpp*shift_pixels);
            }

#ifndef QT_NO_DEBUG
            qDebug() << "3:" << height-1 << mPoint.y();
#endif
            int h;
            for(int j=height-1; j >= mPoint.y(); j--) {
                const QColor _bottom = QColor::fromHsv(bottom_color, colorS, colorV);
                const QColor _top = QColor::fromHsv(top_color, colorS, colorV);

                const uchar r = _bottom.red() - (_bottom.red() - _top.red()) * (height - j) / (float)height;
                const uchar g = _bottom.green() - (_bottom.green() - _top.green()) * (height - j) / (float)height;
                const uchar b = _bottom.blue() - (_bottom.blue() - _top.blue()) * (height - j) / (float)height;
                h = bottom_color - (bottom_color - top_color) * (height - j) / (float)height;

                QRgb pixel = qRgb(r,g,b);
                for (int z=1;z<=shift_pixels;z++) {
                    bufferImage->setPixel(width()-z, j, pixel);
                }
            }

            lastPoint.setX(lastPoint.x()-shift_pixels);
            p.begin(bufferImage);
            p.setPen(QPen(QBrush(Qt::black), 1));
            p.setRenderHint(QPainter::Antialiasing, true);
            p.drawLine(lastPoint, mPoint);
            p.end();

            lastPoint = mPoint;

            swapBuffers();
#ifndef QT_NO_DEBUG
            qDebug() << "sleeping for" << sleep_time * 1000 << "-" << t.elapsed();
#endif
            Sleeper::msleep(sleep_time * 1000 - t.elapsed());
        }
        dataLen = 0;
        dataMutex.unlock();
#ifndef QT_NO_DEBUG
        qDebug() << "unlocked";
#endif
    }
}

void SpectrumWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    p.drawPixmap(this->rect(), QPixmap::fromImage(*currentImage, Qt::ColorOnly));
}

void SpectrumWidget::swapBuffers() {
    qSwap(currentImage, bufferImage);
    update();
}

void SpectrumWidget::resizeEvent(QResizeEvent *e) {
    QMutexLocker locker(&dataMutex);
    currentImage = new QImage(currentImage->scaled(e->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    bufferImage = new QImage(bufferImage->scaled(e->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void SpectrumWidget::mousePressEvent(QMouseEvent *) {
    painting = !painting;
    if (painting) {
        painterResult = QtConcurrent::run(this, &SpectrumWidget::paintFunction);
    } else {
        painterResult.waitForFinished();
        bufferImage->fill(Qt::transparent);
        currentImage->fill(Qt::transparent);
        update();
    }
}
