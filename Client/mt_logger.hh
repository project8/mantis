/*
 * mt_logger.hh
 * Based on KLogger.h, from KATRIN's Kasper
 *
 *  Created on: Jan 21, 2014
 *      Author: nsoblath
 */

#ifndef MTLOGGER_HH_
#define MTLOGGER_HH_

/**
 * @file
 * @brief Contains the logger class and macros, based on Kasper's KLogger class.
 * @date Created on: 18.11.2011
 * @author Marco Haag <marco.haag@kit.edu>
 *
 */

// UTILITY MACROS

#ifndef LOGGER_UTILITY_MACROS_
#define LOGGER_UTILITY_MACROS_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __FILE_LINE__      __FILE__ "(" TOSTRING(__LINE__) ")"
#define __FILENAME_LINE__  (strrchr(__FILE__, '/') ? strrchr(__FILE_LINE__, '/') + 1 : __FILE_LINE__)

#if defined(_MSC_VER)
#if _MSC_VER >= 1300
#define __FUNC__ __FUNCSIG__
#endif
#else
#if defined(__GNUC__)
#define __FUNC__ __PRETTY_FUNCTION__
#endif
#endif
#if !defined(__FUNC__)
#define __FUNC__ ""
#endif

#define va_num_args(...) va_num_args_impl(__VA_ARGS__, 5,4,3,2,1)
#define va_num_args_impl(_1,_2,_3,_4,_5,N,...) N

#define macro_dispatcher(func, ...) macro_dispatcher_(func, va_num_args(__VA_ARGS__))
#define macro_dispatcher_(func, nargs) macro_dispatcher__(func, nargs)
#define macro_dispatcher__(func, nargs) func ## nargs

#endif  /* LOGGER_UTILITY_MACROS_ */

// COLOR DEFINITIONS
#define MTCOLOR_NORMAL "0"
#define MTCOLOR_BRIGHT "1"
#define MTCOLOR_FOREGROUND_RED "31"
#define MTCOLOR_FOREGROUND_GREEN "32"
#define MTCOLOR_FOREGROUND_YELLOW "33"
#define MTCOLOR_FOREGROUND_CYAN "36"
#define MTCOLOR_FOREGROUND_WHITE "37"
#define MTCOLOR_PREFIX "\033["
#define MTCOLOR_SUFFIX "m"
#define MTCOLOR_SEPARATOR ";"

// INCLUDES

#include <string>
#include <iostream>
#include <sstream>

// CLASS DEFINITIONS

/**
 * The standard mantis namespace.
 */
namespace mantis
{

    /**
     * The mantis logger.
     *
     * The usage and syntax is inspired by log4j. logger itself uses the log4cxx library if it
     * was available on the system during compiling, otherwise it falls back to std::stream.
     *
     * The logger output can be configured in a file specified with the environment variable
     * @a LOGGER_CONFIGURATION (by default log4cxx.properties in the config directory).
     *
     * In most cases the following macro can be used
     * to instantiate a Logger in your code:
     * <pre>LOGGER(myLogger, "loggerName");</pre>
     *
     * This is equivalent to:
     * <pre>static mantis::logger myLogger("loggerName");</pre>
     *
     * For logging the following macros can be used. The source code location will then automatically
     * included in the output:
     *
     * <pre>
     * LOG(myLogger, level, "message");
     * TRACE(myLogger, "message");
     * DEBUG(myLogger, "message");
     * INFO(myLogger, "message");
     * WARN(myLogger, "message");
     * ERROR(myLogger, "message");
     * FATAL(myLogger, "message");
     *
     * ASSERT(myLogger, assertion, "message");
     *
     * LOG_ONCE(myLogger, level, "message");
     * TRACE_ONCE(myLogger, "message");
     * DEBUG_ONCE(myLogger, "message");
     * INFO_ONCE(myLogger, "message");
     * WARN_ONCE(myLogger, "message");
     * ERROR_ONCE(myLogger, "message");
     * FATAL_ONCE(myLogger, "message");
     * </pre>
     *
     */
    class logger
    {
        public:
            enum ELevel {
                eTrace, eDebug, eInfo, eWarn, eError, eFatal
            };

        public:
            /**
             * A simple struct used by the Logger macros to pass information about the filename and line number.
             * Not to be used directly by the user!
             */
            struct Location {
                Location(const char* const fileName = "", const char* const functionName = "", int lineNumber = -1) :
                    fLineNumber(lineNumber), fFileName(fileName), fFunctionName(functionName)
                { }
                int fLineNumber;
                std::string fFileName;
                std::string fFunctionName;
            };

        public:
            static logger& GetRootLogger() {
                static logger rootLogger;
                return rootLogger;
            }

