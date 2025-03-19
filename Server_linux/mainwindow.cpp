#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sslserver.h"
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QThread *thread = new QThread;
    Worker *worker = new Worker();

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Worker::process);
    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &Worker::error, this, &MainWindow::errorString);

    thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::errorString(QString err)
{
    qDebug()<<"Error:"<<err;
}
