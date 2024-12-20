#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QHostAddress>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    QUdpSocket *udp_socket;
private slots:
    void dealMsg();
    void on_send_bt_clicked();
    void on_light_on_clicked();
    void on_pushButton_3_clicked();
};

#endif // WIDGET_H
