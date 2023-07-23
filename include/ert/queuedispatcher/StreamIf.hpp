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

namespace ert
{
namespace queuedispatcher
{

/**
 * Stream class interface to process queue dispatcher tasks
 *
 * @see https://gist.github.com/tatsuhiro-t/ba3f7d72d037027ae47b
 */
class StreamIf
{

public:
    StreamIf() {};
    StreamIf(const StreamIf&) = delete;
    ~StreamIf() = default;
    StreamIf& operator=(const StreamIf&) = delete;

    /**
     * Consumer gets job from queue.
     *
     * @param busyConsumers Indicates potential congestion situation (simple congestion control may be context ignore)
     * @param queueSize Indicates queue size (useful to improve congestion control algorithms when congestion is detected)
     */
    virtual void process(bool busyConsumers, int queueSize) = 0;

    /**
     * Queue control indicates the last process duration
     *
     * @param nanoseconds
     */
    virtual void processLapse(unsigned long long nanoseconds) {;}
};

}
}

