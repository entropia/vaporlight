#ifndef SIGNALHANDLING_HPP
#define SIGNALHANDLING_HPP

#include <vector>
#include <csignal>
#include <stdexcept>
#include <atomic>


// This should work almost everywhere:
#if ATOMIC_INT_LOCK_FREE != 2
	#error "Your platform doesn't support a lockfree atomic<int>."
#endif


extern "C"{
	void signal_handler(int signal);
}

/**
 * This class bundles everything you need for signalhandling.
 * 
 * This class is static.
 * 
 * To use it, call the init method with a vector of signals you want to have managed
 * by this class once (as early as possible) in your programm.
 * 
 * After that you have to call getLastSig() or check() in short intervals and react 
 * to their output in a proper way.
 * 
 */
class signalhandling{
	private:
		//as doxygen on debian doesn't understand ctor=delete, make it private:
		/**
		 * We don't need a constructor, so let's delete it.
		 */
		signalhandling() = delete;
		
	public:
		/**
		 * init function for signalhandling. This has to be called early.
		 * @param sigs vector of the signals, that should be handled (defaults to 
		 *        SIGINT and SIGTERM)
		 */
		static void init(std::vector<int> sigs = {SIGINT, SIGTERM});
		
		/**
		 * query for the last signal; this will return 0 if no signal has been caught.
		 */
		static int get_last_signal();
		
		/**
		 * reset the saved signal to 0. 
		 * @returns the value of signalhandling::signal before setting it to zero.
		 */
		static int reset();
		
		/**
		 * check if signalhandling::signal is set and throw a signal_exception if this 
		 * is the case; otherwise do nothing.
		 * @throws signal_exception if signalhandling::signal is set to another value 
		 *                          than zero.
		 */
		static void check();
		
	private:
		
		/**
		 * the actual handlerfunction, that will set signalhandling::signal to 
		 * the new value
		 */
		friend void signal_handler(int signal);
		
		//attributes:
		
		/**
		 * number of the last recieved signal; init will set this to 0.
		 */
		volatile static std::atomic_int signal;
		
		/**
		 * struct that contains the information, what should be done after recieving
		 * a signal
		 */
		static struct sigaction handler_struct;
};



/**
 * Exception that indicates that a signal got caught.
 */
class signal_exception: public std::runtime_error{
	public:
		/**
		 * The ctor takes the number of the signal as an additional argument
		 * @param what_arg a short message
		 * @param sig_num the number of the signal
		 */
		signal_exception(const std::string& what_arg, int sig_num);
		
		/**
		 * Get the number of the signal.
		 */
		int sig_num();
	private:
		
		/**
		 * the number of the signal.
		 */
		int _sig_num;
};



#endif
