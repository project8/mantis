/*
 * mt_logger.cc
 *
 *  Created on: Jan 21, 2014
 *      Author: nsoblath
 */

/*
 * mt_logger.cxx
 * based on KLogger.cxx from KATRIN's Kasper
 *
 *  Created on: 18.11.2011
 *      Author: Marco Haag <marco.haag@kit.edu>
 */

#include "mt_logger.hh"

#include "mt_mutex.hh"

#include <cstdlib>
#include <cstring>

using namespace std;

static const string skMTEndColor =   MTCOLOR_PREFIX MTCOLOR_NORMAL MTCOLOR_SUFFIX;
static const string skMTFatalColor = MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_RED    MTCOLOR_SUFFIX;
static const string skMTErrorColor = MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_RED    MTCOLOR_SUFFIX;
static const string skMTWarnColor =  MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_YELLOW MTCOLOR_SUFFIX;
static const string skMTInfoColor =  MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_GREEN  MTCOLOR_SUFFIX;
static const string skMTDebugColor = MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_CYAN   MTCOLOR_SUFFIX;
static const string skMTOtherColor = MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_WHITE  MTCOLOR_SUFFIX;


#if defined(LOG4CXX_FOUND)

/*
 * Default implementation for systems with the 'log4cxx' library installed.
 */

#include <log4cxx/logstring.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/level.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/logger.h>

using namespace log4cxx;

#ifndef MT_LOG4CXX_COLORED_PATTERN_LAYOUT_H
#define MT_LOG4CXX_COLORED_PATTERN_LAYOUT_H

namespace log4cxx
{

    class LOG4CXX_EXPORT MTColoredPatternLayout : public PatternLayout
    {
        public:
            DECLARE_LOG4CXX_OBJECT(MTColoredPatternLayout)
            BEGIN_LOG4CXX_CAST_MAP()
            LOG4CXX_CAST_ENTRY(MTColoredPatternLayout)
            LOG4CXX_CAST_ENTRY_CHAIN(Layout)
            END_LOG4CXX_CAST_MAP()

            MTColoredPatternLayout() : PatternLayout() {}
            MTColoredPatternLayout(const LogString& pattern) : PatternLayout(pattern) {};
            virtual ~MTColoredPatternLayout() {}

        protected:
            virtual void format(LogString& output, const spi::LoggingEventPtr& event, helpers::Pool& pool) const;
            virtual std::string getColor(const LevelPtr& level) const;

    };

    LOG4CXX_PTR_DEF(MTColoredPatternLayout);

}

#endif /* MT_LOG4CXX_COLORED_PATTERN_LAYOUT_H */

IMPLEMENT_LOG4CXX_OBJECT(MTColoredPatternLayout)

void MTColoredPatternLayout::format(LogString& output, const spi::LoggingEventPtr& event, helpers::Pool& pool) const
{
    PatternLayout::format(output, event, pool);
    output = getColor(event->getLevel()) + output + skMTEndColor;
    return;
}

string MTColoredPatternLayout::getColor(const LevelPtr& level) const
{
    switch(level->toInt())
    {
        case Level::FATAL_INT:
            return skMTFatalColor;
            break;
        case Level::ERROR_INT:
            return skMTErrorColor;
            break;
        case Level::WARN_INT:
            return skMTWarnColor;
            break;
        case Level::INFO_INT:
            return skMTInfoColor;
            break;
        case Level::DEBUG_INT:
            return skMTDebugColor;
            break;
        case Level::TRACE_INT:
            return skMTDebugColor;
            break;
        default:
            return skMTOtherColor;
    }
}

namespace
{

