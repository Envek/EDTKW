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
    double firstPossible, secondPossible;
    // ОСТОРОЖНО, КОСТЫЛЬ! Расчитываем кол-во брусков в завис. от суммы их длин
    if (requiredLen<=logLength) {
        firstPossible = allPossible/double(firstCount+secondCount)*firstCount;
        secondPossible = allPossible/double(firstCount+secondCount)*secondCount;
    } else { // Число 2.0 ниже имеет сакральный смысл и тщательно подобрано
        firstPossible = double(logsCount*(logLength/firstLength))/
                               double(firstCount+secondCount)/2.0*firstCount;
        secondPossible = double(logsCount*(logLength/secondLength))/
                               double(firstCount+secondCount)/2.0*secondCount;
    } // ОСТОРОЖНО, КОСТЫЛЬ! Исправление бага с нерасчётом нечётных ответов
    if (firstPossible - floor(firstPossible) >= 0.5 &&
        secondPossible - floor(secondPossible >= 0.5)) {
        firstPossible = floor(firstPossible) + 0.5;
        secondPossible = floor(secondPossible) + 0.5;
    } else { // Целочисленность результата гарантируются (увеличиваются оба)
        firstPossible = floor(firstPossible);
        secondPossible = floor(secondPossible);
    } // Конец глобальных костылей
    // Симплекс-таблица
    QList< QList<double> > simplex;
    // Z-строка
    QList<double> z;
    z << -firstCount << -secondCount << 0 << 0;
    simplex.append(z);
    // 1-я строка (S1)
    QList<double> s1;
    s1 << firstCount << 0 << firstPossible << 0; // Must be floor(firstPossible)
    simplex.append(s1);
    // 2-я строка (S2)
    QList<double> s2;
    s2 << firstCount << 0 << floor(firstPossible)+1 << 0;
    simplex.append(s2);
    // 3-я строка (S3)
    QList<double> s3;
    s3 << 0 << secondCount << secondPossible << 0; // Must be floor(secondPossible)
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
        min = simplex[row][col]<=0 ? 16777215 : simplex[row][ia]/simplex[row][col];
        simplex[row][ir] = simplex[row][col]<=0 ? 0 : simplex[row][ia]/simplex[row][col];
        for (i=row+1; i<simplex.length(); i++) {
            simplex[i][ir] = !simplex[i][col] ? 0 : simplex[i][ia]/simplex[i][col];
            if (simplex[i][ir] < min && simplex[i][ir]) {
                row = i;
                min = simplex[i][ir];
            }
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
    printCuttingPlan(simplex[0][ia], logLength, logsCount, firstLength,
                     firstCount, secondLength, secondCount);
}

void MainWindow::onSolutionDisplayChanged(bool state) {
    ui->solutionBrowser->setVisible(state);
    if (state) {
        setGeometry(geometry().x(), geometry().y(),
                    geometry().width(), geometry().height()+225);
        setMinimumHeight(minimumHeight()+225);
    } else {
        setMinimumHeight(minimumHeight()-225);
        setGeometry(geometry().x(), geometry().y(),
                    geometry().width(), minimumHeight());
    }
}

void MainWindow::prepareSolutionBrowser() {
    ui->solutionBrowser->clear();
    QTextCursor cursor = ui->solutionBrowser->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(cursor.StartOfLine);
    cursor.insertHtml(QString("<h2>") + tr("Ход решения") + QString("</h2>"));
    cursor.endEditBlock();
}

