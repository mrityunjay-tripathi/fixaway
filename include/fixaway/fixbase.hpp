/**
* @file fixaway/fixbase.hpp
* @author Mrityunjay Tripathi
*
* fixaway is free software; you may redistribute it and/or modify it under the
* terms of the BSD 2-Clause "Simplified" License. You should have received a copy of the
* BSD 2-Clause "Simplified" License along with fixaway. If not, see
* http://www.opensource.org/licenses/BSD-2-Clause for more information.
* Copyright (c) 2025, Mrityunjay Tripathi
*/
#ifndef FIXAWAY_FIXBASE_HPP_
#define FIXAWAY_FIXBASE_HPP_

#include <array>
#include <vector>
#include <cstring>
#include <cassert>

#define FIXAWAY_FILENAME (strrchr("/" __FILE__, '/') + 1)
#define FIXAWAY_ASSERT(x, msg)                                                                                              \
    do                                                                                                                      \
    {                                                                                                                       \
        bool v = x;                                                                                                         \
        if (!(v))                                                                                                           \
        {                                                                                                                   \
            fprintf(stderr, "ERROR: assertion \"%s\" failed at %s:%d  %s\n", msg, FIXAWAY_FILENAME, __LINE__, __func__);    \
            abort();                                                                                                        \
        }                                                                                                                   \
    } while (0)

#define FIXAWAY_THROW(msg)                                                                                                  \
    do                                                                                                                      \
    {                                                                                                                       \
        fprintf(stderr, "EXCEPTION \"%s\" thrown at %s:%d  %s\n", msg, FIXAWAY_FILENAME, __LINE__, __func__);               \
        exit(EXIT_FAILURE);                                                                                                 \
    } while (0)


namespace fixaway { namespace details {

    template <size_t N>
    constexpr size_t largest_power_of_2_less_than() {
        size_t x = N;
        x |= x >> 1; x |= x >> 2; x |= x >> 4;
        x |= x >> 8; x |= x >> 16; x |= x >> 32;
        return x ^ (x >> 1);
    }

    template <size_t N>
    size_t smallest_power_of_2_greater_than() {
        return largest_power_of_2_less_than<N>() << 1;
    }

    /**
     * Integer to String conversion.
     * @param dest The pointer to output buffer.
     * @param val The integral value that is to be converted.
     */
    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    size_t itoa(char* dest, T val)
    {
        if (val < 0) { *dest = '-'; return 1 + itoa(dest + 1, -val); }
        if (val == 0) { *dest = '0'; return 1; }
        int digits = 0;
        T copy = val;
        while (copy > 0) { copy /= 10; digits++; }
        char *d = dest + digits - 1;
        while (val > 0) { *d = '0' + val % 10; val /= 10; d--; }
        return digits;
    }

    /**
     * Double to String conversion.
     * @param dest The pointer to output buffer.
     * @param val The floating point value that is to be converted.
     */
    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
    size_t dtoa(char *dest, T val, int accuracy = 10)
    {
        int ints = 0;
        int64_t valc = std::abs(val);
        while (valc > 0) { valc /= 10; ints++; }
        int width = (val < 0.0) + ints + 1 + accuracy;
        if (val < 0.0) { dest[0] = '-'; val = -val; }
        int64_t inp = (int64_t)val;
        double fr = val - inp;
        char *d = dest + ints - 1;
        while (inp > 0) { *d = '0' + inp % 10; inp /= 10; d--; }
        dest[ints] = '.';
        int s = ints;
        while (accuracy--) { fr *= 10; int tmp = (int)fr; fr -= tmp; dest[++s] = '0' + tmp; }
        return width;
    }

    /**
     * String to Integer conversion.
     * @param first The begin pointer of buffer.
     * @param last The end pointer of buffer.
     * @param output The reference to output variable.
     */
    template <typename First, typename Last, typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    bool atoi(First first, Last last, T &output)
    {
        T val = 0;
        int sign = (*first != '-') - (*first == '-');
        auto itr = sign == 1 ? first : first + 1;
        for (; itr != last; ++itr) {
            int digit = (*itr) - 48;
            if ((0 <= digit) & (digit <= 9)) { val = 10 * val + digit; }
            else return false;
        }
        output = sign * val;
        return true;
    }

