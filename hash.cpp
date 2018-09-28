#include "hash.h"
#include "sha2.h"
#include <chrono>
#include <cmath>

double HASH::ThreadHashrate()
{
    SHA256 sha;
    auto start = std::chrono::high_resolution_clock::now();
    for(uint i = interval_start; i < interval_end; ++i)
    {

        sha("Bitcoin");
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration <double> elapsed = end - start;
    std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds> (elapsed);
    double result = (double)(interval_end - interval_start) * 1000 / (d.count() * std::pow(10,6));   // return MH/s
    return  result;
}

void HASH::ReceiveIntervals(uint interval_start, uint interval_end)
{
    this->interval_start = interval_start;
    this->interval_end = interval_end;
}

void HASH::Start()
{
    double result = ThreadHashrate();
    emit HashrateReady(result);
    emit finished();
}