void MainWindow::printCuttingPlan(int answer, int logLength, int logsCount,
                                  int firstLen, int firstCount,
                                  int secondLen, int secondCount)
{
    QTextCursor cursor = ui->solutionBrowser->textCursor();
    cursor.movePosition(cursor.Start);
    cursor.beginEditBlock();
    cursor.insertHtml(QString("<h2>") + tr("План распила") + QString("</h2>"));
    QStringList plan;
    int first = answer*firstCount;
    int second = answer*secondCount;
    for (int i=0; i<logsCount; i++) {
        int firstThere = 0, secondThere = 0, length=logLength;
        if (firstLen < secondLen)
           qSwap(firstLen, secondLen);
        while (first && length >= firstLen) {
            firstThere++;
            length -= firstLen;
            first--;
        }
        while (second && length >= secondLen) {
            secondThere++;
            length -= secondLen;
            second--;
        }
        if (firstThere || secondThere)
            plan << QString("<li>") +
                    tr("Первых брусков: <strong>%1</strong>; вторых: <strong>"
                       "%2</strong>; метров остатков: %3;").arg(firstThere)
                    .arg(secondThere).arg(length) + QString("</li>");
        else
            plan << QString("<li>") +
                    tr("Нет смысла распиливать - комплекта уже не получить")
                    + QString("</li>");
    }
    cursor.insertHtml(QString(" <ol> ") + plan.join(QString(" ")) + QString(" </ol> "));
    cursor.insertHtml(QString(" <br /> <br /> "));
    cursor.endEditBlock();
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
            if (j!=cols.length()-2) // Костыль, чтобы скрыть действие других костылей
                cellCursor.insertText(tr("%1").arg(simplex.at(i).at(j)), format);
            else // В столбце "Решение" просто отбрасываем дробную часть и всё ОК
                cellCursor.insertText(tr("%1").arg(int(simplex.at(i).at(j))), format);
        }
    }
    cursor.endEditBlock();
    cursor.movePosition(cursor.End);
    cursor.insertHtml("<br />");
}

void MainWindow::showAbout() {
        QMessageBox::about(this, tr("О программе"),
                     tr("<center><h2>КУРСОВАЯ РАБОТА</h2></center>"
                        "<p>по дисциплине «Теория принятия решений»</p>"
                        "<p>По теме: «линейное программирование»</p><p></p>"
                        "<p>Исполнитель: студент группы 753   А.А. Новиков</p>"
                        "<br /><br /><p>Курсовая работа написана на языке "
                        "программирования <strong>C++</strong> с использованием "
                        "програмного каркаса <strong>Qt</strong></p>"
                        "<p>Исходный код, информация о программе, история "
                        "разработки и новые версии могут быть найдены здесь: "
                        "<a href='http://github.com/Envek/EDTKW'>"
                        "http://github.com/Envek/EDTKW</a>.</p>"
                        "<br /><p>Данная программа не одобрена Greenpeace.<br />"
                        "Greenpeace запрещает использовать данную программу.</p>"
                        "<p><big>プログラムを使用していただきありがとうございます。</big></p>"
                        ));
}

void MainWindow::showHelp() {
    QMessageBox::about(this, tr("Помощь"),
                       tr("<h3>Помощь</h3>"
                          "<p>Введите все требуемые значения в поля ввода или "
                          "выберите нужное посредством стрелок возле полей "
                          "ввода и нажмите «Решить».</p>"
                          "<p>До тех пор, пока вы не введёте корректные данные "
                          "во все поля ввода, кнопка «Решить» будет неактивной</p>"
                          "<p>Для ввода значений по умолчанию (указанных в "
                          "условии к задаче) используйте пункт меню Задача -> "
                          "Значения по умолчанию или комбинацию клавиш Ctrl+D "
                          "и затем снова нажмите «Решить» (или клавишу Enter).</p>"
                          "<p>Над блоком кнопок будет выведен ответ, а в "
                          "текстовой области внизу будет показан план распила "
                          "для всех брёвен, а так же ход решения, причём "
                          "первая (верхняя) симплекс-таблица - исходная, "
                          "последняя - результирующая. Во всех таблицах, кроме "
                          "результирующей, цветом выделены разрешающие строка "
                          "и столбец.</p>"
                          ));
}

void MainWindow::showTask() {
    QMessageBox::about(this, tr("Задача"),
                 tr("<h3>Задание</h3>"
                    "<p>Составить математическое описание, выбрать метод "
                    "математического программирования и написать программу "
                    "для реализации следующей задачи:</p>"
                    "<p>Необходимо распилить 20 брёвен длиной по 5 метров "
                    "каждое на бруски по 2 и 3 метра, при этом должно "
                    "получиться равное количество брусков каждого размера. "
                    "Составить такой план распила, при котором будет получено "
                    "максимальное количество комплектов и все брёвна будут "
                    "распилены (в один комплект входит по одному бруску "
                    "каждого размера).</p>"
                    ));
}
