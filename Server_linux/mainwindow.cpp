#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::startWorker);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::startWorker2);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startWorker()
{
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

void MainWindow::startWorker2()
{
    QThread *thread = new QThread;
    Worker *worker = new Worker();

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Worker::process2);
    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &Worker::error, this, &MainWindow::errorString);

    thread->start();
}

void MainWindow::errorString(QString err)
{
    qDebug()<<"Error:"<<err;
}
