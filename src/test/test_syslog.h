#ifndef OC_TEST_SYSLOG_H
#define OC_TEST_SYSLOG_H

#include "../common/ithread.h"
#include "../common/isyslog.h"



// static void *test_logthread1(void *args)
// {
//     for (int i = 0; i < 100; i++)
//     {
//         iocsyslog_printf(IOCSYSLOG_DBG, "Thread 1 count: %d\n", i);
//         ISLEEP(7);
//     }
//     return NULL;
// }

// static void *test_logthread2(void *args)
// {
//     for (int i = 0; i < 100; i++)
//     {
//         iocsyslog_printf(IOCSYSLOG_WAR, "Thread 2 count: %d\n", i);
//         ISLEEP(9);
//     }
//     return NULL;
// }

// static void *test_logthread3(void *args)
// {
//     for (int i = 0; i < 100; i++)
//     {
//         iocsyslog_printf(IOCSYSLOG_FTA, "Thread 3 count: %d\n", i);
//         ISLEEP(11);
//     }
//     return NULL;
// }

static void *test_qps(void *args)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < 10000000; i++)
    {
        iocsyslog_printf(IOCSYSLOG_ERR, "test qps.. count: %d\n", i);
    }
    gettimeofday(&end, NULL);
    uint64_i elapsed = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    fprintf(stdout, "elapsed: %llu\n", elapsed/ 1000000);
    fprintf(stdout, "qps : %llu\n", 10000000 / (elapsed / 1000000));
    return NULL;
}

void test_syslog_main()
{
    assert(iocsyslog_init(IOCSYSLOG_ALL, IOCSYSLOG_MODE_FILE, "testlog"));

    // ithread ith1, ith2, ith3;
    ithread ithqps;
    // assert(ithread_create(&ith1, NULL, test_logthread1, NULL) == 0);
    // assert(ithread_create(&ith2, NULL, test_logthread2, NULL) == 0);
    // assert(ithread_create(&ith3, NULL, test_logthread3, NULL) == 0);
    assert(ithread_create(&ithqps, NULL, test_qps, NULL) == 0);
    // ithread_join(ith1);
    // ithread_join(ith2);
    // ithread_join(ith3);
    ithread_join(ithqps);

    iocsyslog_release();
}


#endif // OC_TEST_SYSLOG_H