    struct MTStaticInitializer {
            MTStaticInitializer()
            {

                ///if (LogManager::getLoggerRepository()->isConfigured())
                ///    return;
                //        AppenderList appenders = Logger::getRootLogger()->getAllAppenders();

                char* envLoggerConfig;
                envLoggerConfig = getenv("LOGGER_CONFIGURATION");
                if (envLoggerConfig != 0)
                {
                    PropertyConfigurator::configure(envLoggerConfig);
                }
                else {
#ifdef LOGGER_CONFIGURATION
                    PropertyConfigurator::configure(LOGGER_CONFIGURATION);
#else
                    LogManager::getLoggerRepository()->setConfigured(true);
                    LoggerPtr root = Logger::getRootLogger();
#ifdef NDEBUG
                    Logger::getRootLogger()->setLevel(Level::getInfo());
#endif
                    static const LogString TTCC_CONVERSION_PATTERN(LOG4CXX_STR("%r [%-5p] %16c: %m%n"));
                    //LayoutPtr layout(new PatternLayout(TTCC_CONVERSION_PATTERN));
                    LayoutPtr layout(new MTColoredPatternLayout(TTCC_CONVERSION_PATTERN));
                    AppenderPtr appender(new ConsoleAppender(layout));
                    root->addAppender(appender);
#endif
                }

            }
    } static sMTLoggerInitializer;

}

namespace mantis
{
    struct logger::Private
    {
            void log(const LevelPtr& level, const string& message, const Location& loc)
            {
                fLogger->forcedLog(level, message, ::log4cxx::spi::LocationInfo(loc.fFileName, loc.fFunctionName, loc.fLineNumber));
            }

            static LevelPtr level2Ptr(ELevel level)
            {
                switch(level)
                {
                    case eTrace : return Level::getTrace();
                    case eDebug : return Level::getDebug();
                    case eInfo  : return Level::getInfo();
                    case eWarn  : return Level::getWarn();
                    case eError : return Level::getError();
                    case eFatal : return Level::getFatal();
                    default     : return Level::getOff();
                }
            }

            LoggerPtr fLogger;
    };

    logger::logger(const char* name) : fPrivate(new Private())
    {
        fPrivate->fLogger = (name == 0) ? Logger::getRootLogger() : Logger::getLogger(name);
    }

    logger::logger(const std::string& name) : fPrivate(new Private())
    {
        fPrivate->fLogger = Logger::getLogger(name);
    }

    logger::~logger()
    {
        delete fPrivate;
    }

    bool logger::IsLevelEnabled(ELevel level) const
    {
        return fPrivate->fLogger->isEnabledFor( Private::level2Ptr(level) );
    }

    void logger::SetLevel(ELevel level) const
    {
        fPrivate->fLogger->setLevel( Private::level2Ptr(level) );
    }

    void logger::Log(ELevel level, const string& message, const Location& loc)
    {
        fPrivate->log(Private::level2Ptr(level), message, loc);
    }

} /* namespace mantis */


#else

/**
 * Fallback solution for systems without log4cxx.
 */

#include <cstdio>
#include <iomanip>
#include <sys/time.h>
#include <time.h>

namespace mantis
{
    struct logger::Private
    {
            static mutex sMutex;

            static char sDateTimeFormat[16];
            static time_t sRawTime;
            static tm* sProcessedTime;
            static char sTimeBuff[512];
            static size_t getTimeAbsoluteStr()
            {
                time(&logger::Private::sRawTime);
                sProcessedTime = gmtime(&logger::Private::sRawTime);
                return strftime(logger::Private::sTimeBuff, 512,
                        logger::Private::sDateTimeFormat,
                        logger::Private::sProcessedTime);
            }


            const char* fLogger;

            static bool fColored;

            static std::ostream* fOut;
            static std::ostream* fErr;

            static const char* level2Str(ELevel level)
            {
                switch(level)
                {
                    case eTrace : return "TRACE"; break;
                    case eDebug : return "DEBUG"; break;
                    case eInfo  : return "INFO"; break;
                    case eWarn  : return "WARN"; break;
                    case eError : return "ERROR"; break;
                    case eFatal : return "FATAL"; break;
                    default     : return "XXX";
                }
            }

