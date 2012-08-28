#include <QtGui/QApplication>
#include <QDebug>
#include "chatwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Radio-T");
    ChatWidget w;
    w.show();
    
    return a.exec();
}
