/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-01     Meco Man     first Version
 * 2021-02-12     Meco Man     move all functions located in <pthread_sleep.c> to this file
 */

#include <unistd.h>
#include <rtthread.h>
#include <rthw.h>

#ifdef RT_USING_POSIX_TERMIOS
#include "termios.h"
int isatty(int fd)
{
    struct termios ts;
    return(tcgetattr(fd, &ts) != -1); /*true if no error (is a tty)*/
}
#else
int isatty(int fd)
{
    if (fd >=0 && fd < 3)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif
RTM_EXPORT(isatty);

char *ttyname(int fd)
{
    return "/dev/tty"; /* TODO: need to add more specific */
}
RTM_EXPORT(ttyname);

unsigned int sleep(unsigned int seconds)
{
    if (rt_thread_self() != RT_NULL)
    {
        rt_thread_delay(seconds * RT_TICK_PER_SECOND);
    }
    else /* scheduler has not run yet */
    {
        while(seconds > 0)
        {
            rt_hw_us_delay(1000000u);
            seconds --;
        }
    }

    return 0;
}
RTM_EXPORT(sleep);

int usleep(useconds_t usec)
{
    if (rt_thread_self() != RT_NULL)
    {
        rt_thread_mdelay(usec / 1000u);
    }
    else  /* scheduler has not run yet */
    {
        rt_hw_us_delay(usec / 1000u);
    }
    rt_hw_us_delay(usec % 1000u);

    return 0;
}
RTM_EXPORT(usleep);

pid_t gettid(void)
{
    /*TODO*/
    return 0;
}

pid_t getpid(void)
{
    return gettid();
}
RTM_EXPORT(getpid);

pid_t getppid(void)
{
    return 0;
}
RTM_EXPORT(getppid);

uid_t getuid(void)
{
    return 0; /*ROOT*/
}
RTM_EXPORT(getuid);

uid_t geteuid(void)
{
    return 0; /*ROOT*/
}
RTM_EXPORT(geteuid);

gid_t getgid(void)
{
    return 0; /*ROOT*/
}
RTM_EXPORT(getgid);

gid_t getegid(void)
{
    return 0; /*ROOT*/
}
RTM_EXPORT(getegid);