            static string level2Color(ELevel level)
            {
                switch(level)
                {
                    case eTrace : return skMTDebugColor; break;
                    case eDebug : return skMTDebugColor; break;
                    case eInfo  : return skMTInfoColor; break;
                    case eWarn  : return skMTWarnColor; break;
                    case eError : return skMTErrorColor; break;
                    case eFatal : return skMTFatalColor; break;
                    default     : return skMTOtherColor;
                }
            }


            void logCout(const char* level, const string& message, const Location& /*loc*/, const string& color = skMTOtherColor)
            {
                logger::Private::sMutex.lock();
                logger::Private::getTimeAbsoluteStr();
                if (logger::Private::fColored)
                    (*fOut) << color << logger::Private::sTimeBuff << " [" << setw(5) << level << "] " << setw(16) << fLogger << ": " << message << skMTEndColor << endl;
                else
                    (*fOut) << logger::Private::sTimeBuff << " [" << setw(5) << level << "] " << setw(16) << fLogger << ": " << message << endl;
                logger::Private::sMutex.unlock();
            }

            void logCerr(const char* level, const string& message, const Location& /*loc*/, const string& color = skMTOtherColor)
            {
                logger::Private::sMutex.lock();
                logger::Private::getTimeAbsoluteStr();
                if (logger::Private::fColored)
                    (*fErr) << color << logger::Private::sTimeBuff << " [" << setw(5) << level << "] " << setw(16) << fLogger << ": " << message << skMTEndColor << endl;
                else
                    (*fErr) << logger::Private::sTimeBuff <<  " [" << setw(5) << level << "] " << setw(16) << fLogger << ": " << message << endl;
                logger::Private::sMutex.unlock();
            }
    };

    mutex logger::Private::sMutex;

    char logger::Private::sDateTimeFormat[16];
    time_t logger::Private::sRawTime;
    tm* logger::Private::sProcessedTime;
    char logger::Private::sTimeBuff[512];

    bool logger::Private::fColored = true;

    std::ostream* logger::Private::fOut = &cout;
    std::ostream* logger::Private::fErr = &cerr;


    logger::logger(const char* name) : fPrivate(new Private())
    {
        if (name == 0)
        {
            fPrivate->fLogger = "root";
        }
        else
        {
            const char* logName = strrchr(name, '/') ? strrchr(name, '/') + 1 : name;
            fPrivate->fLogger = logName;
        }
        fPrivate->fColored = true;
        sprintf(logger::Private::sDateTimeFormat,  "%%FT%%TZ");
    }

    logger::logger(const std::string& name) : fPrivate(new Private())
    {
        fPrivate->fLogger = name.c_str();
        fPrivate->fColored = true;
        sprintf(logger::Private::sDateTimeFormat,  "%%FT%%TZ");
    }

    logger::~logger()
    {
        delete fPrivate;
    }

    bool logger::IsLevelEnabled(ELevel level) const
    {
#ifdef NDEBUG
        return level > eDebug;
#else
        (void) level;
        return true;
#endif
    }

    void logger::SetLevel(ELevel /*level*/) const
    {
        // TODO: implement levels for fallback logger
    }

    void logger::SetColored(bool flag)
    {
        logger::Private::fColored = flag;
        return;
    }

    void logger::SetOutStream(std::ostream* stream)
    {
        logger::Private::fOut = stream;
        return;
    }

    void logger::SetErrStream(std::ostream* stream)
    {
        logger::Private::fErr = stream;
        return;
    }

    void logger::Log(ELevel level, const string& message, const Location& loc)
    {
        const char* levelStr = Private::level2Str(level);

        if (level >= eWarn)
        {
            fPrivate->logCerr(levelStr, message, loc, Private::level2Color(level));
        }
        else
        {
            fPrivate->logCout(levelStr, message, loc, Private::level2Color(level));
        }
    }
}

#endif
