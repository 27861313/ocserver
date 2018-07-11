#include "isyslog.h"
#include "iatom.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define IOCSYSLOG_LOGIDR  "./oclog"

#if IOCSYSLOG_SWITCH == 1
struct IOCSYSLOG 
{
    int8_i          _loglv;       // 日志等级
    Boolean         _logmode;     // 日志模式
    FILE*           _fd;          // 日志文件句柄
    uint64_i        _lastrotate;  // 最后一次rotate时间戳
    char            _file[128];   // 文件名
} g_iocsyslog;

const char* const g_level[] = {
    "ALL",
    "DBG",
    "INF",
    "WAR",
    "ERR",
    "FTA"
};

static Boolean iocsyslog_mkdir()
{
    DIR *dp;
    if ((dp = opendir(IOCSYSLOG_LOGIDR)) == NULL)
    {
        if(!mkdir(IOCSYSLOG_LOGIDR, S_IRWXU))
        {
            return FALSE;
        }
        return TRUE;
    }
    closedir(dp);
    return TRUE;
}
#endif

Boolean iocsyslog_init(int8_i level, int8_i mode, const char* file)
{
#if IOCSYSLOG_SWITCH == 1
    if (level > IOCSYSLOG_FTA || level < IOCSYSLOG_ALL || strlen(file) == 0)
    {
        return FALSE;
    }

    g_iocsyslog._lastrotate = (uint64_i)time(NULL);
    g_iocsyslog._loglv = level;
    g_iocsyslog._logmode = mode;
    
    if (mode & IOCSYSLOG_MODE_FILE)
    {
        size_t filelen = strlen(file);
        strncpy(g_iocsyslog._file, file, filelen > sizeof(g_iocsyslog._file) ? sizeof(g_iocsyslog._file) : filelen);
        // 如果没有日志目录, 自动创建
        if(iocsyslog_mkdir())
        {
            fprintf(stderr, "mkdir '%s' failed.\n", IOCSYSLOG_LOGIDR);
            return FALSE;
        }
        char fname[256] = { 0 };
        struct tm now;
        localtime_r((time_t *)&g_iocsyslog._lastrotate, &now);
        snprintf(
            fname,
            sizeof(fname) -1,
            "%s/%s_%d-%d-%d.log",
            IOCSYSLOG_LOGIDR,
            file,
            now.tm_year,
            now.tm_mon,
            now.tm_mday
        );
    
        g_iocsyslog._fd = fopen((const char*)fname, "w+");
        if (g_iocsyslog._fd == NULL)
        {
            fprintf(stderr, "open log file '%s' failed!\n", g_iocsyslog._file);
            return FALSE;
        }
    }
#endif
    return TRUE;
}

static Boolean isyslog_needrotate(uint64_i now)
{
#if IOCSYSLOG_SWITCH == 1
    if (!(g_iocsyslog._logmode & IOCSYSLOG_MODE_FILE))
        return FALSE;

    uint64_i last = g_iocsyslog._lastrotate;
    struct tm lsttime, nowtime;
    localtime_r((time_t *)&last, &lsttime);
    localtime_r((time_t *)&now, &nowtime);
    if (lsttime.tm_year != nowtime.tm_year || 
        lsttime.tm_mon != nowtime.tm_mon ||
        lsttime.tm_mday != nowtime.tm_mday) 
    {
        if (!(CAS(&g_iocsyslog._lastrotate, last, now)))
        {
            return FALSE;
        }
        return TRUE;
    }
#endif
    return FALSE;
}

void iocsyslog_printf(int8_i level, const char* fmt, ...)
{
#if IOCSYSLOG_SWITCH == 1
    if (level < g_iocsyslog._loglv || level > IOCSYSLOG_FTA) 
    {
        return;
    }
    uint64_i now = (uint64_i)time(NULL);
    struct tm nowtime;
    localtime_r((time_t *)&now, &nowtime);

    if ((g_iocsyslog._logmode & IOCSYSLOG_MODE_FILE) && isyslog_needrotate(now))
    {
        char fname[256] = { 0 };
        snprintf(
            fname,
            sizeof(fname) -1, 
            "%s/%s_%d-%d-%d.log",
            IOCSYSLOG_LOGIDR,
            g_iocsyslog._file,
            nowtime.tm_year,
            nowtime.tm_mon,
            nowtime.tm_mday
        );
        FILE *nfd = fopen(fname, "w+");
        if (nfd == NULL) 
        {
            fprintf(stderr, "open log file '%s' failed!\n", g_iocsyslog._file);
            return;
        }
        dup2(fileno(nfd), fileno(g_iocsyslog._fd));
        fclose(nfd);
    }

    char text[2048] = { 0 }, head[128] = { 0 };
    snprintf(
        head,
        sizeof(head) -1,
        "[%s] %d-%d-%d %d:%d:%d: ",
        g_level[level],
        nowtime.tm_year,
        nowtime.tm_mon,
        nowtime.tm_mday,
        nowtime.tm_hour,
        nowtime.tm_min,
        nowtime.tm_sec
    );

    if (g_iocsyslog._logmode & IOCSYSLOG_MODE_CONSOLE)
    {
        fprintf(stdout, "%s%s", head, text);
    }
    if (g_iocsyslog._logmode & IOCSYSLOG_MODE_FILE) 
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(text, sizeof(text)-1, fmt, args);
        va_end(args);
        fprintf(g_iocsyslog._fd, "%s%s", head, text);
    }
#endif
}

void iocsyslog_release()
{
#if IOCSYSLOG_SWITCH == 1
    if (g_iocsyslog._logmode & IOCSYSLOG_MODE_FILE)
        fclose(g_iocsyslog._fd);
#endif
}