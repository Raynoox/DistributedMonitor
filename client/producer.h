//
// Created by root on 5/18/17.
//
#include "spinbuf.h"

#ifndef DISTRIBUTEDMONITOR_PRODUCER_H
#define DISTRIBUTEDMONITOR_PRODUCER_H
class Producer {
public:
    Producer(zmq::context_t &ctx, int arraySize, int procId, int procNum, Spinbuf spinbuf) : spinbuf(spinbuf) {
        cout<<"Process number - "<<procId<<" No. of processes - "<<procNum<<endl;
        proc_id = procId;
    }
    void produce(int position, int value) {
        spinbuf.putInto(position, value);
    }
    void produceNext(int value) {
        spinbuf.putIntoNext(value);
    }
    void* getSpinbuf() {
        return &spinbuf;
    }
private:
    int proc_id;
    Spinbuf spinbuf;
};
#endif //DISTRIBUTEDMONITOR_PRODUCER_H
