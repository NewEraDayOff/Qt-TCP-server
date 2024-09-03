#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QHostAddress>
#include "tcpitem.h"

class TcpClient : public QTcpSocket, public TcpItem
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient() {} //деструктор
    void SendMessage(const MesType, const QString &) override;
    void SendMessage(const MesType, const QColor &) override;
    bool SetPort(const QString &) override;
    bool SetName(const QString &) override;
    bool SetColor(const QColor &) override;
    bool SetBColor(const QColor &) override;
    bool SetAddress(const QString &);

    bool TryAddress(const QString &);

    bool Open() override;
    bool Close() override;

private:
    unsigned int counter;
    QString address = QString();
    QEventLoop connectLoop;

    bool TryOpen();
    //сеттер состояния клиента с испусканием сигнала при изменении
    void SetCondition(CondType ct)
    {
        if (this->condtype == ct) return;
        this->condtype = ct;
        emit this->ConditionChanged(this->condtype);
    }

private slots:
    void ReadyRead() override;
    void Connected();
    void Disconnected();
    void OnError(SocketError err);

signals:
    void SendError(const QString &);
    void SendInfo(const QString &);
    void ConditionChanged(CondType);

};

#endif // TCPCLIENT_H
