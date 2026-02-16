#include "gtest/gtest.h"

#include "../../src/scripts/bitcoin_bech32.hpp"
#include "../../src/scripts/bitcoin_segwit_addr.hpp"

#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

namespace {

template<int frombits, int tobits, bool pad>
bool convertbits(std::vector<uint8_t> &out, const std::vector<uint8_t> &in) {
    int acc = 0;
    int bits = 0;
    const int maxv = (1 << tobits) - 1;
    const int max_acc = (1 << (frombits + tobits - 1)) - 1;
    for (size_t i = 0; i < in.size(); ++i) {
        const int value = in[i];
        acc = ((acc << frombits) | value) & max_acc;
        bits += frombits;
        while (bits >= tobits) {
            bits -= tobits;
            out.push_back(static_cast<uint8_t>((acc >> bits) & maxv));
        }
    }
    if (pad) {
        if (bits) out.push_back(static_cast<uint8_t>((acc << (tobits - bits)) & maxv));
    } else if (bits >= frombits || ((acc << (tobits - bits)) & maxv)) {
        return false;
    }
    return true;
}

std::string encodeWithVariant(const std::string &hrp, int witver, const std::vector<uint8_t> &witprog, bech32::Encoding encoding) {
    std::vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>(witver));
    convertbits<8, 5, true>(data, witprog);
    return bech32::encode(hrp, data, encoding);
}

} // namespace

TEST(TaprootAddress, WitnessV1RoundTripUsesBech32m) {
    std::vector<uint8_t> witprog(32);
    std::iota(witprog.begin(), witprog.end(), 0);

    const std::string addr = segwit_addr::encode("bc", 1, witprog);
    ASSERT_FALSE(addr.empty());
    ASSERT_EQ(addr.rfind("bc1p", 0), 0U);

    const auto decoded = segwit_addr::decode("bc", addr);
    ASSERT_EQ(decoded.first, 1);
    ASSERT_EQ(decoded.second, witprog);
}

TEST(TaprootAddress, WitnessV1RejectsBech32Checksum) {
    std::vector<uint8_t> witprog(32);
    std::iota(witprog.begin(), witprog.end(), 10);

    const std::string wrongVariant = encodeWithVariant("bc", 1, witprog, bech32::Encoding::BECH32);
    ASSERT_FALSE(wrongVariant.empty());

    const auto decoded = segwit_addr::decode("bc", wrongVariant);
    ASSERT_EQ(decoded.first, -1);
    ASSERT_TRUE(decoded.second.empty());
}

TEST(TaprootAddress, WitnessV0RejectsBech32mChecksum) {
    std::vector<uint8_t> witprog(20);
    std::iota(witprog.begin(), witprog.end(), 1);

    const std::string wrongVariant = encodeWithVariant("bc", 0, witprog, bech32::Encoding::BECH32M);
    ASSERT_FALSE(wrongVariant.empty());

    const auto decoded = segwit_addr::decode("bc", wrongVariant);
    ASSERT_EQ(decoded.first, -1);
    ASSERT_TRUE(decoded.second.empty());
}
