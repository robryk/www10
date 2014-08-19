#ifndef _BENCH_H
#define _BENCH_H

#include <unistd.h>
#include <atomic>
#include <thread>

void sleeper_thread(std::atomic<bool>* stop) {
	sleep(1);
	stop->store(true);
}

struct B {
	std::atomic<bool> stop;
	std::atomic<int> count;
	std::thread sleeper;

	bool should_stop() { return stop.load(); }
	void inc(int delta = 1) { count.fetch_add(delta); }
	double report() {
		sleeper.join();
		return double(count.load()) / 1e6;
	}
	B() : stop(false), count(0) {
		sleeper = std::thread(&sleeper_thread, &stop);
	}
};

#endif // _BENCH_H
