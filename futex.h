#include <linux/futex.h>
#include <sys/time.h>
#include <cassert>
#include <sys/syscall.h>

static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3)
{
	return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

struct futex {
	std::atomic<int> value;

	// Jeśli wartość jest równa expected_value, zasypia. Po obudzeniu zwraca true.
	// W przeciwnym razie zwraca false.
	bool wait(int expected_value) {
		int ret = sys_futex((int*)&value, FUTEX_WAIT, expected_value, NULL, NULL, 0);
		if (ret == 0)
			return true;
		else {
			assert(errno == EWOULDBLOCK);
			return false;
		}
	}

	void wake(int how_many) {
		int ret = sys_futex((int*)&value, FUTEX_WAKE, how_many, NULL, NULL, 0);
		assert(ret >= 0);
	}

	void wake_all() {
		wake(1<<16);
	}
};

