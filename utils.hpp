#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <charconv>
#include <array>
#include "sha2.h"

typedef unsigned int uint;

inline int Hex2Int(char hexChar)
{
    static const std::array<uint8_t, 256> lookupTable = [](){
        std::array<uint8_t, 256> table{};
        for (int i = 0; i < 256; ++i)
        {
            if (i >= '0' && i <= '9')
                table[i] = i - '0';
            else if (i >= 'a' && i <= 'f')
                table[i] = i - 'a' + 10;
            else if (i >= 'A' && i <= 'F')
                table[i] = i - 'A' + 10;
            else
                table[i] = 0; // Invalid hex character
        }
        return table;
    }();
    return lookupTable[static_cast<uint8_t>(hexChar)];
}

inline std::string uintToHex(uint32_t num)
{
    std::array<char, 8> buffer{};  // Exactly 8 hex digits, no null terminator needed
    auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), num, 16);

    // Ensure zero-padding if the number has fewer than 8 digits
    size_t digits = result.ptr - buffer.data();
    std::string hex_str(8 - digits, '0');  // Prepend missing zeros
    hex_str.append(buffer.data(), digits);  // Append converted digits

    return hex_str;
}

// Due to bitcoin specification data before hashing must be in littlendian
inline void Reverse_by_Pair(std::string& input)
{
    for(size_t i = 0, middle = input.size()/2, size = input.size(); i < middle ; i+=2 )
    {
        std::swap(input[i], input[size - i- 2]);
        std::swap(input[i+1], input[size -i - 1]);
    }
}

inline std::string DoubleSHA256(const std::string& hexText)
{
    SHA256 sha;
    std::string str1(80, '\0');
    for(uint i = 0; i < 160; i += 2)
    {
        // convert each pair of hex digits to a byte and store it in str1
        unsigned char byte = (unsigned char) ((Hex2Int(hexText[i]) << 4) + Hex2Int(hexText[i + 1]));
        str1[i / 2] = byte;
    }
    // First SHA256 hash
    std::string s = sha(str1);

    std::string str2(32, '\0');
    for(uint i = 0; i < 64; i += 2)
    {
        // convert each pair of hex digits to a byte and store it in str2
        unsigned char byte = (unsigned char) ((Hex2Int(s[i]) << 4) + Hex2Int(s[i + 1]));
        str2[i / 2] = byte;
    }
    // Return Second SHA256 hash
    return sha(str2);
}


#endif // UTILS_HPP
