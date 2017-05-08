//
// Created by root on 5/3/17.
//
#include <string>
#include "philosopher.h"
#include "configuration.h"

class client_task {
public:
    client_task(int pid)
            : ctx_(1),
              client_socket_(ctx_, ZMQ_DEALER),
              conf() {
            PID = pid;
    }

    void start() {

        int i = 10;
        Philosopher phi = Philosopher(ctx_,conf.PROC_NUM,PID,conf.PROC_NUM);
        pthread_t t;
        pthread_create(&t, NULL, &Monitor::handle_message, &phi);
        getchar();
        while(true) {
            phi.eat();
        }
    }

private:
    Config conf;
    zmq::context_t ctx_;
    zmq::socket_t client_socket_;
    int PID;
};


int main (int argc, char** argv)
{
    if(argc != 2) {
        cout<<"usage ./Philosopher <PID NUMBER from 0>"<<endl;
        return 0;
    }
    client_task ct1(atoi(argv[1]));
    ct1.start();
    getchar();
    return 0;
}