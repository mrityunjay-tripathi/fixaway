#ifndef FIXAWAY_FIX_DATETIME_HPP_
#define FIXAWAY_FIX_DATETIME_HPP_

#include <ctime>
#include <time.h>
#include <chrono>

namespace fixaway {

enum class clock_precision {
    seconds = 0,
    milliseconds = 1,
    microseconds = 2,
    nanoseconds = 3
};

namespace details {
    template <clock_precision Prec>
    inline size_t strfepoch(char* dest, std::tm* t, int64_t ts);

    template <typename Func>
    inline int64_t strtepoch(const char *src, int size, Func&& f);
}

/**
 * Returns the current epoch, from 01/01/1970.
 */
inline int64_t system_timestamp() {
    timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);
    return (int64_t)ts.tv_sec * 1e+9 + ts.tv_nsec;
}
/**
 * Returns the current unix epoch, from 01/01/1970.
 */
inline int64_t epoch_timestamp() {
    return system_timestamp();
}
/**
 * @description Writes the utc time from epoch timestamp to the given buffer
 * @param dest The address to output buffer.
 * @param ts The epoch timestamp.
 */
template <clock_precision Prec>
inline size_t strfutc(char* dest, int64_t ts) {
    std::time_t t = ts / 1000000000LL;
    std::tm* gm_time = std::gmtime(&t);
    return details::strfepoch<Prec>(dest, gm_time, ts);
}
/**
 * @description Writes the utc time from epoch timestamp,
 *              fetched automatically to the given buffer.
 * @param dest The address to output buffer.
 */
template <clock_precision Prec>
inline size_t strfutc(char* dest) {
    return strfutc<Prec>(dest, epoch_timestamp());
}
/**
 * @description Writes the utc time from epoch timestamp to the given buffer
 * @param dest The address to output buffer.
 * @param ts The epoch timestamp.
 * @param prec The clock precision. Valid values are:
 *             's': seconds, 'm': milliseconds, 'u': microseconds, 'n': nanoseconds
 */
inline size_t strfutc(char* dest, int64_t ts, char prec = 's') {
    if (prec == 's') return strfutc<clock_precision::seconds>(dest, ts);
    else if (prec == 'm') return strfutc<clock_precision::milliseconds>(dest, ts);
    else if (prec == 'u') return strfutc<clock_precision::microseconds>(dest, ts);
    else if (prec == 'n') return strfutc<clock_precision::nanoseconds>(dest, ts);
    else return 0;
}
/**
 * @description Writes the utc time from epoch timestamp to the given buffer
 * @param dest The address to output buffer.
 * @param prec The clock precision. Valid values are:
 *             's': seconds, 'm': milliseconds, 'u': microseconds, 'n': nanoseconds
 */
inline size_t strfutc(char* dest, char prec = 's') {
    return strfutc(dest, epoch_timestamp(), prec);
}
/**
 * @description Converts given string in utc time to epoch.
 * @param src The address of input buffer.
 * @param size The size of input buffer.
 */
inline int64_t strtutc(const char* src, int size) {
    auto f = [](struct tm* t) { return timegm(t); };
    return details::strtepoch(src, size, f);
}
/**
 * @description Writes the local time from epoch timestamp to the given buffer
 * @param dest The address to output buffer.
 * @param ts The epoch timestamp.
 */
template <clock_precision Prec>
inline size_t strflocal(char* dest, int64_t ts) {
    std::time_t t = ts / 1000000000LL;
    std::tm* local_time = std::localtime(&t);
    return details::strfepoch<Prec>(dest, local_time, ts);
}
/**
 * @description Writes the local time from epoch timestamp,
 *              fetched automatically to the given buffer.
 * @param dest The address to output buffer.
 */
template <clock_precision Prec>
inline size_t strflocal(char* dest) {
    return strflocal<Prec>(dest, epoch_timestamp());
}
/**
 * @description Writes the local time from epoch timestamp to the given buffer
 * @param dest The address to output buffer.
 * @param ts The epoch timestamp.
 * @param prec The clock precision. Valid values are:
 *             's': seconds, 'm': milliseconds, 'u': microseconds, 'n': nanoseconds
 */
inline size_t strflocal(char* dest, int64_t ts, char prec = 's') {
    if (prec == 's') return strflocal<clock_precision::seconds>(dest, ts);
    else if (prec == 'm') return strflocal<clock_precision::milliseconds>(dest, ts);
    else if (prec == 'u') return strflocal<clock_precision::microseconds>(dest, ts);
    else if (prec == 'n') return strflocal<clock_precision::nanoseconds>(dest, ts);
    else return 0;
}
/**
 * @description Writes the local time from epoch timestamp to the given buffer
 * @param dest The address to output buffer.
 * @param prec The clock precision. Valid values are:
 *             's': seconds, 'm': milliseconds, 'u': microseconds, 'n': nanoseconds
 */
