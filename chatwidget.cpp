#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "key.h"

QString ChatWidget::MESSAGE_FORMAT = "<table cellspacing=0 cellpadding=0 border=0><tr><td width=80>%4&nbsp;</td><td width=120 align=right style=\"padding-right:2\"><a href=\"user://%1/\"><font color=\"%2\">%1</font></a>:</td><td style=\"padding-left:3;background-color:#fff;\" width=100%>%3 </td></tr></table>";
QRegExp ChatWidget::URL_REG_EXP("(\\w+://\\S+\\.\\S+)");
QRegExp ChatWidget::EMAIL_REG_EXP("\\S+@(\\S+\\.\\S+)");

QColor ChatWidget::username_to_color(QString u) {
    quint8 sum = 0;
    foreach(QChar c, u) {
        sum += c.toAscii();
    }
    return QColor::fromHsv(sum, 255, 180);
}

QString ChatWidget::parseServer(QString JID) {
    if (EMAIL_REG_EXP.indexIn(JID) != -1) {
        return EMAIL_REG_EXP.cap(1);
    }
    return "";
}

ChatWidget::ChatWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWidget)
{
    ui->setupUi(this);

    connect(this->ui->message_edit, SIGNAL(returnPressed()), this, SLOT(on_send_button_clicked()));
    this->ui->message_edit->setFocus();

    QSettings settings("mik_os", "qt/Radio-T client");
    this->m_username = settings.value("username", "").toString();
    SimpleCrypt s(CRYPT_KEY);
    this->m_password = s.decryptToString(settings.value("password", "").toString());
    this->m_server = settings.value("server", "").toString();
    this->ui->nick_edit->setText(settings.value("nick", "").toString());
    if (this->m_server.isEmpty() && !this->m_username.isEmpty()) {
        this->m_server = this->parseServer(this->m_username);
    }
    this->ui->login_edit->setText(this->m_username);
    this->ui->password_edit->setText(this->m_password);
    this->ui->server_edit->setText(this->m_server);
    this->ui->message_frame->hide();
    this->ui->connecting_frame->hide();
    this->ui->nick_frame->hide();

    class EventHandler : public QObject {
    public:
        EventHandler(ChatWidget* w) : m_widget(w), was_edited(false) {}
    private:
        ChatWidget* m_widget;
        bool was_edited;
    protected:
        bool eventFilter(QObject*, QEvent *event)
        {
            if (event->type() == QEvent::FocusOut) {
                if(was_edited) {
                    QString server = parseServer(m_widget->ui->login_edit->text());
                    if (!server.isEmpty()) {
                        m_widget->ui->server_edit->setText(server);
                    }
                    was_edited = false;
                }
            } else if (event->type() == QEvent::KeyPress) {
                was_edited = true;
            }
            return false;
        }
    };
    this->ui->login_edit->installEventFilter(new EventHandler(this));
//    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);

    xmpp_client.addExtension(&manager);

    connect(&xmpp_client, SIGNAL(connected()), this, SLOT(xmpp_connected()));
}

ChatWidget::~ChatWidget()
{
    QSettings settings("mik_os", "qt/Radio-T client");
    settings.setValue("username", this->m_username);
    SimpleCrypt s(CRYPT_KEY);
    settings.setValue("password", s.encryptToString(this->m_password));
    settings.setValue("server", this->m_server);
    settings.setValue("nick", this->ui->nick_edit->text());
    xmpp_client.disconnectFromServer();
    delete ui;
}

void ChatWidget::message_received(QXmppMessage m) {
    QString nick = strip_username(m.from());
    QString message = m.body();
    QDateTime stamp = m.stamp();
    QString stamp_str = "";

    bool should_scroll_down = (this->ui->chat_edit->verticalScrollBar()->maximum()
                               - this->ui->chat_edit->verticalScrollBar()->value()) < 10;
    if (!this->m_username.isEmpty() && message.contains(this->m_username)) {
        if (this->isActiveWindow()) {
            emit this->notify();
        }
    }
    /*if (stamp.daysTo(QDateTime::currentDateTime()) > 1) {
        stamp_str = stamp.toString("dd/MM hh:mm:ss");
    } else*/ {
        stamp_str = stamp.toString("hh:mm:ss");
    }
    message.replace(URL_REG_EXP, "<a href=\"\\1\">\\1</a>");
    int i=0, max=nick.length();
    while (this->ui->chat_edit->fontMetrics().width(nick + ":") > 120) {
        if(!nick.endsWith("...")) {
            nick += "...";
        }
        nick.remove(nick.length()-4, 1);
        if (++i>max)
            break;
        qDebug() << this->ui->chat_edit->fontMetrics().width(stamp_str + " " + nick + ":");
    }
    this->ui->chat_edit->insertHtml(this->MESSAGE_FORMAT.arg(nick, username_to_color(nick).name(), message, stamp_str));
    if (should_scroll_down) {
        this->ui->chat_edit->verticalScrollBar()->setValue(this->ui->chat_edit->verticalScrollBar()->maximum());
    }
}

