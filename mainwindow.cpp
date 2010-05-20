#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qmath.h>

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

void MainWindow::solve() {
    int logsCount    = ui->logsCount->value();
    int logLength    = ui->logLength->value();
    int firstLength  = ui->firstLogLength->value();
    int secondLength = ui->secondLogLength->value();
    int firstCount   = 1;
    int secondCount  = 1;
    int maxLen = logsCount*logLength;
    int requiredLen = firstCount*firstLength + secondCount*secondLength;
    double allPossible = maxLen/requiredLen;
    double firstPossible = allPossible/(firstCount+secondCount)*firstCount;
    double secondPossible = allPossible/(firstCount+secondCount)*secondCount;
    // Симплекс-таблица
    QList< QList<double> > simplex;
    // Z-строка
    QList<double> z;
    z << -firstCount << -secondCount << 0 << 0;
    simplex.append(z);
    // 1st row
    QList<double> s1;
    s1 << firstCount << 0 << floor(firstPossible) << 0;
    simplex.append(s1);
    // 2nd row
    QList<double> s2;
    s2 << firstCount << 0 << floor(firstPossible)+1 << 0;
    simplex.append(s2);
    // 3rd row
    QList<double> s3;
    s3 << 0 << secondCount << floor(secondPossible) << 0;
    simplex.append(s3);
    // 4th row
    QList<double> s4;
    s4 << 0 << secondCount << floor(secondPossible)+1 << 0;
    simplex.append(s4);
    // Симплекс-метод
    int col, row, i, j;
    bool optimal;
    do {
        // Выбор столбца
        col = simplex.at(0).at(1) < simplex.at(0).at(0) ? 1 : 0;
        // Выбор строки
        row = 1;
        double min = simplex.at(1).at(2)/simplex.at(1).at(col);
        for (i=1; i<5; i++) {
           simplex[i][3] = simplex[i][2]/simplex[i][col];
           if (simplex.at(i).at(3) < min) row = i;
        }
        // DO A BARREL ROLL
        for (i=0; i<simplex.length(); i++) {
            if (i!=row) {
                double multiplier = -simplex[i][col]/simplex[row][col];
                for (j=0; j<simplex[i].length()-1; j++) {
                    simplex[i][j] += simplex[row][j]*multiplier;
                }
            }
        }
        double multiplier = 1.0/simplex[row][col];
        for (j=0; j<simplex[row].length()-1; j++) {
            simplex[row][j] += simplex[row][j]*multiplier;
        }
        // Проверка оптимальности
        optimal = true;
        for (i=0; i<2; i++) {
            if (simplex.at(0).at(i) < 0) {
                optimal = false;
            }
        }
    } while (!optimal);
    ui->solutionLabel->setText(tr("%1").arg(simplex[0][2]));
}
