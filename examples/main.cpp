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

// C
#include <libgen.h> // basename
#include <signal.h>

// Standard
#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <chrono>
//#include <unistd.h>
#include <thread>
#include <atomic>

#include <ert/tracing/Logger.hpp>

#include <ert/queuedispatcher/QueueDispatcher.hpp>
#include <ert/queuedispatcher/StreamIf.hpp>

// We use low rates (1 per second) to ease library understanding:
#define PRODUCTION_RPS 1 // events per second
#define PRODUCTION_RPS_SLEEP_MS int(1000/PRODUCTION_RPS) // delay to simulate production rate
#define CONSUMPTION_CAPACITY 0.1 // i.e. 0.1 means 10 times slower than production
#define CONSUMPTION_RPS_SLEEP_MS int(PRODUCTION_RPS_SLEEP_MS/CONSUMPTION_CAPACITY) // delay to simulate consumption rate

const char* progname;
std::atomic<int> sequence;


void sighndl(int signal)
{
    LOGWARNING(ert::tracing::Logger::warning(ert::tracing::Logger::asString("Signal received: %d", signal), ERT_FILE_LOCATION));
    switch (signal) {
    case SIGTERM:
    case SIGINT:
        exit(1);
        break;
    }
}

class Stream : public ert::queuedispatcher::StreamIf {
    std::string data_;
    bool congestion_control_;

public:
    Stream(const std::string &data, bool congestionControl) : data_(data), congestion_control_(congestionControl) {;}

    // Process reception
    void process(bool congestion, int queueSize) {
        std::string ifcongestion = (congestion ? "":"");
        std::cout << "[process] Congestion detected: " << (congestion ? "true":"false") << " | Congestion control: " << (congestion_control_ ? "true":"false") << " | Queue size: " << queueSize << std::endl;
        if (congestion && congestion_control_) {
            std::cout << "[process] --- Ignore context for " << data_ << std::endl;
        }
        else {
            std::cout << "[process] --- Data processed: " << data_ << (congestion ? " | Queue will grow as no congestion control is enabled":"") << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(CONSUMPTION_RPS_SLEEP_MS));
        }
    };
};

int main(int argc, char* argv[]) {

    progname = basename(argv[0]);
    ert::tracing::Logger::initialize(progname);
    ert::tracing::Logger::verbose();

    std::string opt;

    std::cout << std::endl;
    std::cout << "Number of queue initial threads [5]:" << std::endl;
    getline(std::cin, opt);
    int consumers = (opt.empty() ? 5:atoi(opt.c_str()));

    std::cout << "Number of queue max threads [" << consumers << "]:" << std::endl;
    getline(std::cin, opt);
    int max_consumers = (opt.empty() ? consumers:atoi(opt.c_str()));

    std::cout << "Congestion control (ignore process when threads are busy) [0: disable]:" << std::endl;
    getline(std::cin, opt);
    bool congestionControl = (opt.empty() ? false:atoi(opt.c_str()));

    ert::queuedispatcher::QueueDispatcher queue("MyQueue", consumers, max_consumers);

    // Capture TERM/INT signals for graceful exit:
    signal(SIGTERM, sighndl);
    signal(SIGINT, sighndl);

    while(true) {
        std::cout << std::endl;
        std::cout << "[iteration " << sequence.load() << "] Consumers threads: " << queue.threads() << " | Consumers max threads: " << queue.maxThreads() << " | Busy threads: " << queue.busyThreads() << std::endl;

        auto stream = std::make_shared<Stream>(std::to_string(sequence.load()), congestionControl);
        sequence++;

        queue.dispatch(stream);
        std::this_thread::sleep_for(std::chrono::milliseconds(PRODUCTION_RPS_SLEEP_MS));
    }
}
