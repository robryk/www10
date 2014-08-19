#include <thread>
#include <mutex>
#include <iostream>
#include "bench.h"
#include <cassert>
#include <semaphore.h>

using namespace std;

sem_t sema;

void init() {
	assert(sem_init(&sema, 0, 1) == 0);
}

void lock() {
	sem_wait(&sema);
}

void unlock() {
	sem_post(&sema);
}

std::atomic<int> tester;

void bench_thread(B* b) {
	while (!b->should_stop()) {
		lock();
		assert(tester.exchange(1) == 0);
		tester.store(0);
		unlock();
		b->inc();
	}
}

void bench_mutex(int n_threads) {
	thread thr[n_threads];
	B b;
	init();
	for(int i=0;i<n_threads;i++)
		thr[i] = thread(&bench_thread, &b);
	for(int i=0;i<n_threads;i++)
		thr[i].join();
	cout << n_threads << " threads - " << b.report() << " ops/us\n";
}

int main() {
	bench_mutex(1);
	bench_mutex(2);
	bench_mutex(4);
	return 0;
}

