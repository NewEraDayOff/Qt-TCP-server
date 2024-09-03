#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent), TcpItem(TcpItem::SERVER) {} //конструктор

void TcpServer::incomingConnection(qintptr descriptor) //принятие входящих подключений новых клиентов
{
    auto user = new QTcpSocket(this);
    QObject::connect(user, &QTcpSocket::readyRead, this, &TcpServer::ReadyRead);
    QObject::connect(user, &QTcpSocket::disconnected, this, &TcpServer::UserDisconnected);
    QObject::connect(user, &QTcpSocket::bytesWritten, &writeLoop, &QEventLoop::quit);

    user->setSocketDescriptor(descriptor);
    this->users.insert(user);
}

bool TcpServer::SetPort(const QString &strport) //сеттер порта
{
    if (!TcpItem::SetPort(strport))
    {
        emit TcpServer::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpServer::SetName(const QString &name) //сеттер имени
{
    if (!TcpItem::SetName(name))
    {
        emit TcpServer::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpServer::SetColor(const QColor &color) //сеттер цвета
{
    if (!TcpItem::SetColor(color))
    {
        emit TcpServer::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpServer::SetBColor(const QColor &bcolor) //сеттер цвета фона
{
    if (!TcpItem::SetBColor(bcolor))
    {
        emit TcpServer::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpServer::TryOpen() //проверка возможности открыть сервер
{
    if (!TcpItem::SetPort(this->port))
    {
        return false;
    }

    if (!TcpItem::SetName(this->name))
    {
        return false;
    }

    if (this->isListening())
    {
        TcpItem::error = "Сервер уже ищет входные подключения!";
        return false;
    }

    if (!this->listen(QHostAddress::Any, this->port))
    {
        TcpItem::error = "Не удалось открыть сервер!";
        return false;
    }
    return true;
}

bool TcpServer::Open() //открытие сервера
{
    if (!this->TryOpen())
    {
        emit TcpServer::SendError(TcpItem::error);
        return false;
    }
    this->active = true;
    this->SetCondition(ON);
    emit this->SendInfo(this->StyledText(Qt::darkGreen, Qt::white, "---Открытие сервера---"));
    emit TcpServer::SendOpenServerInfo(QString("Сервер успешно открыт!\nПорт: %2").arg(this->serverPort()));
    return this->active;
}

bool TcpServer::Close() //закрытие сервера
{
    this->active = false;
    this->SetCondition(OFF);
    emit this->SendInfo(this->StyledText(Qt::red, Qt::white, "---Закрытие сервера---"));
    this->close();
    foreach (auto user, this->users) user->disconnectFromHost();
    return true;
}

void TcpServer::UserDisconnected() //отключение клиента
{
    auto user(qobject_cast<QTcpSocket *>(QObject::sender()));

    emit this->SendInfo(QString("Клиент %1 отключился.").arg(this->names[user]));

    this->names.remove(user);
    this->colors.remove(user);
    this->bcolors.remove(user);
    this->users.remove(user);

    user->deleteLater();
}

void TcpServer::Mailing(const QString &message) //рассылка сообщения клиентам
{
    foreach (auto user, this->users)
    {
        user->write(message.toUtf8());
        if (!this->writeLoop.isRunning())
            this->writeLoop.exec();
    }
}

void TcpServer::SendMessage(const MesType mt, const QString &text) //отправка сообщений
{
    QString message;
    if (mt == MES)
    { message = this->ServerMessage(text, this->MyNameStyled()); }
    else if (mt == BOT)
    { message = this->ServerBotMessage(text, this->MyNameStyled()); }
    else
    { return; }
    emit this->SendInfo(message);
    this->Mailing(this->PackMessage(MES, message));
}

void TcpServer::SendMessage(const MesType mt, const QColor &color) //отправка цвета
{
    QString message;
    if (mt == COL)
        message = this->MyNewCol(this->MyNameStyled(), this->StyledText(color, this->bcolor, this->name));
    else if (mt == BCOL)
        message = this->MyNewCol(this->MyNameStyled(), this->StyledText(this->color, color, this->name));
    else
    {
        message = color.name();
        qDebug() << Q_FUNC_INFO;
        qDebug("MISSINPUT");
        return;
    }
    emit this->SendInfo(message);
    this->Mailing(this->PackMessage(MES, message));
}

void TcpServer::ReSendMessage(const QString &message) //пересылка сообщений других пользователей
{
    emit this->SendInfo(message);
    this->Mailing(this->PackMessage(MES, message));
}

//добавление новых данных о пользователе в набор данных пользователей
bool TcpServer::AddUserInfo(QTcpSocket *user, QHash<QTcpSocket *, QString> &container, QString text)
{
    const bool newUser = !container.contains(user);
    container[user] = text;
    return newUser;
}

//попытка прочитать имя
bool TcpServer::TryReadName(QTcpSocket *user, const QString &line, QString &output)
{
    if (!this->TryRead(NAME, line, output)) return false;
    this->AddUserInfo(user, this->names, output);
    output = this->UserIntro(this->NameStyled(user));
    return true;
}

//попытка прочитать цвет
bool TcpServer::TryReadColor(QTcpSocket *user, const QString &line, QString &output)
{
    if (!this->TryRead(COL, line, output)) return false;
    const QString oldcol = this->colors.contains(user) ? this->colors[user] : QColor(Qt::black).name();
    if (!this->AddUserInfo(user, this->colors, output))
    {
        const QString oldname = this->StyledText(oldcol, this->bcolors[user], this->names[user]);
        const QString newname = this->NameStyled(user);
        output = this->UserNewCol(oldname, newname);
        return true;
    }
    return false;
}

//попытка прочитать цвет фона
bool TcpServer::TryReadBColor(QTcpSocket *user, const QString &line, QString &output)
{
    if (!this->TryRead(BCOL, line, output)) return false;
    const QString oldbcol = this->bcolors.contains(user) ? this->bcolors[user] : QColor(Qt::white).name();
    if (!this->AddUserInfo(user, this->bcolors, output))
    {
        const QString oldname = this->StyledText(this->colors[user], oldbcol, this->names[user]);
        const QString newname = this->NameStyled(user);
        output = this->UserNewCol(oldname, newname);
        return true;
    }
    return false;
}

//попытка прочитать сообщение бота
bool TcpServer::TryReadBot(QTcpSocket *user, const QString &line, QString &output)
{
    if (!this->TryRead(BOT, line, output)) return false;
    output = this->BotMessage(output, this->NameStyled(user));
    return true;
}

//попытка прочитать обычное сообщение
bool TcpServer::TryReadMes(QTcpSocket *user, const QString &line, QString &output)
{
    if (!this->TryRead(MES, line, output)) return false;
    output = this->DefMessage(output, this->NameStyled(user));
    return true;
}

//чтение сообщений от клиентов
void TcpServer::ReadyRead()
{
    auto user = qobject_cast<QTcpSocket *>(QObject::sender());
    while (user->canReadLine())
    {
        const QString line = QString::fromUtf8(user->readLine()).trimmed();
        QString message = QString();

        //попытка декодировать сообщение.
        //Если сообщение соответствует какому-то из нижеперечисленных, то оно собирается и рассылается всем
        if (this->TryReadName(user, line, message) //попытка считать имя
         || this->TryReadColor(user, line, message) //попытка считать цвет
         || this->TryReadBColor(user, line, message) //попытка считать цвет фона
         || this->TryReadBot(user, line, message) //попытка считать бота
         || this->TryReadMes(user, line, message)) //попытка считать сообщение
            this->ReSendMessage(message); //если хоть одна попытка была удачной, рассылаем сообщение
    }
}
