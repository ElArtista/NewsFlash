/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <string>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <time.h>

enum class LogLevel
{
    Info,
    Warn,
    Error
};

std::string LogLevelToStr(LogLevel ll);

//
// Appender typename requirements: Everything that can be called with operator()
// and takes as param a const ref to a std::string
//

template<typename Appender, typename Formatter>
class Logger
{
    private:
        /// The mutex that guarrantees the thread safety of the logger 
        std::mutex mAppenderMutex;

        /// The instance of the appender used
        Appender mAppender;

        /// The instance of the formatter used
        Formatter mFormatter;

    public:
        /// Logs given message with given info level
        void Log(LogLevel level, const std::string& msg);

        /// Logs given message with an info level
        void Info(const std::string& msg);

        /// Logs given message with an warn level
        void Warn(const std::string& msg);

        /// Logs given message with an error level
        void Error(const std::string& msg);
};

template<typename Appender, typename Formatter>
void Logger<Appender, Formatter>::Log(LogLevel level, const std::string& msg)
{
    mAppenderMutex.lock();
    mAppender(mFormatter(level, msg));
    mAppenderMutex.unlock();
}

template<typename Appender, typename Formatter>
void Logger<Appender, Formatter>::Info(const std::string& msg)
{
    return Log(LogLevel::Info, msg);
}

template<typename Appender, typename Formatter>
void Logger<Appender, Formatter>::Warn(const std::string& msg)
{
    return Log(LogLevel::Warn, msg);
}

template<typename Appender, typename Formatter>
void Logger<Appender, Formatter>::Error(const std::string& msg)
{
    return Log(LogLevel::Error, msg);
}

class SimpleFormatter
{
    public:
    std::string operator()(LogLevel, const std::string&);
};

class ConsoleAppender
{
    public:
        void operator()(const std::string&);
};

#endif
