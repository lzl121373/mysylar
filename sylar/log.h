//
// Created by lzl121373 on 2022/11/1.
//

#ifndef SYLAR_LOG_H
#define SYLAR_LOG_H

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <cstdarg>
#include "singleton.h"

#define SYLAR_LOG_LEVEL(logger, level) \
    if(logger -> getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                __FILE__, __LINE__, 0, sylar::GetThreadId(), \
                sylar::GetFiberId(), time(0)))).getSS()

#define SYLAR_LOG_DEBUG(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_ERROR(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_WARN(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_FATAL(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FORMAT_LEVEL(logger, level, fmt, ...) \
    if(logger -> getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                __FILE__, __LINE__, 0, sylar::GetThreadId(), \
                sylar::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FORMAT_DEBUG(logger, fmt, ...) SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_INFO(logger, fmt, ...) SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_ERROR(logger, fmt, ...) SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_WARN(logger, fmt, ...) SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_FATAL(logger, fmt, ...) SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

namespace sylar {

    class Logger;
    class LoggerManager;

    /**
     * @brief 日志级别
     */
    class LogLevel {
    public:
        /**
         * @brief 日志级别枚举
         */
        enum Level {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        /**
         * @brief 将日志级别转成文本输出
         * @param[in] level 日志级别
         */
        static const char* ToString(LogLevel::Level);

        /**
         * @brief 将文本转换成日志级别
         * @param[in] str 日志级别文本
         */
        static LogLevel::Level FromString(const std::string& str);
    };

    /**
     * @brief 日志事件
     */
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        /**
         * @brief Construct a new Log Event object
         * 
         * @param file 
         * @param line 
         * @param elapse 
         * @param thread_id 
         * @param fiber_id 
         * @param time 
         */
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level m_level
                , const char* file, int32_t line, uint64_t elapse
                , uint32_t thread_id, uint32_t fiber_id, uint64_t time);

        const char* getFile() const { return m_file;}

        int32_t getLine() const {
            return m_line;
        }

        uint32_t getThreadId() const {
            return m_threadId;
        }

        uint32_t getFiberId() const {
            return m_fiberId;
        }

        uint64_t getTime() const {
            return m_time;
        }

        uint64_t getElapse() const {
            return m_elapse;
        }

        std::string getContent() const {
            return m_ss.str();
        }

        std::stringstream& getSS() {return m_ss;}

        std::shared_ptr<Logger> getLogger() {return m_logger;}

        LogLevel::Level getLevel() {return m_level;}

        void format(const char* fmt, ...);

        void format(const char* fmt, va_list al);

    private:
        const char* m_file = nullptr;   //文件名
        int32_t m_line = 0;             //行号
        uint64_t m_elapse = 0;          //程序启动到现在的毫秒数
        uint32_t m_threadId = 0;        //线程ID
        uint32_t m_fiberId = 0;         //协程ID
        uint64_t m_time = 0;            //时间戳
        std::stringstream m_ss;          //内容

        std::shared_ptr<Logger> m_logger;
        LogLevel::Level m_level;
    };

    class LogEventWrap {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();
        std::stringstream& getSS();

        LogEvent::ptr getEvent() const { return m_event;}
    private:
        LogEvent::ptr m_event;
    };

    //日志格式器
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string& pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    public:
        class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();
    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
    };

    //日志输出地
    class LogAppender {
    friend class Logger;
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() {}

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        
        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;            //日志等级
        LogFormatter::ptr m_formatter;      //日志格式器
    };

    //日志器
    class Logger : public std::enable_shared_from_this<Logger> {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string& name = "root");
        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        LogLevel::Level getLevel() {return m_level;}
        void setLevel(LogLevel::Level val) {m_level = val;}
        void addAppender(LogAppender::ptr);
        void delAppender(LogAppender::ptr);
        const std::string& getName() const { return m_name;}
    private:
        std::string m_name;                             //日志名称
        LogLevel::Level m_level;                        //日志等级
        std::list<LogAppender::ptr> m_appenders;        //Appender集合
        LogFormatter::ptr m_formatter;
    };

    //输出到控制台的Appender
    class StdoutLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    };

    //输出到文件的Appender
    class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        FileLogAppender(const std::string& filename);
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

        //重新打开文件，文件打开成功返回true。
        bool reopen();
    private:
        std::string m_fileName;     //文件名
        std::ofstream m_fileStream; //文件输出流
    };

    class LoggerManager{
    public:
        LoggerManager();
        Logger::ptr getLogger(const std::string& name) const;

        void init();
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };

    typedef sylar::Singleton<LoggerManager> LoggerMgr;

}

#endif //SYLAR_LOG_H
