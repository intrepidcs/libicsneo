#ifndef __PERIODIC_H__
#define __PERIODIC_H__

#ifdef __cplusplus

#include <condition_variable>
#include <mutex>
#include <functional>
#include <chrono>
#include <thread>

namespace icsneo {

class Periodic {
public:
	using Callback = std::function<bool(void)>;
	Periodic(Callback&& callback, const std::chrono::milliseconds& period) :
		thread(&Periodic::loop, this, std::move(callback), period)
	{}
	~Periodic() {
		{
			std::scoped_lock lk(mutex);
			stop = true;
		}
		cv.notify_all();
		thread.join();
	}
private:
	void loop(Callback&& callback, const std::chrono::milliseconds& period) {
		while (true) {
			{
				std::unique_lock lk(mutex);
				cv.wait_for(lk, period, [&]{ return stop; });
				if(stop) {
					break;
				}
			}
			if (!callback()) {
				break;
			}
		}
	}
	bool stop = false;
	std::condition_variable cv;
	std::mutex mutex;
	std::thread thread;
};

} // icsneo

#endif // __cplusplus

#endif // __PERIODIC_H__
