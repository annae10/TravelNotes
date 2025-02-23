#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include "worker.h"

namespace Ui{
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void startWorker();
    void startWorker2();
    void errorString(QString err);

private:
    Ui::MainWindow *ui;
    Worker *worker;
};

#endif // MAINWINDOW_H