        public:
            /**
             * Standard constructor assigning a name to the logger instance.
             * @param name The logger name.
             */
            logger(const char* name = 0);
            /// @overload
            logger(const std::string& name);

            virtual ~logger();

            /**
             * Check whether a certain log-level is enabled.
             * @param level The log level as string representation.
             * @return
             */
            bool IsLevelEnabled(ELevel level) const;

            /**
             * Set a loggers minimum logging level
             * @param level string identifying the log level
             */
            void SetLevel(ELevel level) const;

            /**
             * Set whether colored text will be used
             * @param flag Bool determining whether colored text will be used
             */
            static void SetColored(bool flag);

            /**
             * Set the ostream pointer used for standard output messages
             * @param stream Stream object for standard output
             */
            static void SetOutStream(std::ostream* stream);

            /**
             * Set the ostream pointer used for standard error messages
             * @param stream Stream object for standard errors
             */
            static void SetErrStream(std::ostream* stream);

            /**
             * Log a message with the specified level.
             * Use the macro LOG(logger, level, message).
             * @param level The log level.
             * @param message The message.
             * @param loc Source code location (set automatically by the corresponding macro).
             */
            void Log(ELevel level, const std::string& message, const Location& loc = Location());

            /**
             * Log a message at TRACE level.
             * Use the macro TRACE(logger, message).
             * @param message The message.
             * @param loc Source code location (set automatically by the corresponding macro).
             */
            void LogTrace(const std::string& message, const Location& loc = Location())
            {
                Log(eTrace, message, loc);
            }
            /**
             * Log a message at DEBUG level.
             * Use the macro DEBUG(logger, message).
             * @param message The message.
             * @param loc Source code location (set automatically by the corresponding macro).
             */
            void LogDebug(const std::string& message, const Location& loc = Location())
            {
                Log(eDebug, message, loc);
            }
            /**
             * Log a message at DEBUG level.
             * Use the macro DEBUG(logger, message).
             * @param message The message.
             * @param loc Source code location (set automatically by the corresponding macro).
             */
            void LogInfo(const std::string& message, const Location& loc = Location())
            {
                Log(eInfo, message, loc);
            }
            /**
             * Log a message at INFO level.
             * Use the macro INFO(logger, message).
             * @param message The message.
             * @param loc Source code location (set automatically by the corresponding macro).
             */
            void LogWarn(const std::string& message, const Location& loc = Location())
            {
                Log(eWarn, message, loc);
            }
            /**
             * Log a message at ERROR level.
             * Use the macro ERROR(logger, message).
             * @param message The message.
             * @param loc Source code location (set automatically by the corresponding macro).
             */
            void LogError(const std::string& message, const Location& loc = Location())
            {
                Log(eError, message, loc);
            }
            /**
             * Log a message at FATAL level.
             * Use the macro FATAL(logger, message).
             * @param message The message.
             * @param loc Source code location (set automatically by the corresponding macro).
             */
            void LogFatal(const std::string& message, const Location& loc = Location())
            {
                Log(eFatal, message, loc);
            }

        private:
            struct Private;
            Private* fPrivate;
    };

}

// PRIVATE MACROS

#define __MTDEFAULT_LOGGER        mantis::logger::GetRootLogger()

#define __MTLOG_LOCATION         mantis::logger::Location(__FILE__, __FUNC__, __LINE__)

