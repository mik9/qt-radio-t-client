#include "playerwidget.h"
#include "ui_playerwidget.h"

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

    int res = mpg123_init();
    if (res != MPG123_OK) {
        qDebug() << "call mpg123_init() failed";
    }
    mh = mpg123_new(NULL, &res);
    if (res != MPG123_OK) {
        qDebug() << "call mpg123_new() failed";
    }
    ao_initialize();
}

PlayerWidget::~PlayerWidget()
{
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

void PlayerWidget::decoder() {
    if (starting) {
        starting = false;
    }
    QNetworkReply *r = (QNetworkReply*)sender();
    if (!playing) {
        r->close();
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
            startPlaying(s);
            return;
        }
    }
}

void PlayerWidget::startPlaying(QString url) {
    precaching = true;
    QNetworkReply *r = m.get(QNetworkRequest(url));
    connect(r, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(decoder()), Qt::QueuedConnection);
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
