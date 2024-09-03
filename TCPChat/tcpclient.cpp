#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent) : QTcpSocket(parent), TcpItem(TcpItem::CLIENT) //конструктор
{
    QObject::connect(this, &QAbstractSocket::connected, this, &TcpClient::Connected);
    QObject::connect(this, &QAbstractSocket::disconnected, this, &TcpClient::Disconnected);
    QObject::connect(this, &QIODevice::readyRead, this, &TcpClient::ReadyRead);
    QObject::connect(this, QOverload<SocketError>::of(&QAbstractSocket::error), this, &TcpClient::OnError);
    QObject::connect(this, &QAbstractSocket::connected, &connectLoop, &QEventLoop::quit);
    QObject::connect(this, &QIODevice::bytesWritten, &writeLoop, &QEventLoop::quit);
}

bool TcpClient::SetPort(const QString &strport) //сеттер порта
{
    if (!TcpItem::SetPort(strport))
    {
        emit TcpClient::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpClient::SetName(const QString &name) //сеттер имени
{
    if (!TcpItem::SetName(name))
    {
        emit TcpClient::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpClient::SetColor(const QColor &color) //сеттер цвета
{
    if (!TcpItem::SetColor(color))
    {
        emit TcpClient::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpClient::SetBColor(const QColor &bcolor) //сеттер цвета фона
{
    if (!TcpItem::SetBColor(bcolor))
    {
        emit TcpClient::SendError(TcpItem::error);
        return false;
    }
    return true;
}

bool TcpClient::SetAddress(const QString &address) //сеттер адреса
{
    const QString taddress = address.trimmed();
    if (taddress.isEmpty())
    {
        TcpItem::error = "Вы ничего не ввели в поле для адреса!";
        return false;
    }
    if (!QHostAddress().setAddress(address))
    {
        TcpItem::error = "Адрес введен неверно!";
        return false;
    }
    this->address = taddress;
    return true;
}

bool TcpClient::TryAddress(const QString &address) //проверка ликвидности адреса
{
    const QString taddress = address.trimmed();
    if (taddress.isEmpty()) return false;
    if (!QHostAddress().setAddress(address)) return false;
    return true;
}

bool TcpClient::TryOpen() //проверка ликвидности открытия клиента
{
    if (!TcpClient::SetAddress(this->address)) return false;
    if (!TcpItem::SetPort(this->port)) return false;
    if (!TcpItem::SetName(this->name)) return false;
    return true;
}

bool TcpClient::Open() //запуск подключения
{
    if (!this->TryOpen())
    {
        emit TcpClient::SendError(TcpItem::error);
        return false;
    }
    this->counter = 3;
    this->connectToHost(this->address, this->port);
    if (!this->connectLoop.isRunning() && QAbstractSocket::error() == UnknownSocketError)
    {
        this->SetCondition(LOOKUP);
        this->connectLoop.exec();
    }
    return this->active;
}

void TcpClient::OnError(SocketError err) //слот возникающих ошибок
{
    emit this->SendInfo(errorString());
    if (err == QAbstractSocket::ConnectionRefusedError && this->counter)
    {
        this->counter--;
        emit this->SendInfo("Попытка подключения...");
        this->connectToHost(this->address, this->port);
    }
    else
    {
        if (!this->counter) emit this->SendInfo("Подключение не удалось.");
        if (this->connectLoop.isRunning())
            this->connectLoop.quit();
    }
}

bool TcpClient::Close() //закрытие клиента
{
    if (this->condtype == LOOKUP && this->connectLoop.isRunning())
    {
        emit this->SendInfo("Поиск прерван.");
        this->connectLoop.quit();
    }
    this->active = false;
    this->SetCondition(OFF);
    this->flush();
    this->abort();
    return true;
}

void TcpClient::Connected() //слот удачного подключения к серверу
{
    this->active = true;
    this->SetCondition(ON);
    emit this->SendInfo("Вы успешно подключились!");
    this->SendMessage(COL, this->color);
    this->SendMessage(BCOL, this->bcolor);
    this->SendMessage(NAME, this->name);
}

void TcpClient::Disconnected() //слот разрыва соединения с сервером
{
    this->active = false;
    this->SetCondition(OFF);
    emit this->SendInfo("Соединение было разорвано!");
    this->flush();
    this->abort();
}

void TcpClient::SendMessage(const MesType mt, const QString &text) //отправка сообщений серверу
{
    const QString message = this->PackMessage(mt, text);
    this->write(message.toUtf8());
    if (!this->writeLoop.isRunning())
        this->writeLoop.exec();
}

void TcpClient::SendMessage(const MesType mt, const QColor &color) //отправка цвета серверу
{
    const QString message = this->PackMessage(mt, color.name());
    this->write(message.toUtf8());
    if (!this->writeLoop.isRunning())
        this->writeLoop.exec();
}

void TcpClient::ReadyRead() //чтение сообщений от сервера
{
    auto user = qobject_cast<QTcpSocket *>(QObject::sender());
    while (user->canReadLine())
    {
        const QString line = QString::fromUtf8(user->readLine()).trimmed();
        QRegExp qre("^/message(.*)$");
        QString message = QString();
        if (qre.indexIn(line) != -1) message = qre.cap(1);
        if (!message.isEmpty()) emit this->SendInfo(message);
    }
}
