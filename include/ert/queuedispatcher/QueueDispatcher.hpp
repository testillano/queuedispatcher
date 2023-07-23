/*
 _____________________________________________________________________________________________________
|             _                                         _ _                 _       _                 |
|            | |                                       | (_)               | |     | |                |
|    ___ _ __| |_   __   __ _ _   _  ___ _   _  ___  __| |_ ___ _ __   __ _| |_ ___| |__   ___ _ __   |
|   / _ \ '__| __| |__| / _` | | | |/ _ \ | | |/ _ \/ _` | / __| '_ \ / _` | __/ __| '_ \ / _ \ '__|  |
|  |  __/ |  | |_      | (_| | |_| |  __/ |_| |  __/ (_| | \__ \ |_) | (_| | || (__| | | |  __/ |     |
|   \___|_|   \__|      \__, |\__,_|\___|\__,_|\___|\__,_|_|___/ .__/ \__,_|\__\___|_| |_|\___|_|     |
|                          | |                                 | |                                    |
|                          |_|                                 |_|                                    |
|_____________________________________________________________________________________________________|

 QUEUE DISPATCHER LIBRARY C++
 Version 0.0.z
 https://github.com/testillano/queuedispatcher

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2021 Eduardo Ramos

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>

#include <ert/queuedispatcher/StreamIf.hpp>

namespace ert
{
namespace queuedispatcher
{
/**
 * FIFO queue dispatcher
 *
 * FIFO queue of tasks are delivered to a fixed initial pool of threads.
 * Maximum number of threads (greater than initial provided to have sense)
 * can be specified, so the threads pool will grow when all threads available
 * are busy. If there is no more chance to grow (maximum reached, or maximum
 * was not provided and threads pool size (initial fixed size) is exhausted),
 * congestion action can be applied: stream process() passes this as a flag to
 * be considered by the user. If no congestion control is applied, or the
 * consumption capacity is smaller than production one the queue size could
 * grow with the degradation consequence in response times.
 *
 * @see ert::queuedispatcher::StreamIf
 */
class QueueDispatcher
{
public:
    /** Constructor
     *
     * @param name QueueDispatcher name/identifier
     * @param threads number of initial consumer threads. Must be positive number (wrong input will configure 1).
     * @param maxThreads maximum number of consumer threads. By default, it is equal to initial threads (freeze consumers pool size),
     */
    QueueDispatcher(std::string name, int threads, int maxThreads = -1);
    ~QueueDispatcher();

    /** Adds work to the queue */
    void dispatch(std::shared_ptr<StreamIf>);

    // Deleted operations
    QueueDispatcher(const QueueDispatcher& rhs) = delete;
    QueueDispatcher& operator=(const QueueDispatcher& rhs) = delete;
    QueueDispatcher(QueueDispatcher&& rhs) = delete;
    QueueDispatcher& operator=(QueueDispatcher&& rhs) = delete;

    /** Maximum threads reachable */
    int getMaxThreads() const {
        return max_threads_;
    }

    /** Number of busy threads */
    int getBusyThreads() const {
        return busy_threads_.load();
    }

    /** Current available threads */
    int getThreads() const {
        return threads_.size();
    }

    /** Queue size */
    int getSize() const {
        return q_.size();
    }

    /** Queue name */
    const std::string &getName() const {
        return name_;
    }

private:
    std::string name_;
    std::mutex lock_;
    std::vector<std::thread> threads_;
    std::atomic<int> busy_threads_{0};
    size_t max_threads_;
    std::queue<std::shared_ptr<StreamIf>> q_;
    std::condition_variable cv_;
    bool quit_ = false;

    void dispatch_thread_handler(void);
    void create_thread();
};

}
}

