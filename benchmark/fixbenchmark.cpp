#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <random>
#include <stdio.h>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <benchmark/benchmark.h>

#include "fixaway.hpp"

#define LO 1 << 1
#define HI 1 << 8

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
namespace bm_details
{
    std::string generateRandomString(int length)
    {
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        std::string randomString;
        randomString.reserve(length);

        for (int i = 0; i < length; ++i)
        {
            randomString += characters[std::rand() % characters.size()];
        }

        return randomString;
    }
}

static void BM_FixWriter(benchmark::State &state)
{
    const size_t N = state.range(0);
    std::vector<double> randomPx;
    for (size_t i = 0; i < N; ++i) {
        randomPx.push_back(random_number(10000.0, 20000.0));
    }

    using namespace fixaway;
    typedef TvpGroup<FixVersion_4_4, BodyLength> FixHeader;
    typedef TvpGroup<MessageType, MsgSeqNum, SenderCompId<16>, TargetCompId<16>, SendingTime<20>> MessageHeader;
    using PxArray = TvpArray<AvgPx, 32>;
    typedef TvpGroup<NoMDEntries, PxArray> MessageBody;
    typedef TvpGroup<CheckSum> FixTrailer;

    typedef TvpGroup<FixHeader, MessageHeader, MessageBody, FixTrailer> Message;

    Message g;
    int64_t noMdEntries = N;
    {
        g.set<MessageType>("GN");
        g.set<MsgSeqNum>(567);
        g.set<SenderCompId<16>>("TSERVER");
        g.set<TargetCompId<16>>("DERIBITSERVER");
        g.set<SendingTime<20>>("0");
        g.set<NoMDEntries>(noMdEntries);
        for (size_t i = 0; i < noMdEntries; ++i) {
            g.set<PxArray>(i, randomPx[i], 2);
        }
        int bodyLen = g.width() - g.width<FixVersion_4_4>() - g.width<BodyLength>() - g.width<CheckSum>();
        g.set<BodyLength>(bodyLen);
        g.set<CheckSum>(104);
    }

    int bW = 0;
    char buffer[8192];
    for (auto _ : state)
    {
        bW = g.dump(buffer);
        benchmark::ClobberMemory();
    }
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(long(state.iterations()) * long(N));
}
BENCHMARK(BM_FixWriter)->RangeMultiplier(2)->Range(2, 32)->Complexity(benchmark::oN);

static void BM_FixReader(benchmark::State &state)
{
    const size_t N = state.range(0);

    using namespace fixaway;
    typedef TvpGroup<FixVersion_4_4, BodyLength> FixHeader;
    typedef TvpGroup<MessageType, MsgSeqNum, SenderCompId<16>, TargetCompId<16>, SendingTime<20>> MessageHeader;
    using PxArray = TvpArray<AvgPx, 18>;
    typedef TvpGroup<CumQty, NoMDEntries, PxArray> MessageBody;
    typedef TvpGroup<CheckSum> FixTrailer;

    typedef TvpGroup<FixHeader, MessageHeader, MessageBody, FixTrailer> Message;
    Message g;

    int bW = 0;
    const char* buffer = "8=FIX.4.4""\x01""9=84""\x01""35=GN""\x01""34=567""\x01""49=TSERVER""\x01""56=DERIBITSERVER""\x01""52=0""\x01""268=4""\x01""6=45.89""\x01""6=45.89""\x01""6=45.89""\x01""6=45.89""\x01""10=104""\x01""";
    for (auto _ : state)
    {
        bW = g.parse(buffer);
        benchmark::ClobberMemory();
    }
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(long(state.iterations()) * long(N));
}
BENCHMARK(BM_FixReader)->RangeMultiplier(2)->Range(LO, HI)->Complexity(benchmark::oN);


static void BM_FixReader2(benchmark::State &state)
{
    const size_t N = state.range(0);

    using namespace fixaway;
    using PxArray = TvpArray<AvgPx, 8>;

    typedef FixMessage<
        FixVersionType::FIX_4_4,
        MessageType, MsgSeqNum, SenderCompId<16>, TargetCompId<16>, SendingTime<20>,
        NoMDEntries, PxArray
    > Message;
    Message g;

    int bW = 0;
    const char* buffer = "8=FIX.4.4""\x01""9=84""\x01""35=GN""\x01""34=567""\x01""49=TSERVER""\x01""56=DERIBITSERVER""\x01""52=0""\x01""268=4""\x01""6=45.89""\x01""6=45.89""\x01""6=45.89""\x01""6=45.89""\x01""10=104""\x01""";
    for (auto _ : state)
    {
        bW = g.parse(buffer);
        benchmark::ClobberMemory();
    }
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(long(state.iterations()) * long(N));
}
BENCHMARK(BM_FixReader2)->RangeMultiplier(2)->Range(LO, HI)->Complexity(benchmark::oN);

BENCHMARK_MAIN();
