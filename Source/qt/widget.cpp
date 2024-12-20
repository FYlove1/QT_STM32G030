#include "widget.h"
#include "ui_widget.h"


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    udp_socket = new QUdpSocket(this);
    udp_socket->bind(8080,QUdpSocket::ShareAddress|QAbstractSocket::ReuseAddressHint);
    setWindowTitle("服务器的端口号是：8080");
    //当服务器接受到连接信息时
    connect(udp_socket,&QUdpSocket::readyRead,this,
            &Widget::dealMsg);
}
void Widget::dealMsg(){
    //char buff[1024]={0};
    QByteArray buff;
    QHostAddress client_Addr;
    quint16 port;
    buff.resize(udp_socket->pendingDatagramSize());
    qint64 len = udp_socket->readDatagram(buff.data(),buff.size(),&client_Addr,&port);
    qDebug()<< buff;
    if(len>0){
        QString str = QString("[%1:%2 %3]").
                arg(client_Addr.toString()).
                arg(port).
                arg(QString::fromLocal8Bit(buff));
        ui->textEdit->setText(str);
    }

}
Widget::~Widget()
{
    delete ui;
}
//写文本
void Widget::on_send_bt_clicked()
{
    QString ip = ui->IP_lineEdit->text();
    qint16 port = ui->port_lineEdit_2->text().toInt();
    QString array = ui->textEdit->toPlainText();
    udp_socket->writeDatagram(array.toLocal8Bit(),QHostAddress(ip),port);
}

void Widget::on_light_on_clicked()
{
    QString ip = ui->IP_lineEdit->text();
    qint16 port = ui->port_lineEdit_2->text().toInt();
    QString array = "on";
    udp_socket->writeDatagram(array.toLocal8Bit(),QHostAddress(ip),port);
}

void Widget::on_pushButton_3_clicked()
{
    QString ip = ui->IP_lineEdit->text();
    qint16 port = ui->port_lineEdit_2->text().toInt();
    QString array = "off";
    udp_socket->writeDatagram(array.toLocal8Bit(),QHostAddress(ip),port);
}
