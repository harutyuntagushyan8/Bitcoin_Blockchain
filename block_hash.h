#ifndef BLOCK_HASH_H
#define BLOCK_HASH_H

#include <string>
#include <vector>
#include <mutex>

typedef unsigned int uint;
extern std::vector<uint> foundedNonces;
extern std::mutex foundedNoncesMutex;

struct BlockData {
    std::string hex_version;
    std::string prev_hash;
    std::string merkle_root;
    std::string hex_time;
    std::string bits;
    uint version;
    uint difficulty;
    uint bits_decimal;
    uint timestamp;
    uint threadId;
};

class BLOCK
{
public:
    BLOCK(BlockData data, std::pair<uint, uint> range);

private:
    void Bits_To_Target(std::string bits);
    void Initialize(BlockData data);
public:
    void FindNonce();
    std::string getHeaderWithoutNonce();
    std::string getTarget();

private:
    BlockData data;
    std::pair<uint, uint> range;
    std::string target;
    std::string block_without_nonce;
    std::string header;
};

#endif // BLOCK_HASH_H
