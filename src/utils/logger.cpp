#include "utils/logger.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

#include <iostream>

namespace incline3d::utils {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    shutdown();
}

void Logger::init(const QString& file_path, int max_size_kb) {
    QMutexLocker locker(&mutex_);

    if (initialized_) {
        shutdown();
    }

    file_path_ = file_path;
    max_size_kb_ = max_size_kb;

    // Создаём директорию если нужно
    QDir dir = QFileInfo(file_path).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    file_.setFileName(file_path);
    if (file_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        initialized_ = true;
        QTextStream stream(&file_);
        stream << "\n--- Начало сессии: "
               << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
               << " ---\n";
    }
}

void Logger::shutdown() {
    QMutexLocker locker(&mutex_);

    if (initialized_ && file_.isOpen()) {
        QTextStream stream(&file_);
        stream << "--- Конец сессии: "
               << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
               << " ---\n\n";
        file_.close();
    }
    initialized_ = false;
}

void Logger::log(LogLevel level, const QString& message, const QString& context) {
    if (level < min_level_) {
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString level_str = levelToString(level);
    QString formatted;

    if (context.isEmpty()) {
        formatted = QString("[%1] [%2] %3").arg(timestamp, level_str, message);
    } else {
        formatted = QString("[%1] [%2] [%3] %4").arg(timestamp, level_str, context, message);
    }

    // Вывод в консоль
    if (console_output_) {
        if (level >= LogLevel::kError) {
            std::cerr << formatted.toStdString() << std::endl;
        } else {
            std::cout << formatted.toStdString() << std::endl;
        }
    }

    // Запись в файл
    {
        QMutexLocker locker(&mutex_);

        if (initialized_ && file_.isOpen()) {
            rotateIfNeeded();

            QTextStream stream(&file_);
            stream << formatted << "\n";
            stream.flush();
        }
    }

    emit messageLogged(level, message);
}

void Logger::debug(const QString& message, const QString& context) {
    log(LogLevel::kDebug, message, context);
}

void Logger::info(const QString& message, const QString& context) {
    log(LogLevel::kInfo, message, context);
}

void Logger::warning(const QString& message, const QString& context) {
    log(LogLevel::kWarning, message, context);
}

void Logger::error(const QString& message, const QString& context) {
    log(LogLevel::kError, message, context);
}

void Logger::critical(const QString& message, const QString& context) {
    log(LogLevel::kCritical, message, context);
}

void Logger::setMinLevel(LogLevel level) {
    min_level_ = level;
}

void Logger::setConsoleOutput(bool enabled) {
    console_output_ = enabled;
}

void Logger::rotateIfNeeded() {
    if (!file_.isOpen()) {
        return;
    }

    qint64 size_kb = file_.size() / 1024;
    if (size_kb < max_size_kb_) {
        return;
    }

    file_.close();

    // Ротация: переименовываем старый файл
    QString backup_path = file_path_ + ".old";
    QFile::remove(backup_path);
    QFile::rename(file_path_, backup_path);

    // Открываем новый файл
    file_.setFileName(file_path_);
    file_.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream stream(&file_);
    stream << "--- Ротация лога: "
           << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
           << " ---\n";
}

QString Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::kDebug:
            return "DEBUG";
        case LogLevel::kInfo:
            return "INFO";
        case LogLevel::kWarning:
            return "WARN";
        case LogLevel::kError:
            return "ERROR";
        case LogLevel::kCritical:
            return "CRIT";
    }
    return "UNKNOWN";
}

}  // namespace incline3d::utils
