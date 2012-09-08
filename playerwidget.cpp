#include "playerwidget.h"
#include "ui_playerwidget.h"
#include <sleeper.h>

#if MPG123_API_VERSION < 26
#define mpg123_encsize(mh) 2
#endif
#if MPG123_API_VERSION < 32
#define mpg123_spf(mh) 1104
#endif

class VolumeToolTipHider : public QObject {
public:
    VolumeToolTipHider(PlayerWidget* _w) : w(_w) {}
    static bool mouse_pressed;
private:
    PlayerWidget* w;
protected:
    bool eventFilter(QObject *, QEvent *e) {
        if (e->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* me = (QMouseEvent*)e;
            if (me->button() == Qt::LeftButton) {
                w->volume_label.hide();
                mouse_pressed = false;
            }
        } else if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = (QMouseEvent*)e;
            if (me->button() == Qt::LeftButton) {
                mouse_pressed = true;
            }
        }
        return false;
    }
};
bool VolumeToolTipHider::mouse_pressed(false);

PlayerWidget::PlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerWidget)
{
    ui->setupUi(this);
    media_source = "http://stream.radio-t.com:8181/stream.m3u";
    ui->playPause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    ui->muteButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaVolume));
    starting = false;
    playing = false;
    device = NULL;
    mh = NULL;
    QSettings settings;
    int volume = settings.value("volume", 100).toInt();
    ui->volumeSlider->setValue(volume);

    int res = mpg123_init();
    if (res != MPG123_OK) {
        qDebug() << "call mpg123_init() failed";
    }
    mh = mpg123_new(NULL, &res);
    if (res != MPG123_OK) {
        qDebug() << "call mpg123_new() failed";
    }
    ao_initialize();

    volume_label.setWindowFlags(Qt::ToolTip);

    ui->volumeSlider->installEventFilter(new VolumeToolTipHider(this));

    volume_label_hide_timer.setSingleShot(true);
    connect(&volume_label_hide_timer, SIGNAL(timeout()), &volume_label, SLOT(hide()));
    volume_label_hide_timer.setInterval(HIDE_TIMER_INTERVAL);
}

PlayerWidget::~PlayerWidget()
{
    QSettings settings;
    settings.setValue("volume", ui->volumeSlider->value());
    playing = false;
    f.waitForFinished();
    ao_shutdown();
    mpg123_delete(mh);
    mpg123_exit();
    delete ui;
}

void PlayerWidget::stateChanged() {
    if (!playing) {
        ui->playPause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    } else {
        ui->playPause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
    }
}

void PlayerWidget::setMediaSource(QString new_media_source) {
    media_source = new_media_source;
}

void PlayerWidget::streaming_finished() {
    playing = false;
    stateChanged();
    QNetworkReply *r = (QNetworkReply*)sender();
    r->deleteLater();
}

void PlayerWidget::decoder() {
    if (starting) {
        starting = false;
    }
    QNetworkReply *r = (QNetworkReply*)sender();
    if (!playing) {
        r->close();
        r->deleteLater();
        return;
    }
    if (precaching && r->bytesAvailable() < PRECACHE_SIZE){
        return;
    } else {
        precaching = false;
    }
    if (r->bytesAvailable() < CHUNK_SIZE) {
        return;
    }
    QByteArray b = r->readAll();
    mpg123_feed(mh, reinterpret_cast<unsigned char*>(b.data()), b.size());
    if (device == NULL) {
        long rate;
        int ch, enc;
        mpg123_getformat(mh, &rate, &ch, &enc);
        int sampleSize = mpg123_encsize(enc);
        ui->spectrum->setSampleSize(sampleSize);
        mpg123_volume(mh, ui->volumeSlider->value()/100.0);
        ao_sample_format format;
        memset(&format, 0, sizeof(format));
        format.channels = ch;
        format.rate = rate;
        format.bits = sampleSize * 8;
        format.byte_format = AO_FMT_NATIVE;
        int driver_id = -1;
#ifdef Q_OS_LINUX
        driver_id = ao_driver_id("pulse");
#endif
        if (driver_id == -1) {
            // Fallback to default driver
             driver_id = ao_default_driver_id();
        }
        if (driver_id == -1) {
            qDebug() << "Cannot open audio driver!!!";
        }
        device = ao_open_live(driver_id, &format, NULL);
        f = QtConcurrent::run(this, &PlayerWidget::play);
    }
}

