#include "signalhandling.hpp"

#include <stdexcept>
#include <string>
#include <cstdint>
#include <climits>


using std::vector;

//put the instances of the static vars here:
volatile std::atomic_int signalhandling::signal = ATOMIC_VAR_INIT(0);
struct sigaction signalhandling::handler_struct;

//declare the actual signalhandler:
extern "C" void signal_handler(int signal);


void signalhandling::init(vector<int> sigs){
	signal.store(0);
	handler_struct.sa_handler = signal_handler;
	for(auto sig: sigs){
		sigaction(sig, &handler_struct, NULL);
	}
}


int signalhandling::get_last_signal(){
	return signal.load();
}

int signalhandling::reset(){
	return signal.fetch_and(0);
}

void signalhandling::check(){
	using std::to_string;
	auto sig = signal.load();
	if(sig){
		throw signal_exception("caught signal #" + to_string(sig), sig);
	}
}

extern "C"{
void signal_handler(int signal){
	signalhandling::signal.store(signal);
}
}

signal_exception::signal_exception(const std::string& what_arg, int sig_num):
	std::runtime_error(what_arg),
	_sig_num(sig_num){}

int signal_exception::sig_num(){
	return _sig_num;
}


