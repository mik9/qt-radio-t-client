#include "playerwidget.h"
#include "ui_playerwidget.h"

PlayerWidget::PlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerWidget)
{
    ui->setupUi(this);
    Phonon::AudioOutput *audioOutput =
        new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(&media_player, audioOutput);
    media_source = "http://stream.radio-t.com:8181/stream.m3u";
    connect(&media_player, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    ui->playPause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
}

PlayerWidget::~PlayerWidget()
{
    delete ui;
}

void PlayerWidget::stateChanged(Phonon::State to, Phonon::State from) {
    if (to == Phonon::ErrorState) {
        qDebug() << "Playback error:" << media_player.errorString();
    }
    if (to == Phonon::PausedState || to == Phonon::StoppedState) {
        ui->playPause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    } else if (to == Phonon::PlayingState) {
        ui->playPause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
    }
}

void PlayerWidget::setMediaSource(QString new_media_source) {
    media_source = new_media_source;
}

void PlayerWidget::parse_playlist() {
    QString s;
    if (media_source.endsWith("m3u")) {
        QNetworkAccessManager m;
        QEventLoop l;
        QNetworkReply *r = m.get(QNetworkRequest(media_source));
        connect(r, SIGNAL(finished()), &l, SLOT(quit()));
        l.exec();
        while (!r->atEnd()) {
            s = r->readLine();
            if (!s.isEmpty() && !s.startsWith("#")) {
                break;
            }
        }
    } else {
        s = media_source;
    }
    QMetaObject::invokeMethod(this, "startPlaying", Qt::QueuedConnection, Q_ARG(QString, s));
}

void PlayerWidget::startPlaying(QString url) {
    media_player.setCurrentSource(Phonon::MediaSource(QUrl(url)));
    media_player.play();
    starting = false;
}

void PlayerWidget::on_playPause_clicked() {
    if (starting) {
        return;
    }
    if (media_player.state() == Phonon::PlayingState) {
        media_player.stop();
    } else {
        starting = true;
        QtConcurrent::run(this, &PlayerWidget::parse_playlist);
    }
}
