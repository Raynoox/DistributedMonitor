# DistributedMonitor
distributed monitor
cmake 3.3.2
https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz
https://github.com/zeromq/cppzmq.git
https://github.com/zeromq/zmqpp.git
sudo zypper install boost_1_61-devel

COMPILATION
cmake . //in root folder
make

./DistributedMonitor on EACH host


example programs:
./Client <unique_id from 0 to {number of processes}> [P]roducer [C]onsumer

or

./Philio <unique_id from 0 to {number of processes}>


how to use for developers

config.txt -> on each HOST
proc_num -> required! GLOBAL number of processes that will be used
proxy=xxx.xxx.xxx.xxx ip of proxy (can declare multiple in new lines)
proxy=yyy.yyy.yyy.yyy (e.g.)
proxy=zzz.zzz.zzz.zzz (e.g.)
/\ declares that LOCAL host has to connect to sockets on hosts with ip (x..., y..., z...)

To broadcast messages, each host needs to have ./DistributedMonitor launched in background which passes messages from and to local processes

!!!IMPORTANT!!! before programs start asking to synchronize, all processes must first "register" in theirs DistributedMonitors (so they can answer for requests, otherwise program will wait endlessly) (in example programs, you need to run programs on all hosts, and if all programs are running, hit enter to start calculations) !!!IMPORTANT!!!
program can "register" with pthread_create(pthread_t, NULL, &Monitor::handle_message, Monitor*);

Implement your program using Monitor operations:
lock() - asks others for access, and then block others
unlock() - unlocks mutex
wait(conditional_id) - forces thread to wait for signal(conditional_id) from other thread
signal(conditional_id) - wakes up other thread which wait on conditional_id

class Lockwrapper -> works as SYNCHRONIZED in JAVA (can conditionally stop with wait(ID) method)

example:
Buffer : Monitor {
	void put(int x) {
		Lockwrapper temp(this);
		while(dataArray->getValue()[0]==0) {
			wait(COND_ID,ID);
		}
		dataArray->getValue()[0] = x;
		signal(COND_ID2,ID);
		unlock(ID);
	}
	int get() {
		Lockwrapper temp(this);
		while(dataArray->getValue()[0]!=0) {
			wait(COND_ID2,ID);
		}
		int result = dataArray->getValue()[0];
		dataArray->getValue()[0] = 0;
		signal(COND_ID,ID);
		return result;
	}
}
## dataArray is data sent in this monitor (array of integers)

to support other types of data in monitor, you need to declare your own DataSerial class in monitor.h and implement serialize method (boost 1.61)

