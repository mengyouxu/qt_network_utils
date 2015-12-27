#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostInfo>
#include <QNetworkInterface>
#include <QMessageBox>

#include "network_utils.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Test network_utils here
    class network_utils *nu = new network_utils();
    string *target_host = new string("www.google.com");
    string *hostname = nu->getHostName();
    std::cout<<"hostname : "<<hostname->c_str()<<std::endl;
    nu->getHostByName(target_host);
    std::flush(std::cout);

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString detail = "";
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    for(int i=0;i<list.count();i++){
        QNetworkInterface interface=list.at(i);
        detail = detail + tr("设备：") + interface.name() + "\n";
        detail = detail + tr("硬件地址： ") + interface.hardwareAddress()+ "\n";
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        for(int j=0;j<entryList.count();j++){
            QNetworkAddressEntry entry = entryList.at(j);
            detail = detail + "\t" + tr("IP 地址： ") + entry.ip().toString() + "\n";
            detail = detail + "\t" + tr("子网掩码： ") + entry.netmask().toString()+"\n";
            detail = detail + "\t" + tr("广播地址： ") + entry.broadcast().toString() +"\n";

        }
    }
    QMessageBox::information(this,tr("Detail"),detail);
}

