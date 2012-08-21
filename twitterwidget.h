#ifndef TWITTERWIDGET_H
#define TWITTERWIDGET_H

#include <QWidget>
#include <oauthtwitter.h>
#include <qtweetsearch.h>
#include <qtweetsearchpageresults.h>
#include <qtweetsearchresult.h>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <time.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QRegExp>
#include <QTextBrowser>
#include <QGraphicsDropShadowEffect>
#include <QDateTime>

#define UPDATE_INTERVAL 60 * 1000
#define MINUTE 60
#define HOUR (MINUTE * 60)
#define DAY (HOUR * 24)
#define MONTH (DAY * 30)
#define YEAR (DAY * 365)

class TwitterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TwitterWidget(QWidget *parent = 0);
    ~TwitterWidget();

    void setHashtag(QString h) {
        hashtag = h;
    }

signals:
    
public slots:
    
private:
    QTweetSearch *mTwitterSearch;
    OAuthTwitter* mOauthTwiteer;
    QTimer mUpdateTimer;
    QTimer mRefreshTimer;
    QList<QTweetSearchResult> results;
    QWidget* slidingWidget;
    QNetworkAccessManager NAManager;
    QPropertyAnimation mAnim;
    QString hashtag;

    QLabel nickName;
    QLabel avatar;
    QLabel text;
    QLabel timeStamp;
    QPixmap nextPixmap;
    int id;
    bool event(QEvent *);

    static QRegExp userRegExp;
    static QRegExp urlRegExp;
    static QRegExp hashTagRegExp;

    QGraphicsDropShadowEffect* shadows[4];


private slots:
    void searchFinished(QTweetSearchPageResults r);
    void startUpdate();
    void startRefresh();
    void finishRefresh();
    void pictureDownloaded();

};

#endif // TWITTERWIDGET_H
