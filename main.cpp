#include <QtGui/QApplication>
#include <QDebug>
#include "chatwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatWidget w;
    w.show();
    
    return a.exec();
}