void ChatWidget::on_chat_edit_anchorClicked(QUrl uri) {
    if (uri.scheme() == "user") {
        QString username = uri.host();
        this->put_username(username);
    } else {
        QDesktopServices::openUrl(uri);
    }
}

void ChatWidget::on_send_button_clicked() {
    QString message = this->ui->message_edit->text();
    emit send_button_clicked(message);
    this->ui->message_edit->setText("");
    this->ui->message_edit->setFocus();
    xmpp_client.sendMessage("online@conference.radio-t.com", message);
}

void ChatWidget::on_login_button_clicked()
{
    if (!this->ui->login_edit->text().isEmpty() && !this->ui->password_edit->text().isEmpty()) {
        if (this->ui->server_edit->text().isEmpty()) {
            QString server = parseServer(this->m_username);
            if (server.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please fill server info.");
                return;
            }
            this->ui->server_edit->setText(server);
        }
    } else {
        QMessageBox::warning(this, "Error", "Check fields.");
        return;
    }
    this->ui->login_frame->hide();
    this->ui->connecting_frame->show();

    xmpp_client.configuration().setDomain(this->m_server);
    xmpp_client.connectToServer(this->m_username, this->m_password);
}

void ChatWidget::on_login_edit_textChanged(const QString &arg1)
{
    this->m_username = arg1;
}

void ChatWidget::on_password_edit_textChanged(const QString &arg1)
{
    this->m_password = arg1;
}

void ChatWidget::on_server_edit_textChanged(const QString &arg1)
{
    this->m_server = arg1;
}

void ChatWidget::update_user_list(QStringList list) {
    this->ui->user_list->clear();
    foreach(QString jid, list) {
        user_in(jid);
    }
}

void ChatWidget::user_in(QString jid) {
    QString nick = strip_username(jid);
    QListWidgetItem* item = new QListWidgetItem(nick);
    item->setForeground(QBrush(username_to_color(nick)));
    this->ui->user_list->addItem(item);
}

void ChatWidget::user_out(QString jid) {
    QString s = strip_username(jid);
    QList<QListWidgetItem*> items = this->ui->user_list->findItems(s, Qt::MatchFixedString);
    if (items.length() > 1) {
        qWarning() << "More than 1 user has same nick";
    }
    this->ui->user_list->removeItemWidget(items.at(0));
}

void ChatWidget::on_user_list_doubleClicked(const QModelIndex &index)
{
    put_username(this->ui->user_list->item(index.row())->text());
}

void ChatWidget::put_username(QString username) {
    if (this->ui->message_edit->text().isEmpty()) {
        username += ": ";
    }
    if (!this->ui->message_edit->text().endsWith(" ")) {
        username = " " + username;
    }
    if (!username.endsWith(" ")) {
        username += " ";
    }
    this->ui->message_edit->setText(this->ui->message_edit->text() + username);
}

void ChatWidget::xmpp_connected() {
    this->ui->connecting_frame->hide();
    this->ui->nick_frame->show();
}

void ChatWidget::room_joined() {
    this->ui->connecting_frame->hide();
    this->ui->message_frame->show();
    this->ui->chat_edit->verticalScrollBar()->setValue(this->ui->chat_edit->verticalScrollBar()->value()+this->ui->message_frame->height());
    update_user_list(manager.rooms().at(0)->participants());
}

void ChatWidget::on_join_button_clicked()
{
    this->ui->nick_frame->hide();
    this->ui->connecting_frame->show();
    this->ui->status_label->setText("Joining room...");
    QXmppMucRoom *room = manager.addRoom("online@conference.radio-t.com");
    room->setNickName(this->ui->nick_edit->text());
    connect(room, SIGNAL(messageReceived(QXmppMessage)), this, SLOT(message_received(QXmppMessage)), Qt::UniqueConnection);
    connect(room, SIGNAL(joined()), this, SLOT(room_joined()), Qt::UniqueConnection);
    connect(room, SIGNAL(participantAdded(QString)), this, SLOT(user_in(QString)), Qt::UniqueConnection);
    connect(room, SIGNAL(participantRemoved(QString)), this, SLOT(user_out(QString)), Qt::UniqueConnection);
    connect(room, SIGNAL(kicked(QString,QString)), this, SLOT(user_left_room()), Qt::UniqueConnection);
    connect(room, SIGNAL(left()), this, SLOT(user_left_room()));
    room->join();
}

QString ChatWidget::strip_username(QString username) {
    QStringList username_parts = username.split("/");
    return username_parts.length() > 1 ? username_parts.at(1) : username;
}

void ChatWidget::user_left_room() {
    this->ui->message_frame->hide();
    this->ui->nick_frame->show();
}

void ChatWidget::on_logout_button_clicked()
{
    this->manager.rooms().at(0)->leave();
}