    /**
     * String to Double conversion.
     * @param first The begin pointer of buffer.
     * @param last The end pointer of buffer.
     * @param output The reference to output variable.
     */
    template <typename First, typename Last, typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
    bool atod(First first, Last last, T &output)
    {
        T val = 0.0;
        int sign = (*first != '-') - (*first == '-');
        auto itr = sign == 1 ? first : first + 1;
        for (; itr != last; ++itr) {
            int digit = (*itr) - 48;
            if ((0 <= digit) & (digit <= 9)) { val = 10 * val + digit; }
            else if (*itr == '.') { ++itr; break; }
            else return false;
        }
        T divisor = 10.0;
        for (; itr != last; ++itr) {
            int digit = (*itr) - 48;
            if ((0 <= digit) & (digit <= 9)) { val += digit / divisor; divisor *= 10.0; }
            else return false;
        }
        output = sign * val;
        return true;
    }

    inline std::pair<int, int> find_tag(const char *haystack, const int m, const char *needle, const int n)
    {
        // Assuming FIX field tags cannot be greater than "999999".
        char tag[8]; tag[0] = '\x01'; std::memcpy(tag + 1, needle, n); tag[n + 1] = '=';
        int start = -1;
        for (int i = 0; i + n + 2 < m; ++i) {
            if (0 == std::memcmp(&haystack[i], tag, n + 2)) { start = i + n + 2; break; }
        }
        if (start != -1) {
            for (int end = start; end < m; ++end) {
                if (haystack[end] == '\x01') return {start, end};
            }
        }
        return {start, -1};
    }


    /**
     * Get the FIX message with `delimiter` replaced with '|' as delimiter.
     * @param s The FIX message string.
     * @param delimiter The separator of FIX message.
     * @returns String that has '|' as delimiter in place of `delimiter`.
     */
    inline std::string fixstring(const std::string &s, char delimiter = '\x01')
    {
        std::string res;
        size_t start = 0;
        size_t end = s.find_first_of(delimiter);
        while (end <= std::string::npos) {
            res += s.substr(start, end - start) + "|";
            if (end == std::string::npos) break;
            start = end + 1;
            end = s.find_first_of(delimiter, start);
        }
        return res;
    }

    /**
     * Get the FIX message with `\x01` replaced with '|' as delimiter.
     * @param buffer The pointer to FIX message string.
     * @param size The size of the FIX message string.
     * @returns String that has '|' as delimiter in place of SOH.
     */
    inline std::string fixstring(const char *buffer, int size)
    {
        return fixstring(std::string(buffer, size), '\x01');
    }

    /**
     * Generate random string of certain length.
     */
    inline std::string randstr(int length)
    {
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        std::string random_str;
        random_str.reserve(length);

        for (int i = 0; i < length; ++i) {
            random_str += characters[std::rand() % characters.size()];
        }
        return random_str;
    }
}}

namespace fixaway {

    static constexpr const char SEPARATOR = '\x01';
    typedef const char* const* TagReference;
    typedef const char* const* MsgTypeReference;
    struct TvpParseData {
        const char* buffer; int64_t meta;
        TvpParseData(const char* buffer = nullptr, int64_t meta = -1) :
            buffer(buffer), meta(meta) {}
    };

