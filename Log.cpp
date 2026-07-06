
/** $VER: Log.cpp (2026.07.06) P. Stuer - Another logger implementation **/

#include "pch.h"

#include "Log.h"

#pragma hdrstop

class NullLog : public ILog
{
public:
    NullLog() noexcept { };

    NullLog(const NullLog &) = delete;
    NullLog(const NullLog &&) = delete;
    NullLog & operator=(const NullLog &) = delete;
    NullLog & operator=(NullLog &&) = delete;

    virtual ~NullLog() final { };

    ILog & SetLevel(LogLevel level) noexcept override final { return *this; }
    LogLevel GetLevel() const noexcept override final { return LogLevel::Never; }

    ILog & AtFatal() noexcept override final { return *this; }
    ILog & AtError() noexcept override final { return *this; }
    ILog & AtWarn() noexcept override final { return *this; }
    ILog & AtInfo() noexcept override final { return *this; }
    ILog & AtDebug() noexcept override final { return *this; }
    ILog & AtTrace() noexcept override final { return *this; }
    ILog & Write(const char * format, ... ) noexcept override final { return *this; }
    ILog & Write(const char * format, va_list args) noexcept override final { return *this; }
};

ILog & Null = *new NullLog();

class log_impl_t : public ILog
{
public:
#ifdef _DEBUG
    log_impl_t() noexcept { SetLevel(LogLevel::Debug); }
#else
    log_impl_t() noexcept { SetLevel(LogLevel::Info); }
#endif

    log_impl_t(const log_impl_t &) = delete;
    log_impl_t(const log_impl_t &&) = delete;
    log_impl_t & operator=(const log_impl_t &) = delete;
    log_impl_t & operator=(log_impl_t &&) = delete;

    virtual ~log_impl_t() noexcept { };

    ILog & SetLevel(LogLevel level) noexcept override final
    {
        _Level = level;

        return *this;
    }

    LogLevel GetLevel() const noexcept override final
    {
        return _Level;
    }

    ILog & AtFatal() noexcept override final
    {
        return (_Level >= LogLevel::Fatal) ? *this : Null;
    }

    ILog & AtError() noexcept override final
    {
        return (_Level >= LogLevel::Error) ? *this : Null;
    }

    ILog & AtWarn() noexcept override final
    {
        return (_Level >= LogLevel::Warn) ? *this : Null;
    }

    ILog & AtInfo() noexcept override final
    {
        return (_Level >= LogLevel::Info) ? *this : Null;
    }

    ILog & AtDebug() noexcept override final
    {
        return (_Level >= LogLevel::Debug) ? *this : Null;
    }

    ILog & AtTrace() noexcept override final
    {
        return (_Level >= LogLevel::Trace) ? *this : Null;
    }

    ILog & Write(const char * format, ... ) noexcept override final
    {
        va_list args;

        va_start(args, format);

        return Write(format, args);
    }

    ILog & Write(const char * format, va_list args) noexcept override final
    {
        char Line[1024] = { };

        (void) ::vsnprintf(Line, sizeof(Line) - 1, format, args);
        console::print(Line);

        return *this;
    }

private:
    LogLevel _Level;
};

ILog & Log = *new log_impl_t();

cfg_var_modern::cfg_int CfgLogLevel({ 0x1ead47d9, 0xc4aa, 0x4079, { 0x95, 0xfa, 0xaf, 0x51, 0x6a, 0xaf, 0xd8, 0xb6 } }, LogLevel::Debug);
