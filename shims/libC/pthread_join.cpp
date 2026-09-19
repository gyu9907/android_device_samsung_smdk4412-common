/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

namespace {
// Only Mali imports the capitalized entry points. Never change the process's
// target SDK or weaken Bionic's checks for other libraries. Legacy Mali can
// join a worker repeatedly during teardown. This replaces the broad Bionic
// invalid-pthread bypass used by the reference device (24502462f953).
struct MaliThread {
    pthread_t thread;
    bool joining;
    MaliThread* next;
};
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
MaliThread* threads;
}

extern "C" {

int Pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                   void* (*start)(void*), void* arg) {
    int detach_state = PTHREAD_CREATE_JOINABLE;
    if (attr != nullptr) pthread_attr_getdetachstate(attr, &detach_state);
    if (detach_state == PTHREAD_CREATE_DETACHED) {
        return pthread_create(thread, attr, start, arg);
    }
    MaliThread* entry = static_cast<MaliThread*>(malloc(sizeof(MaliThread)));
    if (entry == nullptr) return EAGAIN;
    // Publish the handle before a newly started worker can join it.
    pthread_mutex_lock(&thread_mutex);
    int result = pthread_create(thread, attr, start, arg);
    if (result == 0) {
        entry->thread = *thread;
        entry->joining = false;
        entry->next = threads;
        threads = entry;
    } else {
        free(entry);
    }
    pthread_mutex_unlock(&thread_mutex);
    return result;
}

int Pthread_join(pthread_t t, void** return_value) {
    if (t == pthread_self()) return EDEADLK;
    pthread_mutex_lock(&thread_mutex);
    MaliThread* entry = threads;
    while (entry != nullptr && entry->thread != t) entry = entry->next;
    if (entry == nullptr || entry->joining) {
        int result = entry == nullptr ? ESRCH : EINVAL;
        pthread_mutex_unlock(&thread_mutex);
        return result;
    }
    entry->joining = true;
    pthread_mutex_unlock(&thread_mutex);

    int result = pthread_join(t, return_value);
    pthread_mutex_lock(&thread_mutex);
    if (result == 0) {
        MaliThread** link = &threads;
        while (*link != entry) link = &(*link)->next;
        *link = entry->next;
        free(entry);
    } else {
        entry->joining = false;
    }
    pthread_mutex_unlock(&thread_mutex);
    return result;
}

}
