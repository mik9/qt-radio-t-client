#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QDebug>
#include <QDesktopServices>
#include <QTimer>
#include <QScrollBar>
#include <QSettings>
#include <QObject>
#include <QFocusEvent>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <client/QXmppClient.h>
#include <base/QXmppLogger.h>
#include <base/QXmppMessage.h>
#include <client/QXmppMucManager.h>
#include <QStringList>
#include "simplecrypt.h"

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChatWidget(QWidget *parent = 0);
    ~ChatWidget();
    
private slots:
    void on_chat_edit_anchorClicked(QUrl uri);
    void on_send_button_clicked();
    void on_login_button_clicked();
    void on_login_edit_textChanged(const QString &arg1);
    void on_password_edit_textChanged(const QString &arg1);
    void on_server_edit_textChanged(const QString &arg1);
    void update_user_list(QStringList list);
    void user_in(QString jid);
    void user_out(QString jid);
    void on_user_list_doubleClicked(const QModelIndex &index);
    void message_received(QXmppMessage m);
    void xmpp_connected();
    void room_joined();
    void on_join_button_clicked();
    void user_left_room();
    void on_logout_button_clicked();
    void xmpp_disconnected();

signals:
    void send_button_clicked(QString message);
    void notify();

private:
    Ui::ChatWidget *ui;
    void put_username(QString username);
    QString m_username;
    QString m_password;
    QString m_server;
    QString m_nick;
    QXmppClient xmpp_client;
    QXmppMucManager manager;
    QPropertyAnimation* m_chat_anim;
    QString jabber_room;

    static QString MESSAGE_FORMAT;
    static QString ME_MESSAGE_FORMAT;
    static QRegExp URL_REG_EXP;
    static QRegExp EMAIL_REG_EXP;
    static QColor username_to_color(QString u);
    static QString parseServer(QString JID);
    static QString strip_username(QString username);
};

#endif // CHATWIDGET_H
