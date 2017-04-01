//
// Created by root on 4/1/17.
//

#include "client.h"
#include <thread>
#include "monitor.h"
#include <zmq.hpp>
#include <zhelpers.hpp>
using namespace std;
class client_task {
public:
    client_task()
            : ctx_(1),
              client_socket_(ctx_, ZMQ_DEALER)
    {}

    void start() {
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
                    // 10 milliseconds
                    zmq::poll(items, 1, 10);
                    if (items[0].revents & ZMQ_POLLIN) {
                        printf("\n%s ", identity);
                        s_dump(client_socket_);
                    }
                }
                char request_string[16] = {};
                sprintf(request_string, "request #%d", ++request_nbr);
                string message = request_string;

                GenericMessage<int> msg(++request_nbr, request_nbr);
                zmq::message_t msgToSend(sizeof(msg));

                memcpy ( msgToSend.data(), ((GenericMessage<int>*)&msg), sizeof(*  ((GenericMessage<int>*)&msg)));

                //mOutbHandlerSocket.send(msgToSend);
                string res = msg.toString();
                printf("SENT request -> id: %s\n", res.data());
                //std::cout << "SENT2 request: [" << msg.toString() << "]" << std::endl;

                client_socket_.send(msgToSend);
                //zmq::message_t msgtype;
                //msgtype = request_nbr;
                //client_socket_.send();
            }
        }
        catch (std::exception &e) {}
    }

private:
    zmq::context_t ctx_;
    zmq::socket_t client_socket_;
};
int main (void)
{
    client_task ct1;
    client_task ct2;
    client_task ct3;

    thread t1(bind(&client_task::start, &ct1));
    thread t2(bind(&client_task::start, &ct2));
    thread t3(bind(&client_task::start, &ct3));

    t1.detach();
    t2.detach();
    t3.detach();
    getchar();
    return 0;
}