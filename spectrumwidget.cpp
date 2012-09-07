#include "spectrumwidget.h"
#include "sleeper.h"

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
    QPainter p;
    QPoint lastPoint(0,0);
    QPoint mPoint;
    Sleeper::msleep(5);

    while (painting) {
        if (!dataLen) {
            Sleeper::msleep(1);
            continue;
        }

        QElapsedTimer t;t.start();

        dataMutex.lock();

        double sleep_time = ((double)dataLen / _samples_per_frame / _sample_size * _time_per_frame -t.elapsed() ) /FRAMES_NUM;
        for(int i=1;i<=FRAMES_NUM;i++) {
            t.start();

            float s = 0;
            for(int j=dataLen / FRAMES_NUM * (i-1); j < dataLen * i / FRAMES_NUM; j++) {
                if (data[j] > 1) {
                    data[j] = 1;
                } else if (data[j] < -1) {
                    data[j] = 1;
                }
                s+=qAbs(data[j]);
            }
            s /= dataLen / 2 / FRAMES_NUM;
            if (s>1) {
                s=1;
            }

            int y = lastPoint.y() + (height() - height() * s - lastPoint.y()) / (float)FRAMES_NUM * i;
            mPoint = QPoint(width(), y);

            const uchar *oldData = currentImage->bits();
            uchar* _d = bufferImage->bits();
            const int bpl = bufferImage->bytesPerLine();
            const int bpp = bufferImage->depth() / 8;

            for(int j=0;j<bufferImage->height(); j++) {
                memcpy(_d+j*bpl, oldData+j*bpl+bpp*SHIFT_PIXELS, bpl-bpp*SHIFT_PIXELS);
                memset(_d+(j+1)*bpl-bpp*SHIFT_PIXELS, 0, bpp*SHIFT_PIXELS);
            }

            if (lastPoint.x() != 0) {
                lastPoint.setX(lastPoint.x()-SHIFT_PIXELS);
                p.begin(bufferImage);
                p.setPen(QPen(QBrush(Qt::black), 1));
                p.setRenderHint(QPainter::Antialiasing, true);
                p.drawLine(lastPoint, mPoint);
                p.end();
            }

            QLinearGradient gradient(width(), height(), width(), mPoint.y());
            int h;
            for(int j=height()-1; j >= mPoint.y(); j--) {
                h = 120 - 120 * (height() - j) / (float)height() - 20;
                if (h < 0) {
                    h = 0;
                }
                bufferImage->setPixel(width()-1, j, QColor::fromHsv(h, 255,255).rgb());
            }

            lastPoint = mPoint;

            swapBuffers();
            update();
            Sleeper::msleep(sleep_time * 1000 - t.elapsed() + 1);
        }
        dataLen = 0;
        dataMutex.unlock();
    }
}

void SpectrumWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    p.drawPixmap(this->rect(), QPixmap::fromImage(*currentImage, Qt::ColorOnly));
}

void SpectrumWidget::swapBuffers() {
    qSwap(currentImage, bufferImage);
}

void SpectrumWidget::resizeEvent(QResizeEvent *e) {
    QMutexLocker locker(&dataMutex);
    currentImage = new QImage(currentImage->scaled(e->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    bufferImage = new QImage(bufferImage->scaled(e->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
