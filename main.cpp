#include <QtGui/QApplication>
#include <QDebug>
#include "chatwidget.h"

#if defined(STATIC) && defined(Q_OS_WIN32)
#include <phonon/private/factory_p.h>
#include <phonon/mediaobject.h>
#include <QtPlugin>
Q_IMPORT_PLUGIN(phonon_ds9)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Radio-T");
#if defined(STATIC) && defined(Q_OS_WIN32)
    Phonon::Factory::setBackend(qt_plugin_instance_phonon_ds9());
    qRegisterMetaType<Phonon::State>();
#endif
    ChatWidget w;
    w.show();
    
    return a.exec();
}
