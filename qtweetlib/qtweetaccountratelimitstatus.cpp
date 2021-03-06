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
#include "qtweetaccountratelimitstatus.h"
#include "json/qjsondocument.h"
#include "json/qjsonobject.h"

/**
 *  Constructor
 */
QTweetAccountRateLimitStatus::QTweetAccountRateLimitStatus(QObject *parent) :
    QTweetNetBase(parent)
{
}

/**
 *  Constructor
 *  @param oauthTwitter OAuthTwitter object
 *  @param parent parent QObject
 */
QTweetAccountRateLimitStatus::QTweetAccountRateLimitStatus(OAuthTwitter *oauthTwitter, QObject *parent) :
        QTweetNetBase(oauthTwitter, parent)
{
}

/**
 *  Starts checking rate limit status
 *  @remarks Should be emiting rateLimitInfo signal after finishing
 */
void QTweetAccountRateLimitStatus::check()
{
    QUrl url("https://api.twitter.com/1/account/rate_limit_status.json");

    QNetworkRequest req(url);

    if (isAuthenticationEnabled()) {
        QByteArray oauthHeader = oauthTwitter()->generateAuthorizationHeader(url, OAuth::GET);
        req.setRawHeader(AUTH_HEADER, oauthHeader);
    }

    QNetworkReply *reply = oauthTwitter()->networkAccessManager()->get(req);
    connect(reply, SIGNAL(finished()), this, SLOT(reply()));
}

void QTweetAccountRateLimitStatus::parseJsonFinished(const QJsonDocument &jsonDoc)
{
    if (jsonDoc.isObject()) {
        QJsonObject respJsonObject = jsonDoc.object();

        int remainingHits = static_cast<int>(respJsonObject["remaining_hits"].toDouble());
        int resetTime = static_cast<int>(respJsonObject["reset_time_in_seconds"].toDouble());
        int hourlyLimit = static_cast<int>(respJsonObject["hourly_limit"].toDouble());

        emit rateLimitInfo(remainingHits, resetTime, hourlyLimit);
    }
}
