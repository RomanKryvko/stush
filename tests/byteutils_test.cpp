#include <array>
#include <gtest/gtest.h>
#include <byteutils.h>
#include <cstdint>

TEST (ByteutilsTest, packsBytes) {
    int32_t act = packn<int32_t>(0, 0x41, 0x5b, 0x1b);
    int32_t exp = 0b0000'01000001'01011011'00011011;
    EXPECT_EQ(act, exp);
}

TEST (ByteutilsTest, unpacksBytes) {
    auto act = unpackn(int32_t(0b0000'01000001'01011011'00011011));
    std::array<uint8_t, 4> exp {0, 0x41, 0x5b, 0x1b};
    EXPECT_EQ(act, exp);
}

TEST(ByteutilsTest, packsAllZeros) {
    int32_t act = packn<int32_t>(0, 0, 0, 0);
    int32_t exp = 0;
    EXPECT_EQ(act, exp);
}

TEST(ByteutilsTest, unpacksAllZeros) {
    auto act = unpackn(int32_t(0));
    std::array<uint8_t, 4> exp {0, 0, 0, 0};
    EXPECT_EQ(act, exp);
}

TEST(ByteutilsTest, packsAllFF) {
    int32_t act = packn<int32_t>(0xFF, 0xFF, 0xFF, 0xFF);
    int32_t exp = 0xFF'FF'FF'FF;
    EXPECT_EQ(act, exp);
}

TEST(ByteutilsTest, unpacksAllFF) {
    auto act = unpackn(int32_t(0xFF'FF'FF'FF));
    std::array<uint8_t, 4> exp {0xFF, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(act, exp);
}

TEST(ByteutilsTest, roundTrip) {
    std::array<uint8_t, 4> original {0x12, 0x34, 0x56, 0x78};
    auto packed = packn<int32_t>(original[0], original[1], original[2], original[3]);
    auto unpacked = unpackn(packed);
    EXPECT_EQ(unpacked, original);
}

TEST(ByteutilsTest, handlesPartial) {
    int32_t act = packn<int32_t>(0b00010010, 0b00110100);
    int32_t exp = 0b00010010'00110100;
    EXPECT_EQ(act, exp);
}

TEST(ByteutilsTest, unpacksString) {
    int32_t packed = packn<int32_t>('C', 'B', 'A');
    std::string act = unpack_str(packed);
    std::string exp = "ABC";
    EXPECT_EQ(act, exp);
}

TEST(ByteutilsTest, unpacksEmptyString) {
    int32_t packed = packn<int32_t>(0, 0, 0, 0);
    std::string act = unpack_str(packed);
    std::string exp = "";
    EXPECT_EQ(act, exp);
}
