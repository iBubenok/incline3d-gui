#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>
#include <QMessageBox>
#include <QDir>
#include <QLibraryInfo>

#include "ui/main_window.h"
#include "core/settings.h"
#include "utils/logger.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Настройка метаданных приложения
    QApplication::setApplicationName("Incline3D");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName(QString::fromUtf8("ПримГео"));
    QApplication::setOrganizationDomain("primegeo.ru");

    // Инициализация логгера
    QString logPath = QDir::homePath() + "/.incline3d/logs";
    QDir().mkpath(logPath);
    incline3d::utils::Logger::instance().init(logPath + "/incline3d.log");
    incline3d::utils::Logger::instance().setMinLevel(incline3d::utils::LogLevel::kInfo);

    LOG_INFO(QString::fromUtf8("Запуск приложения Incline3D v") +
             QApplication::applicationVersion());

    // Загрузка настроек
    auto& settings = incline3d::core::Settings::instance();
    Q_UNUSED(settings)

    // Установка стиля оформления
    if (QStyleFactory::keys().contains("Fusion")) {
        app.setStyle(QStyleFactory::create("Fusion"));
    }

    // Загрузка переводов Qt
    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale::system(), "qt", "_",
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    // Создание и отображение главного окна
    incline3d::ui::MainWindow mainWindow;
    mainWindow.show();

    // Обработка аргументов командной строки
    QStringList args = app.arguments();
    if (args.size() > 1) {
        QString filePath = args.at(1);
        if (filePath.endsWith(".inclproj", Qt::CaseInsensitive)) {
            // Открыть проект
            mainWindow.openProject(filePath);
        } else if (filePath.endsWith(".ws", Qt::CaseInsensitive) ||
                   filePath.endsWith(".csv", Qt::CaseInsensitive) ||
                   filePath.endsWith(".las", Qt::CaseInsensitive) ||
                   filePath.endsWith(".zak", Qt::CaseInsensitive)) {
            // Открыть файл данных
            mainWindow.openWellFile(filePath);
        }
    }

    int result = app.exec();

    LOG_INFO(QString::fromUtf8("Завершение приложения Incline3D, код возврата: ") +
             QString::number(result));

    return result;
}
