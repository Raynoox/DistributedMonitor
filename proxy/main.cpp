#include <iostream>
//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include <vector>
#include <thread>
#include "../client/monitor.h"
#include "../client/configuration.h"
//  This is our client task class.
//  It connects to the server, and then sends a request once per second
//  It collects responses as they arrive, and it prints them out. We will
//  run several client tasks in parallel, each with a different random ID.
//  Attention2! -- this random work well only on linux.
using namespace std;

//  Each worker task works on one request at a time and sends a random number
//  of replies back, with random delays between replies:

class server_worker {
public:
    server_worker(zmq::context_t &ctx, int sock_type)
            : ctx_(ctx),
              worker_(ctx_, sock_type),
              pub_(ctx_,ZMQ_PUB)
    {}

    void work() {
        worker_.connect("inproc://backend");
        pub_.bind("tcp://*:5571");

        try {
            while (true) {
                zmq::message_t identity;
                zmq::message_t msg;
                zmq::message_t copied_id;
                zmq::message_t copied_msg;
                int counter = 0;
            while(counter < 10000000) {
                cout<<"WAITING"<<endl;
                worker_.recv(&identity);
                worker_.recv(&msg);
                struct Message *msg2 = (struct Message*)(msg.data());
                std::cout<<msg2->pid<<" | PROC_TIME: "<<msg2->time<<"RECEIVED: " << MSG_TStrings[msg2->type] << std::endl;
                if(msg2->type == ACK) {
                    std::cout<<"TO "<<msg2->destpid<<endl;
                }
                pub_.send(msg);
            }


//                int replies = within(5);
                int replies = 1;
                for (int reply = 0; reply < replies; ++reply) {
                    s_sleep(within(1000) + 1);
                    copied_id.copy(&identity);
                    copied_msg.copy(&msg);
                    printf ("identity %d bytes: %s x\n", identity.size(), identity.data());
                    struct Message *msg2 = (struct Message*)(msg.data());
                    std::cout<<msg2->pid<<" | PROC_TIME: "<<msg2->time<<"RECEIVED: " << MSG_TStrings[msg2->type] << std::endl;
                    string sx(msg2->dataString);
                    std::istringstream iss(sx);
                    boost::archive::text_iarchive oa(iss);
                    DataSerial ds = DataSerial(100);
                    oa >> ds;
                    ds.print();
                    worker_.send(copied_id, ZMQ_SNDMORE);
                    worker_.send(copied_msg);
                }
            }
        }
        catch (std::exception &e) {}
    }
    void printArray(int t[]) {
        for(int i=0;i<100;i++) {
            cout<<t[i]<<" ";
        }
        cout<<endl;
    }
private:
    zmq::context_t &ctx_;
    zmq::socket_t worker_;
    zmq::socket_t pub_;
};

//  This is our server task.
//  It uses the multithreaded server model to deal requests out to a pool
//  of workers and route replies back to clients. One worker can handle
//  one request at a time but one client can talk to multiple workers at
//  once.
class server_task {
public:
    server_task()
            : ctx_(1),
              frontend_(ctx_, ZMQ_ROUTER),
              backend_(ctx_, ZMQ_DEALER),
              proxycon_(ctx_,ZMQ_PUB),
              sub_(ctx_, ZMQ_XSUB),
              pub_(ctx_, ZMQ_PUB),
              conf()
    {}

    enum { kMaxThread = 1 };

    void run() {
        frontend_.bind("tcp://*:5570");
        backend_.bind("inproc://backend");
        pub_.bind("tcp://*:5571");

//        proxycon_.bind("tcp://*:5571"); //tutaj moga subowac inne proxy

//        s_sendmore (proxycon_, "BROAD");

       /* vector<server_worker*> worker;
        vector<thread*> worker_thread;
        for (int i = 0; i < kMaxThread; ++i) {
            //worker.push_back(new server_worker(ctx_, ZMQ_DEALER));

            worker.push_back(new server_worker(ctx_,ZMQ_DEALER));
            worker_thread.push_back(new std::thread(std::bind(&server_worker::work, worker.back())));

            worker_thread.back()->detach();
        }*/
        cout<<"print conf"<<endl;

//        try {
            for(std::list<string>::iterator list_iter = conf.addresses.begin();
                list_iter != conf.addresses.end(); list_iter++)
            {

                std::ostringstream ss;
                ss << PROTOCOL <<"://"<<*list_iter<<":"<<LOCAL_PUB_PORT;
                cout<<ss.str()<<endl;
                sub_.connect(ss.str());
                cout<<"connected"<<endl;
//                ss <<
//                   sock_sub_.connect(LOCAL_PUB_URL);
//                sock_sub_.setsockopt(ZMQ_SUBSCRIBE, "", 0);
            }
            zmq::pollitem_t items [] = {
                    { sub_, 0, ZMQ_POLLIN, 0 },
                    { frontend_, 0, ZMQ_POLLIN, 0 }
            };
            printf("SOCKETS INITIALIZED\n");
            cout<<"waiting"<<endl;
            while (1) {
                zmq::message_t message;
                zmq::message_t identity;
                zmq::poll (&items[0], 2, -1);
                //cout<<items[0].revents<<endl;
                //cout<<ZMQ_POLLIN<<endl;
                //cout<<items[1].revents<<endl;
                if (items[0].revents & ZMQ_POLLIN) {
                    cout<<"SUB";
                    sub_.recv(&message);
                    struct Message *msg2 = (struct Message*)(message.data());
                    process_message(msg2);
                    pub_.send(message);

                }
                if (items[1].revents & ZMQ_POLLIN) {
                    frontend_.recv(&identity);
                    frontend_.recv(&message);
                    struct Message *msg2 = (struct Message*)(message.data());
                    process_message(msg2);
                    pub_.send(message);
                }
            }
//            zmq::proxy(frontend_, backend_, nullptr);


//            zmq::proxy(frontend_,proxycon_, nullptr);
//        }
//        catch (exception &e) {}

//        for (int i = 0; i < kMaxThread; ++i) {
            //delete worker.at(i);
            //delete worker_thread.at(i);
//        }
    }
    void process_message(Message *msg2){
        std::cout<<msg2->pid<<" | PROC_TIME: "<<msg2->time<<" RECEIVED: " << MSG_TStrings[msg2->type] << std::endl;
        if(msg2->type == ACK) {
            std::cout<<"TO "<<msg2->destpid<<endl;
        }
    }

private:
    zmq::context_t ctx_;
    zmq::socket_t frontend_;
    zmq::socket_t backend_;
    zmq::socket_t proxycon_;
    zmq::socket_t pub_;
    zmq::socket_t sub_;

    Config conf;
};

//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.
int main (void)
{
    server_task st;

    thread t4(bind(&server_task::run, &st));

    t4.detach();

    getchar();
    return 0;
}