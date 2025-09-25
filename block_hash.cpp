// Program to check hash < target for Bitcoin Block
#include "block_hash.h"
#include "utils.hpp"
#include <QDebug>

std::vector<uint> foundedNonces;
std::mutex foundedNoncesMutex;

BLOCK::BLOCK(BlockData data, std::pair<uint, uint> range)
    : data(data), range(range)
{
    Initialize(data);
}

void BLOCK::Initialize(BlockData data)
{
    std::string hex_version = uintToHex(data.version);
    std::string hex_time = uintToHex(data.timestamp);
    std::string bits = uintToHex(data.bits_decimal);

    target = "0000000000000000000000000000000000000000000000000000000000000000";
    Bits_To_Target(bits);

    // Merge all arguments together and give block_without_nonce
    Reverse_by_Pair(hex_version);
    Reverse_by_Pair(data.prev_hash);
    Reverse_by_Pair(data.merkle_root);
    Reverse_by_Pair(hex_time);
    Reverse_by_Pair(bits);
    block_without_nonce = hex_version + data.prev_hash + data.merkle_root + hex_time + bits;
}

void BLOCK::Bits_To_Target(std::string bits)
{
    int last_zero = 2 * (16 * Hex2Int(bits[0]) + Hex2Int(bits[1])-3);
    int first_zero = 64 - last_zero - 6;
    target[first_zero]   =   bits[2];
    target[first_zero + 1] = bits[3];
    target[first_zero + 2] = bits[4];
    target[first_zero + 3] = bits[5];
    target[first_zero + 4] = bits[6];
    target[first_zero + 5] = bits[7];
}

void BLOCK::FindNonce()
{
    for(uint nonce = range.first; nonce <= range.second && nonce != UINT_MAX; ++nonce)
    {
        // Convert nonce to hex format
        std::string hex_nonce = uintToHex(nonce);

        // Finalize block
        Reverse_by_Pair(hex_nonce);
        header = block_without_nonce + hex_nonce;
        std::string double_hash = DoubleSHA256(header);
        Reverse_by_Pair(double_hash);

        // Check validity
        if (double_hash < target)
        {
            qDebug() << "Nonce = " << nonce << "founded in thread " << data.threadId;
            std::lock_guard<std::mutex> lock(foundedNoncesMutex);
            foundedNonces.push_back(nonce);
        }
    }
    qDebug() << "Thread " << data.threadId << " ended";
}

std::string BLOCK::getHeaderWithoutNonce()
{
    return block_without_nonce;
}

std::string BLOCK::getTarget()
{
    return target;
}
