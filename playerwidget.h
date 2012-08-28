#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <phonon/MediaObject>
#include <phonon/AudioOutput>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtConcurrentRun>

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
    
public slots:
    void on_playPause_clicked();

private slots:
    void stateChanged(Phonon::State from, Phonon::State to);
    void startPlaying(QString url);

private:
    Ui::PlayerWidget *ui;
    Phonon::MediaObject media_player;
    QString media_source;
    bool starting;

    void parse_playlist();
};

#endif // PLAYERWIDGET_H
