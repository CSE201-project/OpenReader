#pragma once

#include <iostream>
#include <sstream>
#include <mutex>

#define serr SyncStream(std::cerr)
#define sout SyncStream(std::cout)

/**
 * Thread-safe std::ostream class.
 *
 * Usage:
 *    sout << "Hello world!" << std::endl;
 *    serr << "Hello world!" << std::endl;
 */
class SyncStream : public std::ostringstream
{
public:
    SyncStream(std::ostream& os) : os_(os)
    {
        // copyfmt causes odd problems with lost output
        // probably some specific flag
        // copyfmt(os);
        // copy whatever properties are relevant
        imbue(os.getloc());
        precision(os.precision());
        width(os.width());
        setf(std::ios::fixed, std::ios::floatfield);
    }

    ~SyncStream()
    {
        std::lock_guard<std::mutex> guard(_mutex_sync_stream);
        os_ << this->str() << std::endl;
    }

private:
    static std::mutex _mutex_sync_stream;
    std::ostream& os_;
};
