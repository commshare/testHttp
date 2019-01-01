/**
 * @defgroup logging Logging
 * @brief The DOMOASTER logging system.
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 * #include <LogDestination.h>
 *
 * // Call this once
 * domoaster::InitLogging(domoaster::DOMOASTER_LOG_WARN, domoaster::DOMOASTER_LOG_STDERR);
 *
 * DOMOASTER_FATAL << "Null pointer!";
 * DOMOASTER_WARN << "Could not connect to server: " << ip_address;
 * DOMOASTER_INFO << "Reading configs from " << config_dir;
 * DOMOASTER_DEBUG << "Counter was " << counter;
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * @addtogroup logging
 * @{
 *
 * @file Logging.h
 * @brief Header file for DOMOASTER Logging
 */
 
 #ifndef DOMOASTERLOG_H
#define DOMOASTERLOG_H

#include <sstream>

/**
 * Provide a stream interface to log a message at the specified log level.
 * Rather than calling this directly use one of the DOMOASTER_FATAL, DOMOASTER_WARN,
 * DOMOASTER_INFO or DOMOASTER_DEBUG macros.
 * @param level the log_level to log at.
 */
#define DOMOASTER_LOG(level) (level <= domoaster::LogLevel()) && \
                        domoaster::LogLine(__FILE__, __LINE__, level).stream()
/**
 * Provide a stream to log a fatal message. e.g.
 * @code
 *     DOMOASTER_FATAL << "Null pointer!";
 * @endcode
 */
#define DOMOASTER_FATAL DOMOASTER_LOG(domoaster::DOMOASTER_LOG_FATAL)

/**
 * Provide a stream to log a warning message.
 * @code
 *     DOMOASTER_WARN << "Could not connect to server: " << ip_address;
 * @endcode
 */
#define DOMOASTER_WARN DOMOASTER_LOG(domoaster::DOMOASTER_LOG_WARN)

/**
 * Provide a stream to log an infomational message.
 * @code
 *     DOMOASTER_INFO << "Reading configs from " << config_dir;
 * @endcode
 */
#define DOMOASTER_INFO DOMOASTER_LOG(domoaster::DOMOASTER_LOG_INFO)

/**
 * Provide a stream to log a debug message.
 * @code
 *     DOMOASTER_DEBUG << "Counter was " << counter;
 * @endcode
 */
#define DOMOASTER_DEBUG DOMOASTER_LOG(domoaster::DOMOASTER_LOG_DEBUG)

namespace domoaster {

using std::string;

/**
 * @brief The DOMOASTER log levels.
 * This controls the verbosity of logging. Each level also includes those below
 * it.
 */
enum log_level {
  DOMOASTER_LOG_NONE,   /**< No messages are logged. */
  DOMOASTER_LOG_FATAL,  /**< Fatal messages are logged. */
  DOMOASTER_LOG_WARN,   /**< Warnings messages are logged. */
  DOMOASTER_LOG_INFO,   /**< Informational messages are logged. */
  DOMOASTER_LOG_DEBUG,  /**< Debug messages are logged. */
  DOMOASTER_LOG_MAX,
};

/**
 * @private
 * @brief Application global logging level
 */
extern log_level logging_level;

/**
 * @brief The destination to write log messages to
 */
typedef enum {
  DOMOASTER_LOG_STDERR,  /**< Log to stderr. */
  DOMOASTER_LOG_SYSLOG,  /**< Log to syslog. */
  DOMOASTER_LOG_NULL,
} log_output;

/**
 * @cond HIDDEN_SYMBOLS
 * @class LogLine
 * @brief A LogLine, this represents a single log message.
 */
class LogLine {
 public:
    LogLine(const char *file, int line, log_level level);
    ~LogLine();
    void Write();

    std::ostream &stream() { return m_stream; }
 private:
    log_level m_level;
    std::ostringstream m_stream;
    unsigned int m_prefix_length;
};

/**
 * @brief Fetch the current level of logging.
 * @returns the current log_level.
 */
log_level LogLevel() ;

} ; // namespace domoaster

#endif // DOMOASTERLOG_H