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

#include <chrono>

#include <ert/tracing/Logger.hpp>

#include <ert/queuedispatcher/QueueDispatcher.hpp>

namespace ert
{
namespace queuedispatcher
{


QueueDispatcher::QueueDispatcher(std::string name, int threads, int maxThreads) :
    name_{std::move(name)}, threads_((threads > 0) ? threads:1), max_threads_(maxThreads)
{
    threads = ((threads > 0) ? threads:1); // protection for bad input
    max_threads_ = (maxThreads > threads) ? maxThreads:threads;

    LOGINFORMATIONAL(
        std::string msg = ert::tracing::Logger::asString("Creating dispatch queue '%s' with '%zu' ", name_.c_str(), threads);
        msg += (max_threads_ == threads) ? "fixed threads.":ert::tracing::Logger::asString("threads and a maximum of '%zu'.", max_threads_);
        ert::tracing::Logger::informational(msg, ERT_FILE_LOCATION));

    for (size_t i = 0; i < threads_.size(); i++)
    {
        threads_[i] = std::thread(&QueueDispatcher::dispatch_thread_handler, this);
    }
}

QueueDispatcher::~QueueDispatcher()
{
    LOGINFORMATIONAL(ert::tracing::Logger::informational(
                         ert::tracing::Logger::asString("Destroying dispatch threads ..."), ERT_FILE_LOCATION));

    // Signal to dispatch threads that it's time to wrap up
    std::unique_lock<std::mutex> lock(lock_);
    quit_ = true;
    lock.unlock();
    cv_.notify_all();

    // Wait for threads to finish before we exit
    for (size_t i = 0; i < threads_.size(); i++)
    {
        if (threads_[i].joinable())
        {
            LOGINFORMATIONAL(ert::tracing::Logger::informational(
                                 ert::tracing::Logger::asString("Joining thread %zu until completion", i), ERT_FILE_LOCATION));
            threads_[i].join();
        }
    }
}

void QueueDispatcher::create_thread(void)
{
    threads_.push_back(std::thread(&QueueDispatcher::dispatch_thread_handler, this));
}

void QueueDispatcher::dispatch_thread_handler(void)
{
    std::unique_lock<std::mutex> lock(lock_);

    do
    {
        //Wait until we have data or a quit signal
        cv_.wait(lock, [this]
        {
            return (q_.size() || quit_);
        });

        //after wait, we own the lock
        if (!quit_ && q_.size())
        {
            auto st = std::move(q_.front());
            q_.pop();

            busy_threads_++;

            //unlock now that we're done messing with the queue
            lock.unlock();

            // No idle consumers, and no margin to create new consumer threads:
            bool busyConsumers = (busy_threads_.load() == threads_.size() && threads_.size() == max_threads_);

            auto begin = std::chrono::steady_clock::now();
            st->process(busyConsumers, q_.size());
            auto end = std::chrono::steady_clock::now();

            // Stream could store statistics and take it together with busyConsumers and queue size to implement
            // congestion control algorithms
            st->processLapse(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());

            lock.lock();
            busy_threads_--;
        }
    }
    while (!quit_);
}

void QueueDispatcher::dispatch(std::shared_ptr<StreamIf> st)
{
    if (busy_threads_.load() == threads_.size() && threads_.size() < max_threads_)
    {
        create_thread();
    }

    std::unique_lock<std::mutex> lock(lock_);
    //    q_.push(std::move(st));
    //
    //    // Manual unlocking is done before notifying, to avoid waking up
    //    // the waiting thread only to block again (see notify_one for details)
    //    lock.unlock();
    //    cv_.notify_one();

    q_.push(st);
    lock.unlock();
    cv_.notify_one();
}


}
}
