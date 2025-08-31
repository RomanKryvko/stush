#pragma once

#include <cstdint>
#include <cstdlib>
#include <stdint.h>
#include <string>
#include <utility>

template <typename T, typename... Args, std::size_t... Is>
constexpr T packn_impl(std::index_sequence<Is...>, Args... args) {
    static_assert((std::is_integral_v<Args> && ...), "All arguments must be integers.");
    static_assert(sizeof(T) >= sizeof...(Args), "Return type must be big enough to hold all arguments.");
    constexpr std::size_t N = sizeof...(Args);
    return ((T{args} << (8 * (N - 1 - Is))) | ...);
}

template <typename T, typename... Args>
constexpr T packn(Args... args) {
    return packn_impl<T>(std::index_sequence_for<Args...>{}, args...);
}

template <typename T>
constexpr std::array<uint8_t, sizeof(T)> unpackn(T code) {
    static_assert(std::is_integral_v<T>, "Code must be an integer.");
    constexpr std::size_t N = sizeof(T);
    std::array<uint8_t, N> res {};
    for (int i = 0; i < N; i++) {
        res[i] = (uint8_t)((code >> (8 * (N - i - 1))) & 0xFF);
    }
    return res;
}

template<typename T>
constexpr int highest_nonzero_byte(T x) {
    static_assert(std::is_integral_v<T>, "Code must be an integer.");
    if (x == 0) return -1;

    constexpr std::size_t N = sizeof(T);

    for (int i = N - 1; i >= 0; --i) {
        if ((x >> (i * 8)) & 0xFF)
            return i;
    }
    return -1;
}

template <typename T>
constexpr std::string unpack_str(T code) {
    static_assert(std::is_integral_v<T>, "Code must be an integer.");

    const auto highest_byte {highest_nonzero_byte(code)};
    auto res = std::string(highest_byte + 1, 0);

    for (int i = 0; i <= highest_byte; i++) {
        res[i] = (code >> (8 * i)) & 0xFF;
    }
    return res;
}
