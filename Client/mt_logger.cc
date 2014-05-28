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

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <sys/time.h>
#include <time.h>

using namespace std;

namespace mantis
{
    const string& EndColor() {static string* color = new string(MTCOLOR_PREFIX MTCOLOR_NORMAL MTCOLOR_SUFFIX); return *color;}
    const string& FatalColor() {static string* color = new string(MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_RED    MTCOLOR_SUFFIX); return *color;}
    const string& ErrorColor() {static string* color = new string(MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_RED    MTCOLOR_SUFFIX); return *color;}
    const string& WarnColor() {static string* color = new string(MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_YELLOW MTCOLOR_SUFFIX); return *color;}
    const string& InfoColor() {static string* color = new string(MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_GREEN  MTCOLOR_SUFFIX); return *color;}
    const string& DebugColor() {static string* color = new string(MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_CYAN   MTCOLOR_SUFFIX); return *color;}
    const string& OtherColor() {static string* color = new string(MTCOLOR_PREFIX MTCOLOR_BRIGHT MTCOLOR_SEPARATOR MTCOLOR_FOREGROUND_WHITE  MTCOLOR_SUFFIX); return *color;}

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
            ELevel fThreshold;

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
                    case eTrace : return DebugColor(); break;
                    case eDebug : return DebugColor(); break;
                    case eInfo  : return InfoColor(); break;
                    case eWarn  : return WarnColor(); break;
                    case eError : return ErrorColor(); break;
                    case eFatal : return FatalColor(); break;
                    default     : return OtherColor();
                }
            }


            void logCout(ELevel level, const string& message, const Location& loc)
            {
                logger::Private::sMutex.lock();
                logger::Private::getTimeAbsoluteStr();
                if (fColored)
                {
                    //cout << color << KTLogger::Private::sTimeBuff << " [" << setw(5) << level << "] " << setw(16) << left << loc.fFileName << "(" << loc.fLineNumber  << "): " << message << skKTEndColor << endl;
                    cout << Private::level2Color(level) << logger::Private::sTimeBuff << " [" << setw(5) << Private::level2Str(level) << "] ";
                    copy(loc.fFileName.end() - std::min< int >(loc.fFileName.size(), 16), loc.fFileName.end(), ostream_iterator<char>(cout));
                    cout << "(" << loc.fLineNumber  << "): ";
                    cout << message << EndColor() << endl;
                }
                else
                {
                    //cout << KTLogger::Private::sTimeBuff << " [" << setw(5) << level << "] " << setw(16) << left << loc.fFileName << "(" << loc.fLineNumber  << "): " << message << endl;
                    cout << logger::Private::sTimeBuff << " [" << setw(5) << level << "] ";
                    copy(loc.fFileName.end() - std::min< int >(loc.fFileName.size(), 16), loc.fFileName.end(), ostream_iterator<char>(cout));
                    cout << "(" << loc.fLineNumber  << "): ";
                    cout << message << endl;
                }
                logger::Private::sMutex.unlock();
            }

            void logCerr(ELevel level, const string& message, const Location& loc)
            {
                logger::Private::sMutex.lock();
                logger::Private::getTimeAbsoluteStr();
                if (fColored)
                {
                    //cout << color << KTLogger::Private::sTimeBuff << " [" << setw(5) << level << "] " << setw(16) << left << loc.fFileName << "(" << loc.fLineNumber  << "): " << message << skKTEndColor << endl;
                    cout << Private::level2Color(level) << logger::Private::sTimeBuff << " [" << setw(5) << Private::level2Str(level) << "] ";
                    copy(loc.fFileName.end() - std::min< int >(loc.fFileName.size(), 16), loc.fFileName.end(), ostream_iterator<char>(cout));
                    cout << "(" << loc.fLineNumber  << "): ";
                    cout << message << EndColor() << endl;
                }
                else
                {
                    //cout << KTLogger::Private::sTimeBuff << " [" << setw(5) << level << "] " << setw(16) << left << loc.fFileName << "(" << loc.fLineNumber  << "): " << message << endl;
                    cout << logger::Private::sTimeBuff << " [" << setw(5) << Private::level2Str(level) << "] ";
                    copy(loc.fFileName.end() - std::min< int >(loc.fFileName.size(), 16), loc.fFileName.end(), ostream_iterator<char>(cout));
                    cout << "(" << loc.fLineNumber  << "): ";
                    cout << message << endl;
                }
                logger::Private::sMutex.unlock();
            }
    };

    mutex logger::Private::sMutex;

    bool logger::Private::fColored = true;

    std::ostream* logger::Private::fOut = &cout;
    std::ostream* logger::Private::fErr = &cerr;

    char logger::Private::sDateTimeFormat[16];
    time_t logger::Private::sRawTime;
    tm* logger::Private::sProcessedTime;
    char logger::Private::sTimeBuff[512];

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
        sprintf(logger::Private::sDateTimeFormat,  "%%T");
        SetLevel(eDebug);
    }

    logger::logger(const std::string& name) : fPrivate(new Private())
    {
        fPrivate->fLogger = name.c_str();
        fPrivate->fColored = true;
        sprintf(logger::Private::sDateTimeFormat,  "%%T");
        SetLevel(eDebug);
    }

    logger::~logger()
    {
        delete fPrivate;
    }

    bool logger::IsLevelEnabled(ELevel level) const
    {
        return level >= fPrivate->fThreshold;
    }

    void logger::SetLevel(ELevel level) const
    {
#if defined(NDEBUG)
        fPrivate->fThreshold = level >= eInfo ? level : eInfo;
#else
        fPrivate->fThreshold = level;
#endif
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
        if (level >= eWarn)
        {
            fPrivate->logCerr(level, message, loc);
        }
        else
        {
            fPrivate->logCout(level, message, loc);
        }
    }
}
