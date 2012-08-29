#include <QtGui/QApplication>
#include <QDebug>
#include "chatwidget.h"

#ifdef STATIC
#include <phonon/private/factory_p.h>
#include <phonon/mediaobject.h>
#include <QtPlugin>
Q_IMPORT_PLUGIN(phonon_ds9)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Radio-T");
#ifdef STATIC
    Phonon::Factory::setBackend(qt_plugin_instance_phonon_ds9());
    qRegisterMetaType<Phonon::State>();
#endif
    ChatWidget w;
    w.show();
    
    return a.exec();
}
