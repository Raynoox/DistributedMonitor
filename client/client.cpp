//
// Created by root on 4/1/17.
//

//#include "client.h"
#include <thread>
#include "monitor.h"
//#include <zmq.hpp>
//#include <zhelpers.hpp>
//#include <pthread.h>
using namespace std;
class client_task {
public:
    client_task(int pid)
            : ctx_(1),
              client_socket_(ctx_, ZMQ_DEALER)
    {PID = pid;}

    void start() {
        printf("MONITOR INITIALIZATION\n");
        Monitor m = Monitor(ctx_,100,PID);
        m.printMessage("COMPLETE");
        pthread_t t;
        m.printArray();
        m.printQueue();
        pthread_create(&t, NULL, &Monitor::handle_message, &m);
        sleep(5);
//        char ch;
//        cout << "Press a key then press enter: "<<endl;;
//        getchar();
//        cout<<"try consume"<<endl;
        int i = 10;
        if(PID == 0 || PID == 1){
            while(i > 0){
                sleep(3);
                m.consume();
            }
        } else {
            while(i > 0){
                sleep(1);
                m.produce();
            }
        }
        sleep(100);


        // generate random identity
        char identity[10] = {};
        sprintf(identity, "%04X-%04X", within(0x10000), within(0x10000));
        printf("%s\n", identity);
        client_socket_.setsockopt(ZMQ_IDENTITY, identity, strlen(identity));
        client_socket_.connect("tcp://localhost:5570");

        zmq::pollitem_t items[] = {client_socket_, 0, ZMQ_POLLIN, 0};
        int request_nbr = 0;
        try {
            while (true) {
                for (int i = 0; i < 100; ++i) {
                    zmq::poll(items, 1, 10);
                    if (items[0].revents & ZMQ_POLLIN) {
                        printf("\n%s ", identity);
                        s_dump(client_socket_);
                    }
                }
                char request_string[16] = {};
                sprintf(request_string, "request #%d", ++request_nbr);
                string message = request_string;
            }
        }
        catch (std::exception &e) {}
    }

private:
    zmq::context_t ctx_;
    zmq::socket_t client_socket_;
    int PID;
};
int main (int argc, char** argv)
{
    if(argc != 2) {
        cout<<"usage ./Client <PID NUMBER>"<<endl;
        return 0;
    }
    client_task ct1(atoi(argv[1]));

    thread t1(bind(&client_task::start, &ct1));

    t1.detach();
    getchar();
    return 0;
}