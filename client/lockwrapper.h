//
// Created by root on 5/19/17.
//
#include "monitor.h"
#ifndef DISTRIBUTEDMONITOR_LOCKWRAPPER_H
#define DISTRIBUTEDMONITOR_LOCKWRAPPER_H


class Lockwrapper {
public:

    Lockwrapper(Monitor* m) : monitor(m){
        monitor->lock();
    }
    ~Lockwrapper() {
        monitor->unlock();
    }
private:
    Monitor* monitor;
};


#endif //DISTRIBUTEDMONITOR_LOCKWRAPPER_H
