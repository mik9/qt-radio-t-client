#include "twitterwidget.h"

QRegExp TwitterWidget::userRegExp("@([\\w_]+)(\\s?)");
QRegExp TwitterWidget::urlRegExp("(https?://\\S+)");
QRegExp TwitterWidget::hashTagRegExp("#(\\S+)(\\s?)");

TwitterWidget::TwitterWidget(QWidget *parent) :
    QWidget(parent)
{
    srand(time(NULL));
    mOauthTwiteer = new OAuthTwitter(this);
    mOauthTwiteer->setNetworkAccessManager(&NAManager);

    mTwitterSearch = new QTweetSearch(mOauthTwiteer, mOauthTwiteer);
    mTwitterSearch->setAuthenticationEnabled(false);

    connect(mTwitterSearch, SIGNAL(parsedPageResults(QTweetSearchPageResults)), this, SLOT(searchFinished(QTweetSearchPageResults)));

    connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(startUpdate()));

    mUpdateTimer.start(UPDATE_INTERVAL);

    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(startRefresh()));

    slidingWidget = new QWidget(this);

    mAnim.setTargetObject(slidingWidget);
    mAnim.setPropertyName("pos");
    mAnim.setDuration(700);

    QHBoxLayout *l1 = new QHBoxLayout(slidingWidget);
    l1->addWidget(&avatar);
    QVBoxLayout *l2 = new QVBoxLayout();
    l2->addWidget(&nickName);
    l2->addWidget(&text);
    l2->addWidget(&timeStamp);
    l1->addLayout(l2);
    text.setWordWrap(true);
    text.setOpenExternalLinks(true);
    nickName.setOpenExternalLinks(true);
    timeStamp.setOpenExternalLinks(true);

    for(int i=0;i<4;i++) {
        shadows[i] = new QGraphicsDropShadowEffect();
        shadows[i]->setBlurRadius(30);
        shadows[i]->setColor(Qt::black);
        shadows[i]->setOffset(0, 0);
    }
    text.setGraphicsEffect(shadows[0]);
    nickName.setGraphicsEffect(shadows[1]);
    avatar.setGraphicsEffect(shadows[2]);
    timeStamp.setGraphicsEffect(shadows[3]);

    text.setMinimumHeight(text.fontMetrics().height()*3);
    nickName.setMinimumHeight(nickName.fontMetrics().height());
    nickName.setMaximumHeight(nickName.fontMetrics().height());

    timeStamp.setMinimumHeight(timeStamp.fontMetrics().height());
    timeStamp.setMaximumHeight(timeStamp.fontMetrics().height());
    text.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    avatar.setAlignment(Qt::AlignLeft | Qt::AlignTop);

    this->setMinimumSize(370, 95);
    this->slidingWidget->layout()->setContentsMargins(15,15,15,15);

    hashtag = "#radiot";
}

void TwitterWidget::searchFinished(QTweetSearchPageResults r) {
    results = r.results();

    if (!mRefreshTimer.isActive()) {
        mRefreshTimer.start(8 * 1000);
        startRefresh();
    }
}

void TwitterWidget::startUpdate() {
    mTwitterSearch->start(hashtag, "", 100);
}

TwitterWidget::~TwitterWidget() {
    delete mTwitterSearch;
}

bool TwitterWidget::event(QEvent *e) {
    if (e->type() == QEvent::Resize) {
        slidingWidget->setGeometry(QRect(slidingWidget->pos(), this->size()));
    } else if (e->type() == e->Show) {
        mUpdateTimer.start();
        if (results.isEmpty()) {
            startUpdate();
        }
    } else if (e->type() == e->Hide) {
        mUpdateTimer.stop();
    }
    return QWidget::event(e);
}

void TwitterWidget::startRefresh() {
    if (results.isEmpty()) {
        return;
    }
    id = rand() % results.length();
    QNetworkReply *r = NAManager.get(QNetworkRequest(QUrl(results.at(id).profileImageUrl())));
    connect(r, SIGNAL(finished()), this, SLOT(pictureDownloaded()));
}

