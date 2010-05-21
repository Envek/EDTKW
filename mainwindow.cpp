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
    int logsCount    = ui->logsCount->value();       // Количество брёвен всего
    int logLength    = ui->logLength->value();       // Длина одного бревна
    int firstLength  = ui->firstLogLength->value();  // Длина первого бруска
    int secondLength = ui->secondLogLength->value(); // Длина второго бруска
    int firstCount   = 1; // Кол-ва брусков в комплекте сделаны фиксированными.
    int secondCount  = 1; // Но, если хотите, можно вынести и крутилку в GUI.
    int maxLen = logsCount*logLength; // Если б было одно бревно такой длины...
    // Длина идеального бревна на 1 комплект
    int requiredLen = firstCount*firstLength + secondCount*secondLength;
    // Чисто теоретически можно сделать столько комплектов
    double allPossible = maxLen/requiredLen;
    // Ну или вот столько первых и вторых брусков соответственно
    double firstPossible = allPossible/(firstCount+secondCount)*firstCount;
    double secondPossible = allPossible/(firstCount+secondCount)*secondCount;
    // Симплекс-таблица
    QList< QList<double> > simplex;
    // Z-строка
    QList<double> z;
    z << -firstCount << -secondCount << 0 << 0;
    simplex.append(z);
    // 1-я строка (S1)
    QList<double> s1;
    s1 << firstCount << 0 << floor(firstPossible) << 0;
    simplex.append(s1);
    // 2-я строка (S2)
    QList<double> s2;
    s2 << firstCount << 0 << floor(firstPossible)+1 << 0;
    simplex.append(s2);
    // 3-я строка (S3)
    QList<double> s3;
    s3 << 0 << secondCount << floor(secondPossible) << 0;
    simplex.append(s3);
    // 4-я строка (S4)
    QList<double> s4;
    s4 << 0 << secondCount << floor(secondPossible)+1 << 0;
    simplex.append(s4);
    // Симплекс-метод
    int col, row, i, j;
    bool optimal;
    do {
        // Выбор столбца 1 или 2 (на данный момент примитивно и без расширяемости)
        col = simplex.at(0).at(1) < simplex.at(0).at(0) ? 1 : 0;
        // Выбор строки
        row = 1;
        while (!simplex[row][col])
            row++;
        double min = !simplex[row][col] ? 0 : simplex[row][2]/simplex[row][col];
        for (i=row+1; i<5; i++) {
            simplex[i][3] = !simplex[i][col] ? 0 : simplex[i][2]/simplex[i][col];
            if (simplex[i][3] < min && simplex[i][3]) row = i;
        }
        // Непосредственно вся соль симплекс-метода
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
            simplex[row][j] *= multiplier;
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
