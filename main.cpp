#include <QtGui/QApplication>
#include <QDebug>
#include "chatwidget.h"
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef QT_NO_DEBUG
    a.addLibraryPath(QDir::current().filePath("plugins"));
#endif
    ChatWidget w;
    w.show();
    
    return a.exec();
}
