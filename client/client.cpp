#include <thread>
#include "consumer.h"
#include "producer.h"
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
        if(type.compare("C") == 0) {
            printf("CONSUMER INITIALIZATION\n");
            Consumer c = Consumer(ctx_,ARRAY_SIZE,PID, conf.PROC_NUM);
            c.printMessage("COMPLETE");
            pthread_t t;
            pthread_create(&t, NULL, &Monitor::handle_message, &c);
            sleep(5);
            while(true) {
                usleep(1000);
                c.consume();
            }
        } else if (type.compare("P") == 0) {
            printf("PRODUCER INITIALIZATION\n");
            Producer p = Producer(ctx_,ARRAY_SIZE,PID, conf.PROC_NUM);
            p.printMessage("COMPLETE");
            pthread_t t;
            pthread_create(&t, NULL, &Monitor::handle_message, &p);
            sleep(5);
            while(true) {
                usleep(1000);
                p.produce();
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

    thread t1(bind(&client_task::start, &ct1));

    t1.detach();
    getchar();
    return 0;
}