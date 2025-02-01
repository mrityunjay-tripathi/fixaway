/**
* @file fixaway/fixdatetime.hpp
* @author Mrityunjay Tripathi
*
* fixaway is free software; you may redistribute it and/or modify it under the
* terms of the BSD 2-Clause "Simplified" License. You should have received a copy of the
* BSD 2-Clause "Simplified" License along with fixaway. If not, see
* http://www.opensource.org/licenses/BSD-2-Clause for more information.
* Copyright (c) 2025, Mrityunjay Tripathi
*/
#ifndef FIXAWAY_FIXMESSAGE_HPP_
#define FIXAWAY_FIXMESSAGE_HPP_

#include "fixaway/fixbase.hpp"
#include "fixaway/fixtags.hpp"

namespace fixaway {

    enum class FixVersionType {
        FIX_4_0,
        FIX_4_1,
        FIX_4_2,
        FIX_4_3,
        FIX_4_4,
        FIX_5_0
    };

    template <FixVersionType FixVersion> struct FixVersionTag { using type = void; };
    template <> struct FixVersionTag<FixVersionType::FIX_4_0> { using type = FixVersion_4_0; };
    template <> struct FixVersionTag<FixVersionType::FIX_4_1> { using type = FixVersion_4_1; };
    template <> struct FixVersionTag<FixVersionType::FIX_4_2> { using type = FixVersion_4_2; };
    template <> struct FixVersionTag<FixVersionType::FIX_4_3> { using type = FixVersion_4_3; };
    template <> struct FixVersionTag<FixVersionType::FIX_4_4> { using type = FixVersion_4_4; };
    template <> struct FixVersionTag<FixVersionType::FIX_5_0> { using type = FixVersion_5_0; };

    template <typename Target, typename First, typename... Rest>
    struct IsLeader { static constexpr bool value = std::is_same_v<Target, First>; };

    template <typename Target, typename First, typename... Rest>
    constexpr bool IsLeaderV = IsLeader<Target, First, Rest...>::value;

    template <FixVersionType FixVersion, typename ... TvpTypes>
    class FixMessage
    {
        static_assert(
            CheckUniqueV<FixVersionTag<FixVersion>, BodyLength, TvpTypes..., CheckSum>,
            "Tag-Value Pair must be unique in message."
        );
        static_assert(
            IsLeaderV<MessageType, TvpTypes...>,
            "The FIX Message body must start with `MessageType`."
        );
        template <typename TvpType, typename ... Args>
        using ValueType = decltype(std::declval<TvpType>().get(std::declval<Args>()...));
    public:
        FixMessage() {}

        template <typename TvpType, typename ... Args>
        ValueType<TvpType, Args...> get(Args&& ... args) const { return mMsgBody.template get<TvpType, Args...>(std::forward<Args>(args)...); }

        template <typename TvpType, typename ... Args>
        void set(Args&& ... args) { return mMsgBody.template set<TvpType, Args...>(std::forward<Args>(args)...); }

        int dump(char* dest, bool updateBodyLength = false, bool updateCheckSum = false) {
            if (updateBodyLength) setBodyLength();
            if (updateCheckSum) setCheckSum();
            int bW = mMsgHeader.dump(dest);
            bW += mMsgBody.dump(dest + bW);
            bW += mMsgTrailer.dump(dest + bW);
            return bW;
        }

        int parse(const char* src) {
            int bR = mMsgHeader.parse(src);
            bR += mMsgBody.parse(src + bR);
            bR += mMsgTrailer.parse(src + bR);
            return bR;
        }

    private:
        void setBodyLength() {
            int bodyLength = mMsgBody.width();
            mMsgHeader.template set<BodyLength>(bodyLength);
        }
        void setCheckSum() {
            uint8_t checksum = mMsgHeader.sum() + mMsgBody.sum();
            mMsgTrailer.set<CheckSum>(checksum);
        }
    private:
        TvpGroup<typename FixVersionTag<FixVersion>::type, BodyLength> mMsgHeader;
        TvpGroup<TvpTypes ...> mMsgBody;
        TvpGroup<CheckSum> mMsgTrailer;
    };
}

#endif

