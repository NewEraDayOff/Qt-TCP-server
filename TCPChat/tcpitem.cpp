#include "tcpitem.h"

bool TcpItem::SetPort(const QString &strport) //сеттер порта
{
    bool ok;
    const qint64 port = strport.toInt(&ok);
    if (!ok)
    {
        this->error = "Порт введен неверно!";
        return false;
    }
    if (!this->SetPort(port))
    {
        return false;
    }
    return true;
}

bool TcpItem::SetPort(const quint64 port) //сеттер порта
{
    this->port = -1;
    if (port < 0)
    {
        this->error = "Порт не может быть меньше нуля!";
        return false;
    }
    if (port > 65535)
    {
        this->error = "Порт не может быть больше 65535!";
        return false;
    }
    this->port = port;
    return true;
}

bool TcpItem::TryPort(const QString &strport) //проверка порта
{
    bool ok;
    const qint64 port = strport.toInt(&ok);
    if (!ok) return false;
    if (port < 0) return false;
    if (port > 65535) return false;
    return true;
}

bool TcpItem::SetName(const QString &name) //сеттер имени
{
    const QString tname = name.trimmed();
    if (tname.isEmpty())
    {
        this->error = "Имя не должно быть пустым!";
        return false;
    }
    if (tname.size() > 16)
    {
        this->error = "Имя не должно быть больше 16-ти символов!";
        return false;
    }
    this->name = tname;
    return true;
}

bool TcpItem::TryName(const QString &name) //проверка имени
{
    const QString tname = name.trimmed();
    if (tname.isEmpty()) return false;
    if (tname.size() > 16) return false;
    return true;
}

bool TcpItem::SetColor(const QColor &color) //сеттер цвета
{
    if (!color.isValid())
    {
        this->error = "Ошибка передачи цвета!";
        return false;
    }
    this->color = color;
    return true;
}

bool TcpItem::TryColor(const QColor &color) //проверка цвета
{
    if (!color.isValid()) return false;
    return true;
}

bool TcpItem::SetBColor(const QColor &bcolor) //сеттер цвета фона
{
    if (!bcolor.isValid())
    {
        this->error = "Ошибка передачи цвета!";
        return false;
    }
    this->bcolor = bcolor;
    return true;
}
