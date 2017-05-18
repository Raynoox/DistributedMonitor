//
// Created by root on 5/3/17.
//


//
// Created by root on 4/13/17.
//
#include "Table.h"
#include "monitor.h"
#ifndef DISTRIBUTEDMONITOR_PHILOSOPHER_H
#define DISTRIBUTEDMONITOR_PHILOSOPHER_H
class Philosopher {
public:
    Philosopher(zmq::context_t &ctx, int arraySize, int procId, int procNum, Table table) : table(table) {
        cout<<"Process number - "<<procId<<" No. of processes - "<<procNum<<endl;
        proc_id = procId;
    }
    void eat() {
        table.pickForks(proc_id);
        cout<<"EATING"<<endl;
        sleep(5);
        table.releaseForks(proc_id);
        cout<<"END"<<endl;
    }
    void* getTable() {
        return &table;
    }
private:
    int proc_id;
    Table table;
};
#endif //DISTRIBUTEDMONITOR_PHILOSOPHER_H


