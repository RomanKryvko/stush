#pragma once

#include <stdint.h>
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

