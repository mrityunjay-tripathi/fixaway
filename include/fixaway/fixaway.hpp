/**
* @file fixaway/fixaway.hpp
* @author Mrityunjay Tripathi
*
* fixaway is free software; you may redistribute it and/or modify it under the
* terms of the BSD 2-Clause "Simplified" License. You should have received a copy of the
* BSD 2-Clause "Simplified" License along with fixaway. If not, see
* http://www.opensource.org/licenses/BSD-2-Clause for more information.
* Copyright (c) 2025, Mrityunjay Tripathi
*/
#ifndef FIXAWAY_HPP_
#define FIXAWAY_HPP_

#include "fixaway/fixbase.hpp"
#include "fixaway/fixtags.hpp"
#include "fixaway/fixmessage.hpp"
#include "fixaway/fixmsgtype.hpp"
#include "fixaway/connection.hpp"

namespace fixaway {

    template <typename DataSourceType, typename MessageVisitor>
    class FixEngine
    {
    public:
        using MsgInitials = TvpGroup<BeginString<16>, BodyLength, MessageType>;
        struct PeekMessage {
            int operator()(const char* buffer, MsgInitials& g) {
                g.parse(buffer);
                return g.width<BeginString<16>>() + g.width<BodyLength>() + g.get<BodyLength>() + 7;
            }
        };
    public:
        FixEngine() : visitor(nullptr), dataSource(nullptr) {}
        FixEngine(DataSourceType* dataSource, MessageVisitor* visitor)
            : visitor(visitor), dataSource(dataSource) {}
        FixEngine(const FixEngine& other) = delete;
        FixEngine& operator=(const FixEngine& other) = delete;
        FixEngine(FixEngine&& other) {
            visitor = other.visitor;
            dataSource = other.dataSource;
            other.visitor = nullptr;
            other.dataSource = nullptr;
        }
        FixEngine& operator=(FixEngine&& other) {
            if (this != &other) {
                visitor = other.visitor;
                dataSource = other.dataSource;
                other.visitor = nullptr;
                other.dataSource = nullptr;
            }
        }
        int connect() { return dataSource->connect(); }
        bool perform() {
            bool readFromSource = true;
            if (dataSource->size() >= 32) {
                MsgInitials hdr;
                int msgLen = PeekMessage()(dataSource->read_ptr(), hdr);
                if (dataSource->size() > msgLen) {
                    readFromSource = false;
                    MessageTypeEnum msgType = MsgTypeStringToEnum(hdr.get<MessageType>());
                    visitor->operator()(msgType, dataSource->read_ptr(), msgLen);
                    dataSource->move_head(msgLen);
                    return msgLen > 0;
                }
            }
            if (readFromSource) dataSource->poll();
            return false;
        }
        template <typename TFixMessage>
        size_t sendmsg(const TFixMessage& msg, bool updateCheckSum = true) {
            int bytes = msg.dump(requestBuf, true, updateCheckSum);
            return bytes > 0 ? dataSource->send_message(requestBuf, bytes) : 0;
        }
    private:
        //! 8kb of request can be send at a time.
        //! If you want to send more, construct message,
        //! and send using the DataSourceType handle.
        char requestBuf[8192];
        MessageVisitor* visitor;
        DataSourceType* dataSource;
    };

}

#endif

