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

void rlock() {
	sem_wait(&sema);
}

void runlock() {
	sem_post(&sema);
}

atomic<int> tester;

void tester_thread(B* b, int read_count) {
	while (!b->should_stop()) {
		lock();
		assert(tester.fetch_add(1<<20) == 0);
		b->inc();
		assert(tester.fetch_add(-(1<<20)) == (1<<20));
		unlock();
		for(int i=0;i<read_count;i++) {
			rlock();
			assert(tester.fetch_add(1) < (1<<20));
			b->inc();
			assert(tester.fetch_add(-1) < (1<<20));
			unlock();
		}
	}
}

void bench_run(int n_threads, int read_count) {
	thread thr[n_threads];
	B b;
	init();
	for(int i=0;i<n_threads;i++)
		thr[i] = thread(&tester_thread, &b, read_count);
	for(int i=0;i<n_threads;i++)
		thr[i].join();
	cout << n_threads << " threads, " << read_count << " reads/write - " << b.report() << " ops/us\n";
}

int main() {
	bench_run(1, 1);
	bench_run(2, 1);
	bench_run(2, 5);
	bench_run(4, 1);
	bench_run(4, 5);
	return 0;
}


