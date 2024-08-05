#ifndef SCOPED_LOGGER_H
#define SCOPED_LOGGER_H

#include <QDebug>

// Define color codes as constexpr
constexpr char const* GREEN_COLOR = "\033[0;32m";  // Info
constexpr char const* YELLOW_COLOR = "\033[0;33m"; // Warning
constexpr char const* RED_COLOR = "\033[0;31m";    // Error
constexpr char const* RESET_COLOR = "\033[0m";

class ScopedLogger {
public:
    ScopedLogger(const char* file, int line, const char* func, const char* color)
        : stream(qDebug()) {
        stream.noquote() << color << file << "line" << line << func << ":" << RESET_COLOR;
    }

    ~ScopedLogger() {
        stream << "\n"; // Ensure that each log entry ends with a new line
    }

    // General message logging using template to handle various types
    template<typename T>
    ScopedLogger& operator<<(const T& value) {
        stream << value;
        return *this;
    }

private:
    QDebug stream;
};

#define LOG_INFO() ScopedLogger(__FILE__, __LINE__, Q_FUNC_INFO, GREEN_COLOR)
#define LOG_WARNING() ScopedLogger(__FILE__, __LINE__, Q_FUNC_INFO, YELLOW_COLOR)
#define LOG_ERROR() ScopedLogger(__FILE__, __LINE__, Q_FUNC_INFO, RED_COLOR)


#endif // SCOPED_LOGGER_H
