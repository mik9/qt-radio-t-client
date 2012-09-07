#ifndef SLEEPER_H
#define SLEEPER_H

class Sleeper : public QThread {
public:
    static void msleep(unsigned long v) { QThread::msleep(v); }
};

#endif // SLEEPER_H
