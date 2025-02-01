#ifndef FIXAWAY_COMMON_HPP_
#define FIXAWAY_COMMON_HPP_

#include <string>
#include <cstring>
#include <type_traits>
#include <cassert>
#include <numeric>
#include <ctime>
#include <random>

namespace fixaway {
    /*
    inline int64_t system_timestamp() {
        timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);
        return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    }
    */
    inline double random_number(double min, double max)
    {
        std::random_device rd;
        std::default_random_engine generator(rd());
        std::uniform_real_distribution<double> distribution(min, max);

        return distribution(generator);
    }
    inline int64_t random_number(int min, int max)
    {
        std::random_device rd;
        std::default_random_engine generator(rd());
        std::uniform_int_distribution<int64_t> distribution(min, max);

        return distribution(generator);
    }
    /*
    inline std::string generateRandomString(int length)
    {
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        std::string randomString;
        randomString.reserve(length);

        for (int i = 0; i < length; ++i) {
            randomString += characters[std::rand() % characters.size()];
        }
        return randomString;
    }
    */
}

#endif
