#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QMutex>

namespace incline3d::utils {

/// Уровень логирования
enum class LogLevel {
    kDebug,
    kInfo,
    kWarning,
    kError,
    kCritical
};

/// Класс логирования с ротацией файлов
class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& instance();

    /// Инициализировать логгер
    void init(const QString& file_path, int max_size_kb = 1024);

    /// Завершить работу логгера
    void shutdown();

    /// Записать сообщение в лог
    void log(LogLevel level, const QString& message, const QString& context = QString());

    // Удобные методы
    void debug(const QString& message, const QString& context = QString());
    void info(const QString& message, const QString& context = QString());
    void warning(const QString& message, const QString& context = QString());
    void error(const QString& message, const QString& context = QString());
    void critical(const QString& message, const QString& context = QString());

    /// Установить минимальный уровень логирования
    void setMinLevel(LogLevel level);

    /// Включить/выключить вывод в консоль
    void setConsoleOutput(bool enabled);

signals:
    /// Сигнал о новом сообщении (для отображения в UI)
    void messageLogged(LogLevel level, const QString& message);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void rotateIfNeeded();
    QString levelToString(LogLevel level) const;

    QFile file_;
    QString file_path_;
    int max_size_kb_{1024};
    LogLevel min_level_{LogLevel::kInfo};
    bool console_output_{true};
    QMutex mutex_;
    bool initialized_{false};
};

// Макросы для удобного логирования
#define LOG_DEBUG(msg) incline3d::utils::Logger::instance().debug(msg, __FUNCTION__)
#define LOG_INFO(msg) incline3d::utils::Logger::instance().info(msg, __FUNCTION__)
#define LOG_WARNING(msg) incline3d::utils::Logger::instance().warning(msg, __FUNCTION__)
#define LOG_ERROR(msg) incline3d::utils::Logger::instance().error(msg, __FUNCTION__)
#define LOG_CRITICAL(msg) incline3d::utils::Logger::instance().critical(msg, __FUNCTION__)

}  // namespace incline3d::utils
