#include <iostream>
#include <ctime>
#include <vector>
#include <cstdint>
#include <fstream>
#include <random>

#include "fixaway/fixaway.hpp"
#include "common.hpp"

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

int generate_fix_message(int N, const char* filename) {
    std::ofstream file;
    file.open(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open file to write.\n";
        return 0;
    }

    char buffer[8192];
    std::memset(buffer, 0, sizeof(buffer));

    MarketUpdate g;
    g.set<MessageType>(MessageTypeEnum::MarketDataIncrementalRefresh);
    g.set<SendingTime>("0");
    g.set<TargetCompId>("TSERVER");
    g.set<SenderCompId>("DERIBITSERVER");

    ExecutionReport e;
    e.set<MessageType>(MessageTypeEnum::ExecutionReport);
    e.set<SendingTime>("0");
    e.set<TargetCompId>("TSERVER");
    e.set<SenderCompId>("DERIBITSERVER");

    for (int i = 0; i < N; ++i) {
        int bW = 0;
        bool which = rand() % 2;
        if (which) {
            int64_t noMdEntries = 4;
            {
                g.set<MsgSeqNum>(i);
                g.set<NoMDEntries>(noMdEntries);
                for (size_t i = 0; i < noMdEntries; ++i) {
                    g.set<PxArray>(i, random_number(100.0, 200.0), 5);
                }
                g.set<CumQty>(random_number(0.5, 2.0), 5);
            }
            bW = g.dump(buffer, true, true);
        } else {
            e.set<MsgSeqNum>(i);
            e.set<ClOrdID>(std::to_string(random_number(100000, 999999)));
            e.set<OrigClOrdID>(std::to_string(random_number(100000, 999999)));
            e.set<Price>(random_number(100.0, 200.0), 5);
            e.set<OrderQty>(random_number(0.5, 2.0), 5);
            bW = e.dump(buffer, true, true);
        }
        file << std::string(buffer, bW);
    }
    return 1;
}

int parse_fix_message(int N, const char* filename) {
    std::ifstream file;
    file.open(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open file";
        return 0;
    }
    MarketUpdate g;

    std::string line;
    line.reserve(512);
    std::vector<std::string> msgLines;
    msgLines.reserve(N);

    uint64_t bytes = 0, count = 0;
    while (std::getline(file, line)) {
        msgLines.push_back(line);
        count++;
    }
    int64_t start = system_timestamp();
    for (const auto& l : msgLines) {
        bytes += g.parse(l.c_str());
    }
    int64_t end = system_timestamp();
    std::cout << "Parsing|nmsgs:" << count << "|bytes:" << bytes << "|latency:" << (end - start) * 1e-6 << "ms|rate:" << (double)(count) * 1e3 / (double)(end - start) << " M msg/sec."<< std::endl;
    return 1;
}

int main(int argc, const char* argv[])
{
    // std::cout << "MarketUpdate Size:" << sizeof(MarketUpdate) << std::endl;
    if (argc != 2) {
        std::cout << "Usage:\n\t<count>\n";
        return -1;
    }
    int N = std::stoi(argv[1]);
    std::string filename = "data/" + details::randstr(16) + ".fix";

    if (!generate_fix_message(N, filename.c_str())) return -1;
    // if (!parse_fix_message(N, filename.c_str())) return -1;
    return 0;
}

