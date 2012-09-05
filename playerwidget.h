#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtConcurrentRun>
#include <mpg123.h>
#include <ao/ao.h>
#include <QTimer>
#include <QSettings>
#include <QLabel>
#include <QMouseEvent>

#define OUTPUT_BUFFER_SIZE 16*1024
#define PRECACHE_SIZE 64*1024
#define CHUNK_SIZE 16*1024
#define HIDE_TIMER_INTERVAL 1000

namespace Ui {
class PlayerWidget;
}

class PlayerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit PlayerWidget(QWidget *parent = 0);
    ~PlayerWidget();
    void setMediaSource(QString new_media_source);
    QLabel volume_label;
    
public slots:
    void on_playPause_clicked();

private slots:
    void decoder();
    void stateChanged();
    void playlist_downloaded();

    void on_volumeSlider_valueChanged(int value);

private:
    Ui::PlayerWidget *ui;
    QString media_source;
    bool starting;
    mpg123_handle* mh;
    ao_device* device;
    QFuture<void> f;
    bool playing;
    bool precaching;
    QNetworkAccessManager m;
    QTimer volume_label_hide_timer;

    void parse_playlist();
    void play();
    void close_ao_device();
    void startPlaying(QString url);
};

#endif // PLAYERWIDGET_H
