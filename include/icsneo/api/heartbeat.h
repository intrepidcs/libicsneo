#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#ifdef __cplusplus

#include <thread>
#include <condition_variable>
#include <mutex>

namespace icsneo {

class Device;

class Heartbeat {
public:
    Heartbeat(Device& device);
    ~Heartbeat();

private:
    enum class Mode {
        Passive, // ResetStatus messages arrive without requesting
        Active, // RequestStatusUpdate needs to be sent
    };
    Device& device;
    const Mode mode;
    bool stop = false;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread thread;

    void run();
};

} // icsneo

#endif // __cplusplus

#endif // __HEARTBEAT_H__
