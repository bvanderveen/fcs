
#ifndef __FCS_TIME
#define __FCS_TIME

int timeval_msec(struct timeval *t)
{
    return 1000 * (t->tv_usec + 1000000 * t->tv_sec)
}

int timeval_subtract(struct timeval *t2, struct timeval *t1)
{
    return timeval_msec(t2) - timeval_msec(t1);
}

#endif