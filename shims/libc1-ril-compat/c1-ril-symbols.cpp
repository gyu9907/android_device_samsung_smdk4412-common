// SPDX-License-Identifier: Apache-2.0
// Legacy libcutils exports required by the C1 SKT libsec-ril blob.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utils/Unicode.h>

extern "C" int android_atomic_release_cas(int32_t old_value, int32_t new_value,
                                         volatile int32_t* address) {
    // Android's legacy CAS returns zero on success, nonzero on failure.
    return __atomic_compare_exchange_n(address, &old_value, new_value, false,
                                       __ATOMIC_RELEASE, __ATOMIC_RELAXED) ? 0 : 1;
}

extern "C" char16_t* strdup8to16(const char* text, size_t* out_length) {
    if (out_length) *out_length = 0;
    if (!text) return nullptr;
    const size_t bytes = strlen(text);
    const auto* input = reinterpret_cast<const uint8_t*>(text);
    const ssize_t length = utf8_to_utf16_length(input, bytes);
    if (length < 0 || static_cast<size_t>(length) >= SIZE_MAX / sizeof(char16_t))
        return nullptr;
    auto* result = static_cast<char16_t*>(malloc((length + 1) * sizeof(char16_t)));
    if (!result) return nullptr;
    utf8_to_utf16(input, bytes, result, length + 1);
    if (out_length) *out_length = length;
    return result;
}
