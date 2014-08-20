#include <thread>
#include <mutex>
#include <iostream>
#include "bench.h"
#include <cassert>
#include <semaphore.h>

using namespace std;

constexpr int n_accounts = 1000;

sem_t semas[n_accounts];
atomic<int> values[n_accounts];

void init() {
	for(int i=0;i<n_accounts;i++) {
		assert(sem_init(&semas[i], 0, 1) == 0);
		values[i] = 0;
	}
}

void destroy() {
	for(int i=0;i<n_accounts;i++) {
		assert(sem_destroy(&semas[i]) == 0);
	}
}

void add(int to, int amount) {
	assert(amount >= 0);
	values[to].fetch_add(amount);
}

bool txfr(int from, int to, int amount) {
	sem_wait(&semas[from]);
	if (values[from].load() < amount) {
		sem_post(&semas[from]);
		return false;
	}
	values[from].fetch_add(-amount);
	values[to].fetch_add(amount);
	sem_post(&semas[from]);
	return true;
}

int read(int accnt) {
	return values[accnt].load();
}

// IMPLEMENTACJA POWYZEJ
// TESTY PONIZEJ

void random_thread(B* b) {
	while (!b->should_stop()) {
		txfr(rand() % n_accounts, rand() % n_accounts, rand() % 1000);
		b->inc();
	}
}

void bench_random(int n_threads) {
	thread thr[n_threads];
	B b;
	init();
	int total_before = 0;
	for(int i=0;i<n_accounts;i++) {
		add(i, 2000);
		total_before += 2000;
	}
	for(int i=0;i<n_threads;i++)
		thr[i] = thread(&random_thread, &b);
	for(int i=0;i<n_threads;i++)
		thr[i].join();
	int total_after = 0;
	for(int i=0;i<n_accounts;i++) {
		total_after += read(i);
	}
	destroy();
	assert(total_before == total_after);
	cout << n_threads << " threads, random - " << b.report() << " ops/us\n";
}

int main() {
	bench_random(1);
	bench_random(2);
	bench_random(4);
	return 0;
}


