#include "tcp.h"
#include "ui_tcp.h"

TCP::TCP(QWidget *parent) : QMainWindow(parent) //конструктор
  , tcp(new tcpt(this))
  , ui(new Ui::TCP)
  , bottimer(new QTimer(this))
{
    this->ui->setupUi(this);
    this->bottimer->callOnTimeout(this, &TCP::BotTextSender);

    auto p = QWidget(this).sizePolicy();
    p.setRetainSizeWhenHidden(true);
    this->ui->lineEdit_address->setSizePolicy(p);
    this->ui->label_address->setSizePolicy(p);

    this->ui->widget_con->setVisible(false);
    this->ui->widget_ip->setVisible(false);
    this->ui->widget_bot->setVisible(false);
    this->ui->pushButton_connect->setVisible(false);

    this->ui->widget_bot->setEnabled(false);
    this->ui->lineEdit_mes->setEnabled(false);
    this->ui->pushButton_send->setEnabled(false);

    this->ui->label_address_i->setStyleSheet(QString("background-color:%1;").arg(QColor(Qt::red).name()));
    this->ui->label_port_i->setStyleSheet(QString("background-color:%1;").arg(QColor(Qt::red).name()));
    this->ui->label_name_i->setStyleSheet(QString("background-color:%1;").arg(QColor(Qt::red).name()));
    this->ui->label_color_i->setStyleSheet(QString("background-color:%1;").arg(QColor(Qt::black).name()));
    this->ui->label_bcolor_i->setStyleSheet(QString("background-color:%1;").arg(QColor(Qt::white).name()));

    this->ui->radioButton_client->setProperty("type", TcpItem::CLIENT);
    this->ui->radioButton_server->setProperty("type", TcpItem::SERVER);
    QObject::connect(this->ui->radioButton_client, &QRadioButton::clicked, this, &TCP::SetUserType);
    QObject::connect(this->ui->radioButton_server, &QRadioButton::clicked, this, &TCP::SetUserType);

    QObject::connect(this->tcp->client(), &TcpClient::ConditionChanged, this, &TCP::ClientCondChanged);
    QObject::connect(this->tcp->server(), &TcpServer::ConditionChanged, this, &TCP::ServerCondChanged);

    QObject::connect(this->tcp->client(), &TcpClient::SendInfo, this->ui->textEdit_chat, &QTextEdit::append);
    QObject::connect(this->tcp->server(), &TcpServer::SendInfo, this->ui->textEdit_chat, &QTextEdit::append);

    QObject::connect(this->ui->lineEdit_mes, &QLineEdit::returnPressed, this, &TCP::on_pushButton_send_clicked);
}

TCP::~TCP() //деструктор
{
    if (this->bottimer->isActive()) this->bottimer->stop();
    delete this->tcp;
    delete this->ui;
}

void TCP::SetUserType() //настройки отображения виджетов на левой панели в зависимости от Tcp объекта
{
    auto type = qvariant_cast<TcpItem::TCPType>(qobject_cast<QObject *>(QObject::sender())->property("type"));

    const bool noinfo = type == TcpItem::NOINFO;
    this->ui->widget_con->setVisible(!noinfo);
    this->ui->widget_ip->setVisible(!noinfo);
    this->ui->widget_bot->setVisible(!noinfo);
    this->ui->pushButton_connect->setVisible(!noinfo);

    const bool client = type == TcpItem::CLIENT;
    this->ui->label_address->setVisible(client);
    this->ui->label_address_i->setVisible(client);
    this->ui->lineEdit_address->setVisible(client);
    this->ui->widget_ip->setVisible(!client);

    this->ui->lineEdit_mes->setEnabled(false);
    this->ui->pushButton_send->setEnabled(false);
    this->ui->lineEdit_address->clear();
    this->ui->listWidget_ip->clear();

    this->SetText(this->ui->pushButton_connect, type == TcpItem::CLIENT ? "Подключиться" : "Создать сервер");

    if (this->tcp->item() != nullptr
     && this->tcp->item()->isActive())
        this->tcp->item()->Close();
    this->tcp->SetTCPType(type);
}

void TCP::on_pushButton_connect_clicked() //кнопка подключения
{
    if (!this->tcp->item()->isActive() && this->tcp->item()->Condition() == TcpItem::OFF)
    {
        const QString port = this->ui->lineEdit_port->text().trimmed();
        const QString name = this->ui->lineEdit_name->text().trimmed();
        const QString address = this->ui->lineEdit_address->text().trimmed();
        if (!this->tcp->item()->SetPort(port)) return;
        if (!this->tcp->item()->SetName(name)) return;
        if (this->tcp->type == TcpItem::CLIENT && !this->tcp->client()->SetAddress(address)) return;

        this->tcp->item()->Open();
    }
    else this->tcp->item()->Close();
}