    template <typename TagType, size_t VSize, TagReference Tag, size_t TSize = strlen(*Tag)>
    struct alignas(details::largest_power_of_2_less_than<TSize + VSize + sizeof(size_t)>()) TvpStatic
    {
        enum : size_t { TagSize = TSize };
        enum : size_t { ValueSize = VSize };
        const char* tag = *Tag;
        char value[VSize];
        size_t usedLen = 0;
        TvpStatic() {}
        template <size_t VSizeOther>
        bool operator==(const TvpStatic<TagType, VSizeOther, Tag, TSize>& other) {
            return usedLen == other.usedLen && 0 == std::memcmp(value, other.value, usedLen);
        }
        template <size_t VSizeOther>
        bool operator!=(const TvpStatic<TagType, VSizeOther, Tag, TSize>& other) {
            return !(*this == other);
        }
        int dump(char* dest) const {
            if (usedLen == 0) return 0;
            int bW = 0;
            std::memcpy(dest + bW, tag, TagSize); bW += TagSize;
            dest[bW] = '='; bW += 1;
            std::memcpy(dest + bW, value, usedLen); bW += usedLen;
            dest[bW] = SEPARATOR; bW += sizeof(SEPARATOR);
            return bW;
        }
        int parse(TvpParseData& pd) {
            int i = 0, bR = 0;
            if (0 != std::memcmp(pd.buffer, tag, TagSize)) return bR;
            bR += TagSize + 1;      // Tag and assign character processed.
            while (pd.buffer[bR] != SEPARATOR) value[i++] = pd.buffer[bR++];
            bR += 1; usedLen = i;   // Tag Value Pair Separator also processed.
            pd.buffer += bR;
            return bR;
        }
        constexpr int width() const { return TSize + 1 + usedLen + 1; }
        constexpr uint8_t sum() const {
            uint8_t w = uint8_t(0);
            int i = 0; while (i < TSize) w += tag[i++]; w += '=';
            i = 0; while (i < usedLen) w += value[i++]; w += SEPARATOR;
            return w;
        }
    protected:
        void specifytag(const char* t) { std::memcpy(tag, t, TagSize); }
    };

    template <typename TagType, TagReference Tag, size_t TSize = strlen(*Tag)>
    struct TvpDynamic
    {
        enum : size_t { TagSize = TSize };
        size_t ValueSize = 0;
        const char* tag = *Tag;
        std::string value;
        TvpDynamic() {}
        bool operator==(const TvpDynamic& other) {
            return value == other.value;
        }
        bool operator!=(const TvpDynamic& other) {
            return !(*this == other);
        }
        int dump(char* dest) const {
            if (ValueSize == 0) return 0;
            int bW = 0;
            std::memcpy(dest + bW, tag, TagSize); bW += TagSize;
            dest[bW] = '='; bW += 1;
            std::memcpy(dest + bW, value.c_str(), ValueSize); bW += ValueSize;
            dest[bW] = SEPARATOR; bW += sizeof(SEPARATOR);
            return bW;
        }
        int parse(TvpParseData& pd) {
            int bR = 0;
            if (0 != std::memcmp(pd.buffer, tag, TagSize)) return bR;
            bR += TagSize + 1;  // Tag and assign character processed.
            value.reserve(64);
            while (pd.buffer[bR] != SEPARATOR) { value.push_back(pd.buffer[bR++]); }
            bR += 1;            // Tag Value Pair Separator also processed.
            pd.buffer += bR;
            return bR;
        }
        constexpr int width() const { return TSize + 1 + ValueSize + 1; }
        constexpr uint8_t sum() const {
            uint8_t w = uint8_t(0);
            int i = 0; while (i < TSize) w += tag[i++]; w += '=';
            i = 0; while (i < ValueSize) w += value[i++]; w += SEPARATOR;
            return w;
        }
    protected:
        void specifytag(const char* t) { std::memcpy(tag, t, TagSize); }
    };

