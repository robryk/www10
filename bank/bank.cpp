#include <thread>
#include <mutex>
#include <iostream>
#include "bench.h"
#include <cassert>
#include <semaphore.h>

using namespace std;

constexpr int n_accounts = 1000;

sem_t sema;
int values[n_accounts];

void init() {
	assert(sem_init(&sema, 0, 1) == 0);
	for(int i=0;i<n_accounts;i++)
		values[i] = 0;
}

void destroy() {
	assert(sem_destroy(&sema) == 0);
}

void add(int to, int amount) {
	assert(amount >= 0);
	sem_wait(&sema);
	values[to] += amount;
	sem_post(&sema);
}

bool txfr(int from, int to, int amount) {
	bool ret = false;
	assert(amount >= 0);
	sem_wait(&sema);
	if (values[from] >= amount) {
		values[from] -= amount;
		values[to] += amount;
		ret = true;
	}
	sem_post(&sema);
	return ret;
}

int read(int accnt) {
	sem_wait(&sema);
	int r = values[accnt];
	sem_post(&sema);
	return r;
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
	return 0;
}


