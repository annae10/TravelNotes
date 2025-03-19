#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "worker.h"

namespace Ui{
    class MainWindow;
}

class MainWindow: public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void updateText(const QString &text);
    void clearText();
    void startWorker();
    void startWorker2();
    void errorString(QString err);
private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    Worker *worker;
};

#endif // MAINWINDOW_H
