#ifndef TCP_H
#define TCP_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>
#include <QColorDialog>
#include <QPushButton>
#include <QHostInfo>
#include <QTimer>
#include "tcpserver.h"
#include "tcpclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TCP; }
QT_END_NAMESPACE

Q_DECLARE_METATYPE(TcpItem::TCPType)

class TCP : public QMainWindow
{
    Q_OBJECT

public:
    TCP(QWidget *parent = nullptr);
    ~TCP();
    //структура, содержащая в себе экземпляры клиента и сервера, а также методы взаимодействия с ними
    struct tcpt
    {
    public:
        tcpt(QWidget *parent) : parent(parent) //конструктор
        {
            if (this->s == nullptr)
            {
                //создаем экземпляр сервера
                this->s = new TcpServer(this->parent);
                //подключаем вызов ошибок
                QObject::connect(this->s, &TcpServer::SendError, this->parent, [=](const QString &etext)
                {QMessageBox::warning(this->parent, "Ошибка", etext);});
                //подключаем вызов оповещений
                QObject::connect(this->s, &TcpServer::SendOpenServerInfo, this->parent, [=](const QString &info)
                {QMessageBox::information(this->parent, "Сервер открыт", info);});
            }

            if (this->c == nullptr)
            {
                //создаем экземпляр клиента
                this->c = new TcpClient(this->parent);
                //подключаем вызов ошибок
                QObject::connect(this->c, &TcpClient::SendError, this->parent, [=](const QString &etext)
                {QMessageBox::warning(this->parent, "Ошибка", etext);});

            }
        }
        ~tcpt() //деструктор с отключением и удалением экземпляров сервера и клиента
        {
            if (this->s != nullptr)
            {
                if (this->s->isListening())
                    this->s->Close();
                this->s->deleteLater();
                this->s = nullptr;
            }
            if (this->c != nullptr)
            {
                if (this->c->isOpen())
                    this->c->Close();
                this->c->deleteLater();
                this->c = nullptr;
            }
            this->parent = nullptr;
        }
        //тип рабочего экземпляра
        inline void SetTCPType(TcpItem::TCPType type) { this->type = type; }

        TcpItem *item() const//обращение к экземпляру через базовый класс
        {
            if (this->s != nullptr && this->type == TcpItem::SERVER)
            { return this->s; }
            if (this->c != nullptr && this->type == TcpItem::CLIENT)
            { return this->c; }
            return nullptr;
        }
        TcpServer *server() const { return this->s; } //обращение к экземпляру сервера
        TcpClient *client() const { return this->c; } //обращение к экземпляру клиента
        TcpItem::TCPType type = TcpItem::NOINFO;

    private:
        tcpt(); //базовый конструктор закрыт для пользования
        tcpt(const tcpt &); //конструктор копирования закрыт для пользования
        QWidget *parent = nullptr; //родитель закрыт для пользования
        TcpServer *s = nullptr;
        TcpClient *c = nullptr;
    } *tcp = nullptr; //указатель на экземпляр структуры

private:
    Ui::TCP *ui;
    QTimer *bottimer = nullptr;
    //сеттер цвета для QLabel-индикаторов
    inline void SetColor(QLabel *l, QColor c) { l->setStyleSheet(QString("background-color:%1;").arg(c.name())); }
    void SetEnabledOnConnect(bool enabled);

private slots:
    void SetUserType();
    //сеттер текста на кнопке
    void SetText(QPushButton *pb, QString text)
    { if (pb == nullptr) return; pb->setText(text); }
    void ClientCondChanged(TcpItem::CondType);
    void ServerCondChanged(TcpItem::CondType);
    void BotTextSender();

    void on_pushButton_connect_clicked();
    void on_pushButton_color_clicked();
    void on_lineEdit_port_textEdited(const QString &arg1);
    void on_lineEdit_address_textEdited(const QString &arg1);
    void on_pushButton_bcolor_clicked();
    void on_lineEdit_name_textEdited(const QString &arg1);
    void on_pushButton_send_clicked();
    void on_pushButton_ip_clicked();
    void on_pushButton_strtbot_clicked();
};
#endif // TCP_H
