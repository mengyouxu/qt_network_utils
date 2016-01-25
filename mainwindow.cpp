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
    ui->setupUi(this);

    //init network_utils here
    nu = new network_utils();
    string *target_host = new string("www.baidu.com");
    list<string> host_addr = nu->getHostByName(target_host);
    nu->pingHost(target_host);
    for(list<string>::iterator i=host_addr.begin();i!=host_addr.end();i++){
        std::cout<<"addr : "<<(*i).c_str()<<std::endl;
    }
    std::flush(std::cout);
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


void MainWindow::on_pushButtonGetHostname_clicked()
{
    string *hostname = nu->getHostName();
    ui->labelHostname->setText(hostname->c_str());
}