#define __MTLOG_LOG_4(I,L,M,O) \
        { \
    if (I.IsLevelEnabled(mantis::logger::e##L)) { \
        static bool _sLoggerMarker = false; \
        if (!O || !_sLoggerMarker) { \
            _sLoggerMarker = true; \
            std::ostringstream stream; stream << M; \
            I.Log(mantis::logger::e##L, stream.str(), __MTLOG_LOCATION); \
        } \
    } \
        }

#define __MTLOG_LOG_3(I,L,M)     __MTLOG_LOG_4(I,L,M,false)
#define __MTLOG_LOG_2(L,M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,L,M,false)
#define __MTLOG_LOG_1(M)         __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Debug,M,false)

#define __MTLOG_TRACE_2(I,M)     __MTLOG_LOG_4(I,Trace,M,false)
#define __MTLOG_TRACE_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Trace,M,false)

#define __MTLOG_DEBUG_2(I,M)     __MTLOG_LOG_4(I,Debug,M,false)
#define __MTLOG_DEBUG_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Debug,M,false)

#define __MTLOG_INFO_2(I,M)      __MTLOG_LOG_4(I,Info,M,false)
#define __MTLOG_INFO_1(M)        __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Info,M,false)

#define __MTLOG_WARN_2(I,M)      __MTLOG_LOG_4(I,Warn,M,false)
#define __MTLOG_WARN_1(M)        __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Warn,M,false)

#define __MTLOG_ERROR_2(I,M)     __MTLOG_LOG_4(I,Error,M,false)
#define __MTLOG_ERROR_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Error,M,false)

#define __MTLOG_FATAL_2(I,M)     __MTLOG_LOG_4(I,Fatal,M,false)
#define __MTLOG_FATAL_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Fatal,M,false)

#define __MTLOG_ASSERT_3(I,C,M)  if (!(C)) { __MTLOG_ERROR_2(I,M) }
#define __MTLOG_ASSERT_2(C,M)    __MTLOG_ASSERT_3(__MTDEFAULT_LOGGER,C,M)


#define __MTLOG_LOG_ONCE_3(I,L,M)     __MTLOG_LOG_4(I,L,M,true)
#define __MTLOG_LOG_ONCE_2(L,M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,L,M,true)
#define __MTLOG_LOG_ONCE_1(M)         __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Debug,M,true)

#define __MTLOG_TRACE_ONCE_2(I,M)     __MTLOG_LOG_4(I,Trace,M,true)
#define __MTLOG_TRACE_ONCE_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Trace,M,true)

#define __MTLOG_DEBUG_ONCE_2(I,M)     __MTLOG_LOG_4(I,Debug,M,true)
#define __MTLOG_DEBUG_ONCE_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Debug,M,true)

#define __MTLOG_INFO_ONCE_2(I,M)      __MTLOG_LOG_4(I,Info,M,true)
#define __MTLOG_INFO_ONCE_1(M)        __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Info,M,true)

#define __MTLOG_WARN_ONCE_2(I,M)      __MTLOG_LOG_4(I,Warn,M,true)
#define __MTLOG_WARN_ONCE_1(M)        __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Warn,M,true)

#define __MTLOG_ERROR_ONCE_2(I,M)     __MTLOG_LOG_4(I,Error,M,true)
#define __MTLOG_ERROR_ONCE_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Error,M,true)

#define __MTLOG_FATAL_ONCE_2(I,M)     __MTLOG_LOG_4(I,Fatal,M,true)
#define __MTLOG_FATAL_ONCE_1(M)       __MTLOG_LOG_4(__MTDEFAULT_LOGGER,Fatal,M,true)


// PUBLIC MACROS

#define MTLOGGER(I,K)      static mantis::logger I(K);

#define MTLOG(...)         macro_dispatcher(__MTLOG_LOG_, __VA_ARGS__)(__VA_ARGS__)
#define MTTRACE(...)       macro_dispatcher(__MTLOG_TRACE_, __VA_ARGS__)(__VA_ARGS__)
#define MTDEBUG(...)       macro_dispatcher(__MTLOG_DEBUG_, __VA_ARGS__)(__VA_ARGS__)
#define MTINFO(...)        macro_dispatcher(__MTLOG_INFO_, __VA_ARGS__)(__VA_ARGS__)
#define MTWARN(...)        macro_dispatcher(__MTLOG_WARN_, __VA_ARGS__)(__VA_ARGS__)
#define MTERROR(...)       macro_dispatcher(__MTLOG_ERROR_, __VA_ARGS__)(__VA_ARGS__)
#define MTFATAL(...)       macro_dispatcher(__MTLOG_FATAL_, __VA_ARGS__)(__VA_ARGS__)
#define MTASSERT(...)      macro_dispatcher(__MTLOG_ASSERT_, __VA_ARGS__)(__VA_ARGS__)

#define MTLOG_ONCE(...)    macro_dispatcher(__MTLOG_LOG_ONCE_, __VA_ARGS__)(__VA_ARGS__)
#define MTTRACE_ONCE(...)  macro_dispatcher(__MTLOG_TRACE_ONCE_, __VA_ARGS__)(__VA_ARGS__)
#define MTDEBUG_ONCE(...)  macro_dispatcher(__MTLOG_DEBUG_ONCE_, __VA_ARGS__)(__VA_ARGS__)
#define MTINFO_ONCE(...)   macro_dispatcher(__MTLOG_INFO_ONCE_, __VA_ARGS__)(__VA_ARGS__)
#define MTWARN_ONCE(...)   macro_dispatcher(__MTLOG_WARN_ONCE_, __VA_ARGS__)(__VA_ARGS__)
#define MTERROR_ONCE(...)  macro_dispatcher(__MTLOG_ERROR_ONCE_, __VA_ARGS__)(__VA_ARGS__)
#define MTFATAL_ONCE(...)  macro_dispatcher(__MTLOG_FATAL_ONCE_, __VA_ARGS__)(__VA_ARGS__)

#endif /* MTLOGGER_H_ */
