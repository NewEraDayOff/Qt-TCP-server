#ifndef TCPITEM_H
#define TCPITEM_H

#include <QObject>
#include <QColor>
#include <QEventLoop>
#include <QDebug>

class TcpItem
{

public:
    enum TCPType { NOINFO, CLIENT, SERVER }; //тип Tcp объекта
    enum MesType { NAME, COL, BCOL, BOT, MES }; //тип сообщения
    enum CondType { OFF, LOOKUP, ON }; //тип состояния Tcp объекта

    explicit TcpItem(TCPType type) : ttype(type) {} //конструктор
    virtual ~TcpItem() {}; //деструктор

    virtual bool Open() = 0;
    virtual bool Close() = 0;
    virtual void SendMessage(const MesType, const QString &) = 0;
    virtual void SendMessage(const MesType, const QColor &) = 0;
    virtual bool SetPort(const QString &);
    virtual bool SetName(const QString &);
    virtual bool SetColor(const QColor &);
    virtual bool SetBColor(const QColor &);

    inline const QString &LastError() const { return this->error; } //геттер последней ошибки
    inline const QString &GetName() const { return this->name; } //геттер имени
    inline const QColor &GetColor() const { return this->color; } //геттер цвета
    inline const QColor &GetBColor() const { return this->bcolor; } //геттер цвета фона
    inline CondType Condition() const { return this->condtype; } //геттер состояния Tcp объекта
    inline bool isActive() const { return this->active; } //геттер активности Tcp объекта

    //упаковка сообщения в отправляемую форму
    inline const QString PackMessage(const MesType mt, const QString &message) const { return this->pr[mt] + message + "\n"; }

    bool TryPort(const QString &);
    bool TryName(const QString &);
    bool TryColor(const QColor &);

protected:
    bool active = false;
    qint64 port = -1;
    QString name = QString();
    QString error = QString();
    QColor color = Qt::black;
    QColor bcolor = Qt::white;
    CondType condtype = OFF;
    QEventLoop writeLoop;

    const QMap<MesType, QString> pr //словарь связок "тип сообщения - код сообщения"
    {
        {NAME, "/name"},
        {COL, "/color"},
        {BCOL, "/bcolor"},
        {BOT, "/bot"},
        {MES, "/message"}
    };
    const TCPType ttype;

    bool SetPort(const quint64);

private:

protected slots:
    virtual void ReadyRead() = 0;

};

#endif // TCPITEM_H
