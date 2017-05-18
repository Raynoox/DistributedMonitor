#include <thread>
#include "producer.h"
#include "consumer.h"
#include "configuration.h"

#include <string>


class client_task {
public:
    client_task(int pid,string arg_type)
            : ctx_(1),
              client_socket_(ctx_, ZMQ_DEALER),
              conf() {
        PID = pid;
        type = arg_type;
    }

    void start() {

        int i = 10;
        const int BUFFER_NUMBER = 1;//id of buffer to monitor
        Spinbuf buffer = Spinbuf(ctx_, ARRAY_SIZE, PID, conf.PROC_NUM, BUFFER_NUMBER);
        if(type.compare("C") == 0) {
            printf("CONSUMER INITIALIZATION\n");
            Consumer c = Consumer(ctx_,ARRAY_SIZE,PID, conf.PROC_NUM,buffer);
           // c.printMessage("COMPLETE");
            pthread_t t;
            pthread_create(&t, NULL, &Monitor::handle_message, c.getSpinbuf());
            cout<<"PROCESS IS HANDLING MESSAGES"<<endl<<"PRESS <ENTER> TO START WORK"<<endl;
            getchar();
            int pos = 0;
            while(true) {
               // sleep(1);
                c.consume((pos++)%ARRAY_SIZE);
            }
        } else if (type.compare("P") == 0) {
            printf("PRODUCER INITIALIZATION\n");
            Producer p = Producer(ctx_,ARRAY_SIZE,PID, conf.PROC_NUM,buffer);
           // p.printMessage("COMPLETE");
            pthread_t t;
            pthread_create(&t, NULL, &Monitor::handle_message, p.getSpinbuf());
            cout<<"PROCESS IS HANDLING MESSAGES"<<endl<<"PRESS <ENTER> TO START WORK"<<endl;
            getchar();
            int pos = 0;
            while(true) {
             //   sleep(1);
                p.produce((pos++)%ARRAY_SIZE, 15);
            }
        } else {
            cout<<"Wrong type! -> "<<type<<endl;
        }
    }

private:
    Config conf;
    zmq::context_t ctx_;
    zmq::socket_t client_socket_;
    int PID;
    string type;
};
int main (int argc, char** argv)
{
    if(argc != 3) {
        cout<<"usage ./Client <PID NUMBER from 0> <[C]onsumer/[P]roducer>"<<endl;
        return 0;
    }
    string str(argv[2], argv[2] + 1);
    cout<<str<<endl;
    client_task ct1(atoi(argv[1]),str);
    ct1.start();
    //thread t1(bind(&client_task::start, &ct1));

    //t1.detach();
    getchar();
    return 0;
}