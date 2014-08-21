#include <thread>
#include <mutex>
#include <iostream>
#include "bench.h"
#include <cassert>
#include <semaphore.h>

using namespace std;

constexpr int writer = 1<<20;

sem_t w_mutex;
sem_t r_sema;
sem_t w_sema;
atomic<int> count;
atomic<int> readers_remaining;

void init() {
	assert(sem_init(&w_mutex, 0, 1) == 0);
	assert(sem_init(&r_sema, 0, 0) == 0);
	assert(sem_init(&w_sema, 0, 0) == 0);
	count = 0;
	readers_remaining = 0;
}

void destroy() {
	assert(sem_destroy(&w_mutex) == 0);
	assert(sem_destroy(&r_sema) == 0);
	assert(sem_destroy(&w_sema) == 0);
}

void lock() {
	sem_wait(&w_mutex);
	int readers_inside = count.fetch_add(writer);
	if (readers_remaining.fetch_add(readers_inside) != -readers_inside)
		sem_wait(&w_sema);
}

void unlock() {
	int readers_waiting = count.fetch_add(-writer) - writer;
	for(int i=0;i<readers_waiting;i++)
		sem_post(&r_sema);
	sem_post(&w_mutex);
}

void rlock() {
	int count_before = count.fetch_add(1);
	if (count_before >= writer)
		sem_wait(&r_sema);
}

void runlock() {
	if (count.fetch_add(-1) >= writer) {
		if (readers_remaining.fetch_add(-1) == 1)
			sem_post(&w_sema);
	}
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
			runlock();
		}
	}
}

void bench_run(int n_threads, int read_count) {
	thread thr[n_threads];
	B b;
	init();
	tester = 0;
	for(int i=0;i<n_threads;i++)
		thr[i] = thread(&tester_thread, &b, read_count);
	for(int i=0;i<n_threads;i++)
		thr[i].join();
	destroy();
	cout << n_threads << " threads, " << read_count << " reads/write - " << b.report() << " ops/us\n";
}

int main() {
	bench_run(1, 1);
	bench_run(2, 0);
	bench_run(2, 1);
	bench_run(2, 5);
	bench_run(4, 1);
	bench_run(4, 5);
	return 0;
}


