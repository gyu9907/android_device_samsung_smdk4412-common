// SPDX-License-Identifier: Apache-2.0
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
extern "C" int android_atomic_release_cas(int32_t, int32_t, volatile int32_t*);
extern "C" char16_t* strdup8to16(const char*, size_t*);
int main() {
    volatile int32_t value = 7;
    assert(android_atomic_release_cas(7, 11, &value) == 0 && value == 11);
    assert(android_atomic_release_cas(7, 13, &value) != 0 && value == 11);
    size_t length = 99;
    char16_t* text = strdup8to16("A\xED\x95\x9C\xF0\x9F\x98\x80", &length);
    const char16_t expected[] = {u'A', 0xD55C, 0xD83D, 0xDE00, 0};
    assert(text && length == 4 && memcmp(text, expected, sizeof(expected)) == 0);
    free(text);
    text = strdup8to16("", &length);
    assert(text && length == 0 && text[0] == 0);
    free(text);
    assert(!strdup8to16("\xE2\x82", &length) && length == 0);
    assert(!strdup8to16(nullptr, &length) && length == 0);
    text = strdup8to16("abc", nullptr);
    assert(text && text[3] == 0);
    free(text);
}