inline size_t strflocal(char* dest, char prec = 's') {
    return strflocal(dest, epoch_timestamp(), prec);
}
/**
 * @description Converts given string int local time to epoch.
 * @param src The address of input buffer.
 * @param size The size of input buffer.
 */
inline int64_t strtlocal(const char* src, int size) {
    auto f = [](struct tm* t) { return timelocal(t); };
    return details::strtepoch(src, size, f);
}

namespace details {

    template <clock_precision Prec>
    size_t strfepoch(char* dest, std::tm* t, int64_t ts) {
        size_t size = 0;
        int year = t->tm_year + 1900;
        dest[3] = '0' + year % 10; year /= 10;
        dest[2] = '0' + year % 10; year /= 10;
        dest[1] = '0' + year % 10; year /= 10;
        dest[0] = '0' + year % 10; year /= 10;
        int month = t->tm_mon + 1;
        dest[5] = '0' + month % 10; month /= 10;
        dest[4] = '0' + month % 10; month /= 10;
        int day = t->tm_mday;
        dest[7] = '0' + day % 10; day /= 10;
        dest[6] = '0' + day % 10; day /= 10;
        dest[8] = '-';
        int hour = t->tm_hour;
        dest[10] = '0' + hour % 10; hour /= 10;
        dest[9] = '0' + hour % 10; hour /= 10;
        dest[11] = ':';
        int min = t->tm_min;
        dest[13] = '0' + min % 10; min /= 10;
        dest[12] = '0' + min % 10; min /= 10;
        dest[14] = ':';
        int sec = t->tm_sec;
        dest[16] = '0' + sec % 10; sec /= 10; size = 17;
        dest[15] = '0' + sec % 10; sec /= 10;
        int nsec = std::max(ts % 1000000000LL, 0LL);
        if constexpr (Prec > clock_precision::seconds) {
            dest[17] = '.';
        }
        if constexpr (Prec >= clock_precision::milliseconds) {
            dest[20] = '0' + nsec % 10; nsec /= 10; size = 21;
            dest[19] = '0' + nsec % 10; nsec /= 10;
            dest[18] = '0' + nsec % 10; nsec /= 10;
        }
        if constexpr (Prec >= clock_precision::microseconds) {
            dest[23] = '0' + nsec % 10; nsec /= 10; size = 24;
            dest[22] = '0' + nsec % 10; nsec /= 10;
            dest[21] = '0' + nsec % 10; nsec /= 10;
        }
        if constexpr (Prec >= clock_precision::nanoseconds) {
            dest[26] = '0' + nsec % 10; nsec /= 10; size = 27;
            dest[25] = '0' + nsec % 10; nsec /= 10;
            dest[24] = '0' + nsec % 10; nsec /= 10;
        }
        return size;
    }

    template <typename Func>
    int64_t strtepoch(const char *src, int size, Func&& f)
    {
        int year = 0;
        year = 10 * year + src[0] - '0';
        year = 10 * year + src[1] - '0';
        year = 10 * year + src[2] - '0';
        year = 10 * year + src[3] - '0';
        int month = 0;
        month = 10 * month + src[4] - '0';
        month = 10 * month + src[5] - '0';
        int day = 0;
        day = 10 * day + src[6] - '0';
        day = 10 * day + src[7] - '0';
        int hour = 0;
        hour = 10 * hour + src[9] - '0';
        hour = 10 * hour + src[10] - '0';
        int minute = 0;
        minute = 10 * minute + src[12] - '0';
        minute = 10 * minute + src[13] - '0';
        int second = 0;
        second = 10 * second + src[15] - '0';
        second = 10 * second + src[16] - '0';

        std::tm dt = {};
        dt.tm_year = year - 1900;
        dt.tm_mon = month - 1;
        dt.tm_mday = day;
        dt.tm_hour = hour;
        dt.tm_min = minute;
        dt.tm_sec = second;
        if (size < 20)
            return static_cast<int64_t>(f(&dt));
        int ms = 0;
        int p = 1000;
        ms = 10 * ms + src[18] - '0';
        ms = 10 * ms + src[19] - '0';
        ms = 10 * ms + src[20] - '0';
        if (size > 23) {
            p *= 1000;
            ms = 10 * ms + src[21] - '0';
            ms = 10 * ms + src[22] - '0';
            ms = 10 * ms + src[23] - '0';
        }
        if (size > 26) {
            p *= 1000;
            ms = 10 * ms + src[24] - '0';
            ms = 10 * ms + src[25] - '0';
            ms = 10 * ms + src[26] - '0';
        }
        return static_cast<int64_t>(f(&dt)) * p + ms;
    }
}
}

#endif