const quint16 PCMS16MaxAmplitude =  32768; // because minimum is -32768
inline qreal PlayerWidget::pcmToReal(qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}

void PlayerWidget::emitter(unsigned char *data, size_t len) {
    float *f = new float[len/2];
    for(size_t i=0; i < len; i+=2) {
        const qint16 pcmSample = *reinterpret_cast<const qint16*>(data+i);
        f[i/2] = qAbs(pcmToReal(pcmSample));
    }
    ui->spectrum->new_data(f, len/2);
    delete [] data;
}

bool _f = true;
void PlayerWidget::play() {
    size_t bytes;
    int res;
    unsigned char* data = new unsigned char[OUTPUT_BUFFER_SIZE];
    while (playing) {
        res = mpg123_read(mh, data, OUTPUT_BUFFER_SIZE, &bytes);
        if (res == MPG123_OK) {
            unsigned char* data2 = new unsigned char[bytes];
            memcpy(data2, data, bytes);
            QtConcurrent::run(this, &PlayerWidget::emitter, data2, bytes);
            if (_f) {
                ui->spectrum->setSPF(mpg123_spf(mh));
                ui->spectrum->setTPF(mpg123_tpf(mh));
                _f = false;
            }
            ao_play(device, reinterpret_cast<char*>(data), bytes);
        } else {
            Sleeper::msleep(10);
            precaching = true;
        }
    }
    ao_close(device);
    device = NULL;
    precaching = true;
}

void PlayerWidget::parse_playlist() {
    if (media_source.endsWith("m3u")) {
        QNetworkReply *r = m.get(QNetworkRequest(media_source));
        connect(r, SIGNAL(finished()), this, SLOT(playlist_downloaded()));
    } else {
        startPlaying(media_source);
    }
}

void PlayerWidget::playlist_downloaded() {
    QString s;
    QNetworkReply* r = (QNetworkReply*)sender();
    while (!r->atEnd()) {
        s = QString(r->readLine()).replace("\r\n", "");
        if (!s.isEmpty() && !s.startsWith("#")) {
            break;
        }
    }
    r->deleteLater();
    startPlaying(s);
}

void PlayerWidget::startPlaying(QString url) {
    precaching = true;
    QNetworkReply *r = m.get(QNetworkRequest(url));
    connect(r, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(decoder()), Qt::QueuedConnection);
    connect(r, SIGNAL(finished()), this, SLOT(streaming_finished()));
}

void PlayerWidget::on_playPause_clicked() {
    if (starting) {
        return;
    }
    if (!playing) {
        playing = true;
        mpg123_open_feed(mh);
        parse_playlist();
        stateChanged();
    } else {
        playing = false;
        mpg123_close(mh);
        ui->playPause->setEnabled(false);
        QtConcurrent::run(this, &PlayerWidget::close_ao_device);
    }
}
void PlayerWidget::close_ao_device() {
    f.waitForFinished();
    ui->playPause->setEnabled(true);
    QMetaObject::invokeMethod(this, "stateChanged");
}

void PlayerWidget::on_volumeSlider_valueChanged(int value)
{
    QPoint pos = QCursor::pos();
    pos.setX(pos.x() + 15);
    volume_label.move(pos);
    volume_label.setText(QString::number(value));
    volume_label.resize(volume_label.fontMetrics().width(volume_label.text())+6, volume_label.fontMetrics().height()+6);
    volume_label.setMargin(3);
    if (volume_label.isHidden()) {
        volume_label.show();
    }
    if (!VolumeToolTipHider::mouse_pressed) {
        volume_label_hide_timer.start();
    }
    if (mh) {
        mpg123_volume(mh, value/100.0);
    }
}
void PlayerWidget::on_muteButton_toggled(bool checked) {
    if (checked) {
        double d1,d2;
        mpg123_getvolume(mh, &last_volume, &d1, &d2);
        mpg123_volume(mh, 0);
        ui->muteButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    } else {
        mpg123_volume(mh, last_volume);
        ui->muteButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaVolume));
    }
}