void TwitterWidget::pictureDownloaded() {
    QNetworkReply *r = (QNetworkReply*)sender();
    nextPixmap.loadFromData(r->readAll());

    mAnim.setStartValue(slidingWidget->pos());
    mAnim.setEndValue(QPoint(0, -150));
    mAnim.setEasingCurve(QEasingCurve::InBack);
    connect(&mAnim, SIGNAL(finished()), this, SLOT(finishRefresh()));
    mAnim.start();
}


QString choose_from_text(int n, int type) {
    static char* some[6][3] = {
        {"год", "года", "лет"},
        {"месяц", "месяца", "месяцев"},
        {"день", "дня", "дней"},
        {"час", "часа", "часов"},
        {"минута", "минуты", "минут"},
        {"секунда", "секунды", "секунд"}
    };
    if (n > 20) {
        n %= 20;
    }
    if (n == 1) {
        return QString::fromUtf8(some[type][0]);
    } else if (n >= 2 && n <= 4) {
        return QString::fromUtf8(some[type][1]);
    } else {
        return QString::fromUtf8(some[type][2]);
    }
}

void TwitterWidget::finishRefresh() {
    avatar.setPixmap(nextPixmap.copy());
    avatar.setMaximumSize(nextPixmap.size());
    nickName.setText(tr("<a href=\"https://twitter.com/%1/\" style=\"color:#555;text-decoration: none;\">%1 (%2)</a>").arg(results.at(id).fromUser(), results.at(id).fromUserName()));

    QString body = Qt::escape(results.at(id).text());
    body.replace(urlRegExp, "<a href=\"\\1\">\\1</a>")
            .replace(userRegExp, "<a href=\"https://twitter.com/\\1/\">@\\1</a>\\2")
            .replace(hashTagRegExp, "<a href=\"https://twitter.com/#!/search/%23\\1\">#\\1</a>\\2");
    text.setText(body);

    QDateTime stamp = results.at(id).createdAt();
    int diff = stamp.secsTo(QDateTime::currentDateTime());
    int years = diff / YEAR; diff -= YEAR * years;
    int months = diff / MONTH; diff -= MONTH * months;
    int days = diff / DAY; diff -= DAY * days;
    int hours = diff / HOUR; diff -= HOUR * hours;
    int minutes = diff / MINUTE; diff -= MINUTE * minutes;
    int secs = diff;

    QString word;
    int n = 0;
    if (years) {
        word = choose_from_text(years, 0);
        n = years;
    } else if (months) {
        word = choose_from_text(months, 1);
        n = months;
    } else if (days) {
        word = choose_from_text(days, 2);
        n = days;
    } else if (hours) {
        word = choose_from_text(hours, 3);
        n = hours;
    } else if (minutes) {
        word = choose_from_text(minutes, 4);
        n = minutes;
    } else if (secs) {
        word = choose_from_text(secs, 5);
        n = secs;
    }

    timeStamp.setText(QString::fromUtf8("<span style=\"font-size:11px;color:#444;\">примерно %1 %2 назад из <span>%3</span>").arg(QString::number(n), word,
                                                                                                                            results.at(id).source()
                                                                                                                            .replace("&lt;", "<")
                                                                                                                            .replace("&gt;", ">")
                                                                                                                            .replace("&quot;", "\"")
                                                                                                                            .replace("&amp;", "&")
                                                                                                                            .replace("<a", "<a style=\"color:#555;text-decoration: none;\" ")));


    disconnect(&mAnim, SIGNAL(finished()), this, SLOT(finishRefresh()));
    mAnim.setEasingCurve(QEasingCurve::OutBack);
    mAnim.setStartValue(slidingWidget->pos());
    mAnim.setEndValue(QPoint(0, 0));
    mAnim.start();
    mRefreshTimer.start(8 * 1000);
}
