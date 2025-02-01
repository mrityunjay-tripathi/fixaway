#include <iostream>
#include <ctime>
#include <vector>
#include <cstdint>

#include "fixaway/fixaway.hpp"
#include "fixaway/fixdatetime.hpp"

using namespace fixaway;
using PxArray = TvpArray<AvgPx, 4>;

typedef FixMessage<
    FixVersionType::FIX_4_4,
    MessageType, MsgSeqNum, SenderCompId, TargetCompId, SendingTime,
    NoMDEntries, PxArray, CumQty
> MarketUpdate;

typedef FixMessage<
    FixVersionType::FIX_4_4,
    MessageType, MsgSeqNum, SenderCompId, TargetCompId, SendingTime,
    ClOrdID, OrigClOrdID, Price, OrderQty
> ExecutionReport;

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage:\n\t<filename>\n";
        return -1;
    }
    std::string filename = std::string(argv[1]);

    file_client fc(filename,
            [](){ std::cout << "Connected" << std::endl; },
            [](){ std::cout << "Disconnected" << std::endl; },
            [](int ec, const std::string& msg){ std::cout << "Error:" << ec << "," << msg << std::endl; });
    fc.connect();

    struct MessageVisitor
    {
        int count = 0;
        void operator()(MessageTypeEnum msgType, const char* buffer, size_t n)
        {
            count++;
            char d[8192];
            // int64_t start = system_timestamp();
            if (msgType == MessageTypeEnum::MarketDataIncrementalRefresh) {
                MarketUpdate u;
                u.parse(buffer);

                int b = u.dump(d);
                // std::cout << details::fixstring(d, b) << std::endl;
            }
            else if (msgType == MessageTypeEnum::ExecutionReport) {
                ExecutionReport e;
                e.parse(buffer);

                int b = e.dump(d);
                // std::cout << details::fixstring(d, b) << std::endl;
            }
            // std::cout << (end - start) << std::endl;
        }
    };

    MessageVisitor mv;
    FixEngine e(&fc, &mv);

    int64_t start, end;
    try {
        start = system_timestamp();
        while (fc.active()) {
            e.perform();
        }
    }
    catch (const connection_exception& exc) {
        end = system_timestamp();
        char bufStart[32]; std::memset(bufStart, 0, sizeof(bufStart));
        strfutc<clock_precision::nanoseconds>(bufStart, end);
        char bufEnd[32]; std::memset(bufEnd, 0, sizeof(bufEnd));
        strfutc<clock_precision::nanoseconds>(bufEnd, end);
        std::cout << "Exception: " << exc.what() << ", avg time: " << (end - start) / mv.count << ", start: " << bufStart << ", end: " << bufEnd << std::endl;
    }
}

