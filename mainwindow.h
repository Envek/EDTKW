#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTextBrowser;
QT_END_NAMESPACE

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;

private slots:
    void checkGivenValues();
    void fillDefaultValues();
    void solve();
    void onSolutionDisplayChanged(bool state);

    void prepareSolutionBrowser();
    void printCuttingPlan(int answer, int logLength, int logsCount,
                          int firstLen, int firstCount,
                          int secondLen, int secondCount);
    void drawSimplexTable(const QList< QList <double> > simplex,
                         const QStringList rows, const QStringList cols,
                         const int row, const int col);

signals:
    void newSimplexTable(const QList< QList <double> > simplex,
                         const QStringList rows, const QStringList cols,
                         const int row, const int col);

};

#endif // MAINWINDOW_H
