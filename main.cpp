#include <QtGui/QApplication>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());  // Для внутренних преобразований Qt
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8")); // Для функций перевода tr()
    MainWindow w;
    w.show();
    return a.exec();
}