    template <TagReference Tag>
    struct TvpChar : public TvpStatic<TvpChar<Tag>, 1, Tag> {
        typedef TvpStatic<TvpChar<Tag>, 1, Tag> Base;
        TvpChar() : Base() { Base::usedLen = 0; }
        TvpChar(char c) : Base() { set(c); }
        char get() const { return Base::value[0]; }
        void set(const char& val) { Base::value[0] = val; Base::usedLen = 1; }
    };
    template <size_t VSize, TagReference Tag>
    struct TvpStringFixed : public TvpStatic<TvpStringFixed<VSize, Tag>, VSize, Tag> {
        typedef TvpStatic<TvpStringFixed<VSize, Tag>, VSize, Tag> Base;
        TvpStringFixed() : Base() { Base::usedLen = 0; }
        TvpStringFixed(const std::string& str) : Base() { set(str); }
        TvpStringFixed(const char* str, size_t strLen) : Base() { set(std::string_view(str, strLen)); }
        std::string_view get() const { return std::string_view(Base::value, Base::usedLen); }
        void set(const std::string_view& val) {
            FIXAWAY_ASSERT(val.size() < VSize, "string size greater than capacity");
            std::memcpy(Base::value, val.data(), val.size());
            Base::usedLen = val.size();
        }
    };
    template <TagReference Tag>
    struct TvpStringDynamic : public TvpDynamic<TvpStringDynamic<Tag>, Tag> {
        typedef TvpDynamic<TvpStringDynamic<Tag>, Tag> Base;
        TvpStringDynamic() : Base() {}
        TvpStringDynamic(const std::string& str) : Base() { set(str); }
        TvpStringDynamic(const char* str, size_t strLen) : Base() { set(std::string_view(str, strLen)); }
        std::string_view get() const { return std::string_view(Base::value, Base::ValueSize); }
        void set(const std::string_view& val) { Base::value = val; Base::ValueSize = val.size(); }
    };
    template <typename IntegerType, size_t VSize, TagReference Tag>
    struct TvpInteger : public TvpStatic<TvpInteger<IntegerType, VSize, Tag>, VSize, Tag> {
        static_assert(std::is_integral<IntegerType>::value, "IntegerType must be integral");
        typedef TvpStatic<TvpInteger<IntegerType, VSize, Tag>, VSize, Tag> Base;
        TvpInteger() : Base() { Base::usedLen = 0; }
        TvpInteger(IntegerType val) : Base() { set(val); }
        IntegerType get() const { IntegerType output; details::atoi(Base::value, Base::value + Base::usedLen, output); return output; }
        void set(const IntegerType& val) { Base::usedLen = details::itoa(Base::value, val); }
    };
    template <typename FloatType, size_t VSize, TagReference Tag>
    struct TvpFloat : public TvpStatic<TvpFloat<FloatType, VSize, Tag>, VSize, Tag> {
        static_assert(std::is_floating_point<FloatType>::value, "FloatType must be floating point number");
        typedef TvpStatic<TvpFloat<FloatType, VSize, Tag>, VSize, Tag> Base;
        TvpFloat() : Base() { Base::usedLen = 0; }
        TvpFloat(FloatType val) : Base() { set(val); }
        FloatType get() const { FloatType output; details::atod(Base::value, Base::value + Base::usedLen, output); return output; }
        void set(const FloatType& val, uint8_t decimals = 4) { Base::usedLen = details::dtoa(Base::value, val, decimals); }
    };
    template <typename TvpType, size_t ArraySize>
    struct TvpArray
    {
    public:
        enum : size_t { Size = ArraySize };
        size_t usedLen = 0;
        template <typename T>
        using ValueType = decltype(((T*)(nullptr))->get());
        TvpArray() {}
        TvpType& operator[](size_t i) { return data[i]; }
        const TvpType& operator[](size_t i) const { return data[i]; }
        ValueType<TvpType> get(size_t i = 0) const { return data[i].get(); }
        template <typename ... Args>
        void set(size_t i, Args&& ... args) { data[i].set(std::forward<Args>(args)...); usedLen = std::max(usedLen, i + 1); }
        int dump(char* dest) const {
            int w = 0; for (size_t i = 0; i < usedLen; ++i) { w += data[i].dump(dest + w); } return w;
        }
        int parse(TvpParseData& pd) {
            assert((void("TvpArray expects 0 <= size < Size"), (pd.meta != -1) & (pd.meta <= Size)));
            int w = 0; usedLen = pd.meta; for (size_t i = 0; i < usedLen; ++i) { w += data[i].parse(pd); } return w;
        }
        constexpr int width() const {
            int w = 0; for (size_t i = 0; i < usedLen; ++i) { w += data[i].width(); } return w;
        }
        uint8_t sum() const {
            uint8_t w = uint8_t(0); for (size_t i = 0; i < usedLen; ++i) { w += data[i].sum(); } return w;
        }
    private:
        std::array<TvpType, ArraySize> data;
    };
    template <typename TvpType>
    struct TvpVector
    {
    public:
        size_t Size = 0;
        template <typename T>
        using ValueType = decltype(((T*)(nullptr))->get());
        TvpVector() {}
        TvpVector(size_t capacity) { resize(capacity); }
        TvpType& operator[](size_t i) { return data[i]; }
        const TvpType& operator[](size_t i) const { return data[i]; }
        void resize(size_t capacity) { data.resize(capacity); Size = capacity; }
        ValueType<TvpType> get(size_t i) const { return data[i].get(); }
        template <typename ... Args>
        void set(size_t i, Args&& ... args) { data[i].set(std::forward<Args>(args)...); }
        int dump(char* dest) const {
            int w = 0; for (const auto& t : data) { w += t.dump(dest + w); } return w;
        }
        int parse(TvpParseData& pd) {
            assert((void("TvpVector expects size >= 0"), pd.meta != -1));
            data.resize(pd.meta);
            int w = 0; for (auto& t : data) { w += t.parse(pd); } return w;
        }
        constexpr int width() const {
            int w = 0; for (const auto& t : data) { w += t.width(); } return w;
        }
        uint8_t sum() const {
            uint8_t w = uint8_t(0); for (const auto& t : data) { w += t.sum(); } return w;
        }
    private:
        std::vector<TvpType> data;
    };

