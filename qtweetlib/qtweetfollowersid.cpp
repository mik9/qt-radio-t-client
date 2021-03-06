/* Copyright (c) 2010, Antonie Jovanoski
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact e-mail: Antonie Jovanoski <minimoog77_at_gmail.com>
 */

#include <QtDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "qtweetfollowersid.h"
#include "json/qjsondocument.h"
#include "json/qjsonarray.h"
#include "json/qjsonobject.h"

/**
 *  Constructor
 */
QTweetFollowersID::QTweetFollowersID(QObject *parent) :
    QTweetNetBase(parent)
{
}

/**
 *  Constructor
 *  @param oauthTwitter OAuthTwitter object
 *  @param parent parent QObject
 */
QTweetFollowersID::QTweetFollowersID(OAuthTwitter *oauthTwitter, QObject *parent) :
        QTweetNetBase(oauthTwitter, parent)
{
}

/**
 *  Starts fetching one page of id's
 *  @param user the ID of the user for whom to return results for.
 *  @param cursor use from signal response nextCursor and prevCursor to allow paging back and forth
 */
void QTweetFollowersID::fetch(qint64 user, const QString &cursor)
{
    QUrl url("https://api.twitter.com/1/followers/ids.json");

    url.addQueryItem("user_id", QString::number(user));
    url.addQueryItem("cursor", cursor);

    QNetworkRequest req(url);

    if (isAuthenticationEnabled()) {
        QByteArray oauthHeader = oauthTwitter()->generateAuthorizationHeader(url, OAuth::GET);
        req.setRawHeader(AUTH_HEADER, oauthHeader);
    }

    QNetworkReply *reply = oauthTwitter()->networkAccessManager()->get(req);
    connect(reply, SIGNAL(finished()), this, SLOT(reply()));
}

/**
 *  Starts fetching one page of id's
 *  @param screenName the screen name of the user for whom to return results for.
 *  @param cursor use from signal response nextCursor and prevCursor to allow paging back and forth
 */
void QTweetFollowersID::fetch(const QString &screenName, const QString &cursor)
{
    QUrl url("https://api.twitter.com/1/followers/ids.json");

    url.addQueryItem("screen_name", screenName);
    url.addQueryItem("cursor", cursor);

    QNetworkRequest req(url);

    if (isAuthenticationEnabled()) {
        QByteArray oauthHeader = oauthTwitter()->generateAuthorizationHeader(url, OAuth::GET);
        req.setRawHeader(AUTH_HEADER, oauthHeader);
    }

    QNetworkReply *reply = oauthTwitter()->networkAccessManager()->get(req);
    connect(reply, SIGNAL(finished()), this, SLOT(reply()));
}

void QTweetFollowersID::parseJsonFinished(const QJsonDocument &jsonDoc)
{
    if (jsonDoc.isObject()) {
        QList<qint64> idList;

        QJsonObject respJsonObject = jsonDoc.object();

        QJsonArray idJsonArray = respJsonObject["ids"].toArray();

        for (int i = 0; i < idJsonArray.size(); ++i)
            idList.append(static_cast<qint64>(idJsonArray[i].toDouble()));

        QString nextCursor = respJsonObject["next_cursor_str"].toString();
        QString prevCursor = respJsonObject["previous_cursor_str"].toString();

        emit parsedIDs(idList, nextCursor, prevCursor);
    }
}
