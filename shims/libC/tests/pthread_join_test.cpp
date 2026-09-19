// SPDX-License-Identifier: Apache-2.0
// Standalone host test: compile this file with ../pthread_join.cpp and -pthread.
#include <pthread.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <atomic>
#include <thread>
#include <unistd.h>
extern "C" int Pthread_create(pthread_t*, const pthread_attr_t*, void*(*)(void*),void*);
extern "C" int Pthread_join(pthread_t,void**);
static void* work(void* p) { return p; }
static std::atomic<bool> workerRelease;
static void* blockedWorker(void*) {
    while (!workerRelease.load()) std::this_thread::yield();
    return nullptr;
}
int main() {
    alarm(20);
    pthread_t t; void* result = nullptr;
    for (int i=0;i<1000;i++) {
        assert(Pthread_create(&t,nullptr,work,(void*)42)==0);
        assert(Pthread_join(t,&result)==0 && result==(void*)42);
        assert(Pthread_join(t,nullptr)==ESRCH);
    }
    assert(Pthread_join((pthread_t)1234,nullptr)==ESRCH);
    assert(Pthread_join(pthread_self(),nullptr)==EDEADLK);
    pthread_attr_t attr; pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    assert(Pthread_create(&t,&attr,work,nullptr)==0);
    assert(Pthread_join(t,nullptr)==ESRCH);
    pthread_attr_destroy(&attr);
    for (int i = 0; i < 100; ++i) {
        workerRelease.store(false);
        assert(Pthread_create(&t, nullptr, blockedWorker, nullptr) == 0);
        std::atomic<int> finished{0};
        int r1 = -1, r2 = -1;
        std::thread first([&] { r1 = Pthread_join(t, nullptr); ++finished; });
        std::thread second([&] { r2 = Pthread_join(t, nullptr); ++finished; });
        while (!finished.load()) std::this_thread::yield();
        workerRelease.store(true);
        first.join(); second.join();
        assert((r1 == 0 && r2 == EINVAL) || (r2 == 0 && r1 == EINVAL));
    }
    alarm(0);
    puts("PASS: 1000 valid/duplicate joins, return values, unknown handles, self join, detached thread, 100 concurrent join races");
}