void TCP::SetEnabledOnConnect(bool enabled) //сеттер включения виджетов левой панели в зависимости от активности Tcp объекта
{
    if (!enabled && this->bottimer->isActive())
    {
        this->SetText(this->ui->pushButton_strtbot, "Запустить спам-бота");
        this->bottimer->stop();
    }

    this->ui->lineEdit_address->setEnabled(!enabled);
    this->ui->lineEdit_name->setEnabled(!enabled);
    this->ui->lineEdit_port->setEnabled(!enabled);

    this->ui->widget_bot->setEnabled(enabled);
    this->ui->lineEdit_mes->setEnabled(enabled);
    this->ui->pushButton_send->setEnabled(enabled);
}

void TCP::ClientCondChanged(TcpItem::CondType ct) //слот при изменении состояния клиента
{
    this->SetEnabledOnConnect(this->tcp->item()->isActive());
    QString buttonText;
    switch (ct)
    {
    case TcpItem::LOOKUP: buttonText = "Прервать поиск"; break;
    case TcpItem::OFF: buttonText = "Подключиться"; break;
    case TcpItem::ON: buttonText = "Отключиться"; break;
    }
    this->SetText(this->ui->pushButton_connect, buttonText);
}

void TCP::ServerCondChanged(TcpItem::CondType ct) //слот при изменении состояния сервера
{
    this->SetEnabledOnConnect(this->tcp->item()->isActive());
    QString buttonText;
    switch (ct)
    {
    case TcpItem::LOOKUP: buttonText = "Закрыть сервер"; break;
    case TcpItem::OFF: buttonText = "Создать сервер"; break;
    case TcpItem::ON: buttonText = "Закрыть сервер"; break;
    }
    this->SetText(this->ui->pushButton_connect, buttonText);
}

void TCP::on_lineEdit_port_textEdited(const QString &arg1) //изменение цвета индикатора при изменении порта
{
    const QColor tcolor(this->tcp->item()->TryPort(arg1) ? Qt::darkGreen : Qt::red);
    this->SetColor(this->ui->label_port_i, tcolor);
}

void TCP::on_lineEdit_address_textEdited(const QString &arg1) //изменение цвета индикатора при изменении адреса
{
    const QColor tcolor(this->tcp->client()->TryAddress(arg1) ? Qt::darkGreen : Qt::red);
    this->SetColor(this->ui->label_address_i, tcolor);
}

void TCP::on_lineEdit_name_textEdited(const QString &arg1) //изменение цвета индикатора при изменении имени
{
    const QColor tcolor(this->tcp->item()->TryName(arg1.trimmed()) ? Qt::darkGreen : Qt::red);
    this->SetColor(this->ui->label_name_i, tcolor);
}

void TCP::on_pushButton_color_clicked() //кнопка смены цвета
{
    const QColor tcolor(QColorDialog::getColor(this->tcp->item()->GetColor(), this, "Цвет ника"));
    if (!this->tcp->item()->TryColor(tcolor)) return;
    if (this->tcp->item()->isActive()) this->tcp->item()->SendMessage(TcpItem::COL, tcolor);
    if (!this->tcp->item()->SetColor(tcolor)) return;
    this->SetColor(this->ui->label_color_i, tcolor);
}

void TCP::on_pushButton_bcolor_clicked() //кнопка смены цвета фона
{
    const QColor tcolor(QColorDialog::getColor(this->tcp->item()->GetBColor(), this, "Цвет ника"));
    if (!this->tcp->item()->TryColor(tcolor)) return;
    if (this->tcp->item()->isActive()) this->tcp->item()->SendMessage(TcpItem::BCOL, tcolor);
    if (!this->tcp->item()->SetBColor(tcolor)) return;
    this->SetColor(this->ui->label_bcolor_i, tcolor);
}

void TCP::on_pushButton_send_clicked() //кнопка отправки сообщения
{
    const QString text = this->ui->lineEdit_mes->text().trimmed();
    if (text.isEmpty()) return;
    this->tcp->item()->SendMessage(TcpItem::MES, text);
    this->ui->lineEdit_mes->clear();
    this->ui->lineEdit_mes->setFocus();
}

void TCP::on_pushButton_ip_clicked() //кнопка отображения IP
{
    this->ui->listWidget_ip->clear();
    const auto &localhost = QHostAddress(QHostAddress::LocalHost);

    const QList<QHostAddress> addresses = QHostInfo::fromName(QHostInfo::localHostName()).addresses();
    foreach (auto adrs, addresses)
        if (adrs.protocol()  == QAbstractSocket::IPv4Protocol && adrs != localhost)
            this->ui->listWidget_ip->addItem(adrs.toString());
}

void TCP::BotTextSender() //рассылка сообщений бота
{ this->tcp->item()->SendMessage(TcpItem::BOT, this->ui->textEdit_bot->toPlainText()); }

void TCP::on_pushButton_strtbot_clicked() //кнопка включения бота
{
    if (!this->bottimer->isActive())
    {
        if (this->ui->textEdit_bot->toPlainText().trimmed().isEmpty())
        {
            QMessageBox::warning(this, "Ошибка", "Задайте сообщение для бота!");
            return;
        }
        this->SetText(this->ui->pushButton_strtbot, "Остановить спам-бота");
        this->bottimer->start(1000);
    }
    else
    {
        this->SetText(this->ui->pushButton_strtbot, "Запустить спам-бота");
        this->bottimer->stop();
    }
}

