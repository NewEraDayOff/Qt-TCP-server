#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpSocket>
#include <QTcpServer>
#include "tcpitem.h"

class TcpServer : public QTcpServer, public TcpItem
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    void SendMessage(const MesType, const QString &) override;
    void SendMessage(const MesType, const QColor &) override;
    bool SetPort(const QString &) override;
    bool SetName(const QString &) override;
    bool SetColor(const QColor &) override;
    bool SetBColor(const QColor &) override;
    bool Open() override;
    bool Close() override;

private:
    QSet<QTcpSocket *> users = QSet<QTcpSocket *>(); //контейнер пользователей
    QHash<QTcpSocket *, QString> names = QHash<QTcpSocket *, QString>(); //набор имен пользователей
    QHash<QTcpSocket *, QString> colors  = QHash<QTcpSocket *, QString>(); //набор цветов пользователей
    QHash<QTcpSocket *, QString> bcolors  = QHash<QTcpSocket *, QString>(); //набор цветов фона пользователей
    QMap<MesType, QRegExp> prReader //словарь-декодер связок "тип сообщения - расшифровка"
    {
        {NAME, QRegExp("^/name(.*)$")},
        {COL, QRegExp("^/color(#.{6})$")},
        {BCOL, QRegExp("^/bcolor(#.{6})$")},
        {BOT, QRegExp("^/bot(.*)$")},
        {MES, QRegExp("^/message(.*)$")}
    };
    void incomingConnection(qintptr) override;
    void ReSendMessage(const QString &);
    bool TryOpen();
    //сеттер состояния клиента с испусканием сигнала при изменении
    void SetCondition(CondType ct)
    {
        if (this->condtype != ct)
        {
            this->condtype = ct;
            emit this->ConditionChanged(this->condtype);
        }
    }
    //цветовое изменение серверного имени
    inline const QString MyNameStyled() const
    { return this->StyledText(this->color, this->bcolor, this->name); }

    //цветовое изменение имени
    inline const QString NameStyled(QTcpSocket *user) const
    { return this->StyledText(this->colors[user], this->bcolors[user], this->names[user]); }

    //цветовое изменение текста
    inline const QString StyledText(const QColor &c, const QColor &bc, const QString &n) const
    { return this->StyledText(c.name(), bc.name(), n); }
    //цветовое изменение текста
    inline const QString StyledText(const QColor &c, const QString &bc, const QString &n) const
    { return this->StyledText(c.name(), bc, n); }
    //цветовое изменение текста
    inline const QString StyledText(const QString &c, const QColor &bc, const QString &n) const
    { return this->StyledText(c, bc.name(), n); }
    //цветовое изменение текста
    inline const QString StyledText(const QString &c, const QString &bc, const QString &n) const
    { return QString("<font style=\"color:%2;background-color:%3\">%1</font>").arg(n, c, bc); }

    //Трафарет сообщения сервера
    inline const QString ServerMessage(const QString &text, const QString &name) const
    { return QString("<b>Сервер</b>(%2): %1").arg(text, name); }

    //Трафарет сообщения спам-бота
    inline const QString BotMessage(const QString &text, const QString &name) const
    { return QString("Бот(%2): <b>%1</b>").arg(text, name); }

    //Трафарет сообщения спам-бота сервера
    inline const QString ServerBotMessage(const QString &text, const QString &name) const
    { return QString("<b>Сервер Бот</b>(%2): <b><font style=\"color:red\">%1</font></b>").arg(text, name); }

    //трафарет обычного сообщения
    inline const QString DefMessage(const QString &text, const QString &name) const
    { return QString("%2: %1").arg(text, name); }

    //Трафарет приветствия нового пользователя
    inline const QString UserIntro(const QString &name) const
    { return QString("Поприветствуйте нового пользователя! Здраствуй, %1!").arg(name); }

    //Трафарет сообщения изменения цвета пользователя
    inline const QString UserNewCol(const QString &oldname, const QString &newname) const
    { return QString("Пользователь %1 сменил цвет на %2.").arg(oldname, newname); }
    //Трафарет сообщения изменения цвета сервера
    inline const QString MyNewCol(const QString &oldname, const QString &newname) const
    { return QString("<b>Сервер</b>(%1) сменил цвет на <b>Сервер</b>(%2).").arg(oldname, newname); }

    //Попытка прочитать сообщение заданного типа с выводом результата в output
    bool TryRead(const MesType mt, const QString &line, QString &output)
    {
        if (this->prReader[mt].indexIn(line) == -1) return false;
        output = this->prReader[mt].cap(1);
        return true;
    }

    bool TryReadName(QTcpSocket *, const QString &line, QString &output);
    bool TryReadColor(QTcpSocket *, const QString &line, QString &output);
    bool TryReadBColor(QTcpSocket *, const QString &line, QString &output);
    bool TryReadBot(QTcpSocket *, const QString &line, QString &output);
    bool TryReadMes(QTcpSocket *, const QString &line, QString &output);

    bool AddUserInfo(QTcpSocket *, QHash<QTcpSocket *, QString> &, QString);

    void Mailing(const QString &);

private slots:
    void ReadyRead() override;
    void UserDisconnected();

signals:
    void SendInfo(const QString &);
    void SendError(const QString &);
    void SendOpenServerInfo(const QString &);
    void ConditionChanged(CondType);
};

#endif // TCPSERVER_H
