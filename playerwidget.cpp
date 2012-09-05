#include "playerwidget.h"
#include "ui_playerwidget.h"

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
        mpg123_volume(mh, ui->volumeSlider->value()/100.0);
        ao_sample_format format;
        memset(&format, 0, sizeof(format));
        format.channels = ch;
        format.rate = rate;
        format.bits = 16;
        format.byte_format = AO_FMT_NATIVE;
        device = ao_open_live(ao_default_driver_id(), &format, NULL);
        f = QtConcurrent::run(this, &PlayerWidget::play);
    }
}

void PlayerWidget::play() {
    size_t bytes;
    int res;
    unsigned char* data = new unsigned char[OUTPUT_BUFFER_SIZE];
    while (playing) {
        res = mpg123_read(mh, data, OUTPUT_BUFFER_SIZE, &bytes);
        if (res == MPG123_OK) {
            ao_play(device, reinterpret_cast<char*>(data), bytes);
        } else {
            QEventLoop loop; QTimer::singleShot(10, &loop, SLOT(quit())); loop.exec();
        }
    }
    ao_close(device);
    device = NULL;
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
