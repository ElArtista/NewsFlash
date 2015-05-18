#include "Logger.hpp"
#include <iostream>

std::string LogLevelToStr(LogLevel ll)
{
    switch (ll)
    {
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warn:
            return "Warn";
        case LogLevel::Error:
            return "Error";
    }
    return "";
}

std::string SimpleFormatter::operator()(LogLevel level, const std::string& msg)
{
    // Get current time
    time_t t = time((time_t*)0);
#ifdef _MSC_VER
    tm lnow = { 0 };
    tm* now = &lnow;
    localtime_s(&lnow, &t);
#else
    tm* now = localtime(&t);
#endif
    
    // Use string stream to construct date
    std::stringstream ss;
    ss << std::to_string(now->tm_hour) << ":";
    ss << std::setw(2) << std::setfill('0') << std::to_string(now->tm_min) << ":";
    ss << std::setw(2) << std::setfill('0') << std::to_string(now->tm_sec);
    
    // Create the formatted string
    std::string x = "[" + LogLevelToStr(level) + "]" + "(" + ss.str() + "): " + msg;

    return std::move(x);
}

void ConsoleAppender::operator()(const std::string& msg)
{
    std::cout << msg << std::endl;
}

