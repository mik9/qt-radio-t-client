#ifndef SLEEPER_H
#define SLEEPER_H

#include <QThread>

class Sleeper : public QThread {
public:
    static void msleep(long v) {
        if (v > 0) {
            QThread::msleep(v);
        }
    }
    static void sleep(long v) {
        if (v > 0) {
            QThread::sleep(v);
        }
    }
    static void usleep(long v) {
        if (v > 0) {
            QThread::usleep(v);
        }
    }
};

#endif // SLEEPER_H
