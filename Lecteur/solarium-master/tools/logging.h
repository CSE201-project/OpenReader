#pragma once

#include "sync_stream.h"

enum LogLevel
{
    Debug,
    Verbose,
    Normal,
    ErrorsOnly
};

// Set new log level (default: Normal)
void LogSetLevel(LogLevel level);

// Our full set of logging methods (we just log to stdout)
//
// NOTE: Some methods will only log if the appropriate `logLevel` is set
void LogStrDebug(const char *pText);
void LogStrInfo(const char *pText);
void LogStrStatus(const char *pText);
void LogStrWarn(const char *pText);
void LogStrError(const char *pText);
void LogStrFatal(const char *pText);
void LogStrAlways(const char *pText);
void LogStrTrace(const char *pText);

// Streaming version
#define LogDebug    LogStream(LogStream::GetSyncStreamDebug())
#define LogInfo     LogStream(LogStream::GetSyncStreamInfo())
#define LogStatus   LogStream(LogStream::GetSyncStreamStatus())
#define LogWarn     LogStream(LogStream::GetSyncStreamWarn())
#define LogError    LogStream(LogStream::GetSyncStreamError())
#define LogFatal    LogStream(LogStream::GetSyncStreamFatal())
#define LogAlways   LogStream(LogStream::GetSyncStreamAlways())
#define LogTrace    LogStream(LogStream::GetSyncStreamTrace())

class LogStream
{
public:
    static SyncStream *GetSyncStreamDebug();
    static SyncStream *GetSyncStreamInfo();
    static SyncStream *GetSyncStreamStatus();
    static SyncStream *GetSyncStreamWarn();
    static SyncStream *GetSyncStreamError();
    static SyncStream *GetSyncStreamFatal();
    static SyncStream *GetSyncStreamAlways();
    static SyncStream *GetSyncStreamTrace();

    LogStream(SyncStream *stream) : m_stream(stream) {}
    ~LogStream()
    {
        if (m_stream != nullptr)
        {
            delete m_stream;
        }
    }

    template <typename T>
    LogStream &operator<<(T const &obj)
    {
        if (m_stream != nullptr)
        {
            *m_stream << obj;
        }
        return *this;
    }

    LogStream(const LogStream &other) = delete;
    LogStream(LogStream &&other) = delete;

private:
    SyncStream *m_stream;
};

