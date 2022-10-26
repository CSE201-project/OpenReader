#include "logging.h"

#include <iostream>
#include <mutex>

static LogLevel s_logLevel = Normal;

void LogSetLevel(LogLevel level)
{
    s_logLevel = level;
}

void LogStrDebug(const char *pText)
{
    LogDebug << pText;
}

void LogStrInfo(const char *pText)
{
    LogInfo << pText;
}
void LogStrStatus(const char *pText)
{
    LogStatus << pText;
}

void LogStrWarn(const char *pText)
{
    LogWarn << pText;
}

void LogStrError(const char *pText)
{
    LogError << pText;
}

void LogStrFatal(const char *pText)
{
    LogFatal << pText;
}

void LogStrAlways(const char *pText)
{
    LogAlways << pText;
}

void LogStrTrace(const char *pText)
{
    LogTrace << pText;
}

SyncStream *LogStream::GetSyncStreamDebug()
{
    if (s_logLevel <= Debug)
    {
        SyncStream *stream = new sout;
        *stream << "  DEBUG: ";
        return stream;
    }
    return nullptr;
}

SyncStream *LogStream::GetSyncStreamInfo()
{
    if (s_logLevel <= Verbose)
    {
        SyncStream *stream = new sout;
        *stream << "   INFO: ";
        return stream;
    }
    return nullptr;
}

SyncStream *LogStream::GetSyncStreamStatus()
{
    if (s_logLevel <= Normal)
    {
        SyncStream *stream = new sout;
        *stream << " STATUS: ";
        return stream;
    }
    return nullptr;
}

SyncStream *LogStream::GetSyncStreamWarn()
{
    SyncStream *stream = new sout;
    *stream << "WARNING: ";
    return stream;
}

SyncStream *LogStream::GetSyncStreamError()
{
    SyncStream *stream = new sout;
    *stream << "!!ERROR: ";
    return stream;
}

SyncStream *LogStream::GetSyncStreamFatal()
{
    SyncStream *stream = new sout;
    *stream << "**FATAL: ";
    return stream;
}

SyncStream *LogStream::GetSyncStreamAlways()
{
    SyncStream *stream = new sout;
    *stream << "..Log..: ";
    return stream;
}

SyncStream *LogStream::GetSyncStreamTrace()
{
    SyncStream *stream = new sout;
    *stream << "-Trace-: ";
    return stream;
}
