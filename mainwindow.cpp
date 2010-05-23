#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qmath.h>
#include <QtGui>

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
    prepareSolutionBrowser();
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
    double allPossible = double(maxLen)/double(requiredLen);
    // Ну или вот столько первых и вторых брусков соответственно
    double firstPossible = allPossible/double(firstCount+secondCount)*firstCount;
    double secondPossible = allPossible/double(firstCount+secondCount)*secondCount;
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
    // Символьные обозначения строк и столбцов
    QStringList rows, cols;
    rows << tr("Z") << tr("S1") << tr("S2") << tr("S3") << tr("S4");
    cols << tr("X") << tr("Y") << tr("Реш.") << tr("Отн.");
    // Симплекс-метод
    int col, row, i, j;
    bool optimal;
    int ir = simplex.at(0).length()-1; // Индекс (номер) колонки "отношение"
    int ia = ir-1;                     // Индекс (номер) колонки "решение"
    double min;
    do {
        // Выбор столбца
        col = 0;
        min = simplex[0][col];
        for (i=1; i<ir; i++) {
            if (simplex[0][i] < min) col = i;
        }
        // Выбор строки
        row = 1;
        while (!simplex[row][col])
            simplex[row++][ir] = 0;
        min = !simplex[row][col] ? 0 : simplex[row][2]/simplex[row][col];
        simplex[row][ir] = !simplex[row][col] ? 0 : simplex[row][ia]/simplex[row][col];
        for (i=row+1; i<simplex.length(); i++) {
            simplex[i][ir] = !simplex[i][col] ? 0 : simplex[i][ia]/simplex[i][col];
            if (simplex[i][ir] < min && simplex[i][ir]) row = i;
        }
        // Рисуем новую симплекс-таблицу
        drawSimplexTable(simplex, rows, cols, row, col);

        rows[row] = cols[col];
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
        for (i=0; i<ia; i++) {
            if (simplex.at(0).at(i) < 0) {
                optimal = false;
            }
        }
    } while (!optimal);
    // Очищаем столбец "отношение" в симплекс-таблице
    for (i=0; i<simplex.count(); i++)
        simplex[i][ir] = 0;
    // FIN
    drawSimplexTable(simplex, rows, cols, -1, -1);
    ui->solutionLabel->setText(tr("%1").arg(simplex[0][2]));
    ui->hintLabel->setText(tr("Всего максимально возможно получить комплектов:"));
}

void MainWindow::onSolutionDisplayChanged(bool state) {
    ui->solutionBrowser->setVisible(state);
    if (state) {
        setGeometry(geometry().x(), geometry().y(),
                    geometry().width(), geometry().height()+200);
        setMinimumHeight(minimumHeight()+200);
    } else {
        setMinimumHeight(minimumHeight()-200);
        setGeometry(geometry().x(), geometry().y(),
                    geometry().width(), geometry().height()-200);
    }
}

void MainWindow::prepareSolutionBrowser() {
    ui->solutionBrowser->clear();
}

void MainWindow::finishSolutionBrowser() {

}

// Рисуем симплекс-таблицу в solutionBrowser
void MainWindow::drawSimplexTable(QList<QList<double> >simplex,
                                  QStringList rows, QStringList cols,
                                  int row, int col)
{
    QTextCursor cursor = ui->solutionBrowser->textCursor();
    cursor.beginEditBlock();

    QTextTableFormat tableFormat;
    tableFormat.setAlignment(Qt::AlignHCenter);
    tableFormat.setCellPadding(2);
    tableFormat.setCellSpacing(0);
    tableFormat.setHeaderRowCount(1);

    QVector<QTextLength> constraints;
    for (int i=0; i<cols.count()+1; i++)
        constraints << QTextLength(QTextLength::PercentageLength, 100/(cols.count()+1));
    tableFormat.setColumnWidthConstraints(constraints);

    QTextTable *table = cursor.insertTable(rows.count()+1, cols.count()+1, tableFormat);

    // Различные форматы текста
    QTextCharFormat format = cursor.charFormat();
    format.setFontPointSize(10);
    QTextCharFormat boldFormat = format;
    boldFormat.setFontWeight(QFont::Bold);
    QTextCharFormat highlightedFormat = boldFormat;
    highlightedFormat.setBackground(Qt::yellow);

    // Устанавливаем заголовки столбцов
    for (int i = 0; i < cols.count(); i++) {
        QTextTableCell cell = table->cellAt(0, i+1);
        QTextCursor cellCursor = cell.firstCursorPosition();
        if (i == col && col >= 0)
            cellCursor.insertText(cols.at(i), highlightedFormat);
        else
            cellCursor.insertText(cols.at(i), boldFormat);
    }
    // Устанавливаем заголовки строк
    for (int i = 0; i < rows.count(); i++) {
        QTextTableCell cell = table->cellAt(i+1, 0);
        QTextCursor cellCursor = cell.firstCursorPosition();
        if (i == row && row >= 0)
            cellCursor.insertText(rows.at(i), highlightedFormat);
        else
            cellCursor.insertText(rows.at(i), boldFormat);
    }
    // Заполняем симплекс-таблицу
    for (int i=0; i<simplex.count(); i++) {
        for (int j=0; j<cols.count(); j++) {
            QTextTableCell cell = table->cellAt(i+1, j+1);
            QTextCursor cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(tr("%1").arg(simplex.at(i).at(j)), format);
        }
    }
    cursor.endEditBlock();
    cursor.movePosition(cursor.End);
    cursor.insertHtml("<br />");
}