    template <typename ...> struct CheckUnique;
    template <> struct CheckUnique<> : std::true_type {};

    template <typename T, typename ... Args>
    struct CheckUnique<T, Args...>
        : std::conditional_t<(std::is_same_v<T, Args> || ...), std::false_type, CheckUnique<Args...>> {};

    template <typename ... TvpTypes>
    constexpr bool CheckUniqueV = CheckUnique<TvpTypes...>::value;

    template <typename ... TvpTypes>
    struct TvpGroup : public TvpTypes...
    {
        static_assert(CheckUniqueV<TvpTypes...>, "Tag-Value Pair must be unique in group.");
        template <typename TvpType, typename ... Args>
        using ValueType = decltype(std::declval<TvpType>().get(std::declval<Args>()...));
        template <typename TvpType, typename ... Args>
        using ReturnTypeOfResize = decltype(std::declval<TvpType>().resize(std::declval<Args>()...));
        TvpGroup() : TvpTypes()... {}
        TvpGroup(TvpTypes&& ... tvpTypes) : TvpTypes(std::forward<TvpTypes>(tvpTypes))... {}
        template <typename TvpType, typename ... Args>
        ReturnTypeOfResize<TvpType, Args...> resize(Args&& ... args) { return TvpType::resize(std::forward<Args>(args)...); }
        template <typename TvpType, typename ... Args>
        ValueType<TvpType, Args...> get(Args&& ... args) const { return TvpType::get(std::forward<Args>(args)...); }
        template <typename TvpType, typename ... Args>
        void set(Args&& ... args) { return TvpType::set(std::forward<Args>(args)...); }
        int dump(char* dest) const { return dump_impl(dest, static_cast<const TvpTypes*>(this)...); }
        int parse(const char* src) { TvpParseData pd(src, -1); return parse(pd); }
        int parse(TvpParseData& pd) { return parse_impl(pd, static_cast<TvpTypes*>(this)...); }
        template <typename TvpType>
        constexpr int width() const { return TvpType::width(); }
        constexpr int width() const { return width_impl(static_cast<const TvpTypes*>(this)...); }
        constexpr uint8_t sum() const { return sum_impl(static_cast<const TvpTypes*>(this)...); }
    private:
        int dump_impl(char* buffer) const { return 0; }
        template <typename T, typename ... Args>
        int dump_impl(char* buffer, T* first, Args* ... args) const {
            int ret = first->dump(buffer);
            return ret + dump_impl(buffer + ret, args...);
        }
        int parse_impl(TvpParseData& pd) { return 0; }
        template <typename T, typename ... Args>
        int parse_impl(TvpParseData& pd, T* first, Args* ... args) {
            int ret = first->parse(pd);
            return ret + parse_impl(pd, args...);
        }
        constexpr int width_impl() const { return 0; }
        template <typename T, typename ... Args>
        constexpr int width_impl(T* first, Args* ... args) const { return first->width() + width_impl(args...); }
        constexpr uint8_t sum_impl() const { return uint8_t(0); }
        template <typename T, typename ... Args>
        constexpr uint8_t sum_impl(T* first, Args* ... args) const { return first->sum() + sum_impl(args...); }
    };

}
#endif

