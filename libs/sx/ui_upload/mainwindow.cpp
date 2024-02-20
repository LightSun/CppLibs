#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "pub_api.h"
#include <qdebug.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto vec = med_api::show_uploads_dlg();
    for(auto& f: vec){
        qDebug() << QString::fromStdString(f);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

