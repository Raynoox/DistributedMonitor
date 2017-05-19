//
// Created by root on 4/13/17.
//
#include "spinbuf.h"

#ifndef DISTRIBUTEDMONITOR_CONSUMER_H
#define DISTRIBUTEDMONITOR_CONSUMER_H
class Consumer {
public:
    Consumer(zmq::context_t &ctx, int arraySize, int procId, int procNum, Spinbuf spinbuf) : spinbuf(spinbuf) {
        cout<<"Process number - "<<procId<<" No. of processes - "<<procNum<<endl;
        proc_id = procId;
    }
    int consume(int position) {
        spinbuf.getAndClear(position);
    }
    int consumeNext() {
        spinbuf.getAndClearNext();
    }
    void* getSpinbuf() {
        return &spinbuf;
    }
private:
    int proc_id;
    Spinbuf spinbuf;
};
#endif //DISTRIBUTEDMONITOR_CONSUMER_H
