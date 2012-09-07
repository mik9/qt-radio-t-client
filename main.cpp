#include <QtGui/QApplication>
#include <QDebug>
#include "chatwidget.h"
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<size_t>("size_t");
    a.setApplicationName("qt/Radio-T client");
    a.setOrganizationName("mik_os");
#ifdef QT_NO_DEBUG
    a.addLibraryPath(QDir::current().filePath("plugins"));
#endif
    ChatWidget w;
    w.show();
    
    return a.exec();
}
