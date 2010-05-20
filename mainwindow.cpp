#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

// Проверка исходных значений
void MainWindow::checkGivenValues() {
    bool OK = true;
    bool dontChangeStatus = false;
    int logsCount = ui->logsCount->value();
    int logLength = ui->logLength->value();
    int firstLength = ui->firstLogLength->value();
    int secondLength = ui->secondLogLength->value();
    // Бруски
    if (firstLength > logLength || secondLength > logLength) {
        if (firstLength > logLength)
            ui->firstLogLength->setValue(logLength);
        if (secondLength > logLength)
            ui->secondLogLength->setValue(logLength);
        ui->statusBar->showMessage(
                tr("Длины брусков не должны превышать длины бревна!"), 5000);
        dontChangeStatus = true; // Чтобы увидели это сообщение -^
        OK = firstLength && secondLength; // Только если указаны обе длины
    } else if (!firstLength || !secondLength) {
        ui->statusBar->showMessage(tr("Введите длины брусков!"));
        OK = false;
    } else OK = true;
    // Бревно
    if (!logLength) {
        ui->statusBar->showMessage(tr("Введите длину бревна!"));
        OK = false;
    }
    if (!logsCount) {
        ui->statusBar->showMessage(tr("Укажите количество брёвен!"));
        OK = false;
    }
    if (OK) {
        ui->hintLabel->setText(tr("Нажмите «решить», чтобы получить решение задачи."));
        if (!dontChangeStatus) ui->statusBar->showMessage(tr("Готов решать!"));
        ui->solveButton->setEnabled(true);
        ui->actionSolve->setEnabled(true);
    } else {
        ui->hintLabel->setText(tr("Исходные данные, похоже, ещё некорректны."));
        ui->solveButton->setEnabled(false);
        ui->actionSolve->setEnabled(false);
    }
}

void MainWindow::fillDefaultValues() {
    ui->logsCount->setValue(20);
    ui->logLength->setValue(5);
    ui->firstLogLength->setValue(2);
    ui->secondLogLength->setValue(3);
}
