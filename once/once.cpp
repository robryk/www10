#include <thread>
#include <mutex>
#include <iostream>
#include "bench.h"

using namespace std;

mutex mu;
bool done;

void init_fn() {
	printf("Init\n");
}

void once() {
	mu.lock();
	if (!done) {
		init_fn();
		done = true;
	}
	mu.unlock();
}

void once_reset() {
	done = false;
}

void bench_thread(B *b) {
	while (!b->should_stop()) {
		once();
		b->inc();
	}
}

void once_bench(int n_threads) {
	B b;
	thread thrs[n_threads];
	once_reset();
	for(int i=0;i<n_threads;i++)
		thrs[i] = thread(&bench_thread, &b);
	for(int i=0;i<n_threads;i++)
		thrs[i].join();
	cout << n_threads << " threads - " << b.report() << " ops/us\n";
}

int main() {
	once_bench(1);
	once_bench(2);
	once_bench(4);
	return 0;
}



