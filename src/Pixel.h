#ifndef PIXEL_H
#define PIXEL_H

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits> // Required for std::common_type_t

template <typename T>
struct GenericPixel {
    T r, g, b, a;

    // --- Helper (Now Public) ---
    // Made public static so global friend operators can use it safely.
    static T clamp_cast(int64_t value) {
        constexpr int64_t max_val = static_cast<int64_t>(std::numeric_limits<T>::max());
        return static_cast<T>(std::clamp(value, 0LL, max_val));
    }

    // --- Templated Compound Assignment ---
    // Allows: pixel += satPixel (clamps large values down to 8-bit)
    //         satPixel += pixel (accumulates small values into 32-bit)
    
    template <typename U>
    GenericPixel& operator+=(const GenericPixel<U>& other) {
        r = clamp_cast(static_cast<int64_t>(r) + other.r);
        g = clamp_cast(static_cast<int64_t>(g) + other.g);
        b = clamp_cast(static_cast<int64_t>(b) + other.b);
        // Note: Alpha is usually ignored in arithmetic +=
        return *this;
    }

    template <typename U>
    GenericPixel& operator-=(const GenericPixel<U>& other) {
        r = clamp_cast(static_cast<int64_t>(r) - other.r);
        g = clamp_cast(static_cast<int64_t>(g) - other.g);
        b = clamp_cast(static_cast<int64_t>(b) - other.b);
        return *this;
    }

    // --- Scalar Operators ---
    GenericPixel& operator+=(int value) {
        r = clamp_cast(static_cast<int64_t>(r) + value);
        g = clamp_cast(static_cast<int64_t>(g) + value);
        b = clamp_cast(static_cast<int64_t>(b) + value);
        return *this;
    }

    GenericPixel& operator-=(int value) {
        return *this += (-value);
    }

    GenericPixel& operator/=(int value) {
        if (value == 0) return *this;
        r /= value; g /= value; b /= value;
        return *this;
    }

    GenericPixel& operator=(int value) {
        r = g = b = a = clamp_cast(value);
        return *this;
    }
};

// --- Global Binary Operators ---
// These handle all combinations: Pixel+Pixel, SatPixel+SatPixel, and Pixel+SatPixel.

template <typename L, typename R>
auto operator+(const GenericPixel<L>& lhs, const GenericPixel<R>& rhs) {
    // Automatically pick the larger type (uint8 + uint32 -> uint32)
    using ResT = std::common_type_t<L, R>;
    using PixT = GenericPixel<ResT>;
    
    PixT res;
    // We manually assign to preserve aggregate initialization rules (no constructors)
    res.r = PixT::clamp_cast(static_cast<int64_t>(lhs.r) + rhs.r);
    res.g = PixT::clamp_cast(static_cast<int64_t>(lhs.g) + rhs.g);
    res.b = PixT::clamp_cast(static_cast<int64_t>(lhs.b) + rhs.b);
    res.a = static_cast<ResT>(lhs.a); // Preserve LHS alpha convention
    return res;
}

template <typename L, typename R>
auto operator-(const GenericPixel<L>& lhs, const GenericPixel<R>& rhs) {
    using ResT = std::common_type_t<L, R>;
    using PixT = GenericPixel<ResT>;
    
    PixT res;
    res.r = PixT::clamp_cast(static_cast<int64_t>(lhs.r) - rhs.r);
    res.g = PixT::clamp_cast(static_cast<int64_t>(lhs.g) - rhs.g);
    res.b = PixT::clamp_cast(static_cast<int64_t>(lhs.b) - rhs.b);
    res.a = static_cast<ResT>(lhs.a); 
    return res;
}

// --- Scalar Binary Operators ---
// (Pixel + int)
template <typename T>
GenericPixel<T> operator+(GenericPixel<T> lhs, int value) {
    lhs += value;
    return lhs;
}

template <typename T>
GenericPixel<T> operator-(GenericPixel<T> lhs, int value) {
    lhs -= value;
    return lhs;
}

// --- Aliases ---
using Pixel = GenericPixel<uint8_t>;
using SatPixel = GenericPixel<uint32_t>;

#endif
