#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <vector>

#include "models/well_data.h"
#include "models/project_point.h"
#include "models/shot_point.h"

namespace incline3d::core {

/// Настройки визуализации для сохранения в проекте
struct ViewSettings {
    // 3D вид
    double rotation_x{30.0};
    double rotation_y{-45.0};
    double rotation_z{0.0};
    double scale{1.0};
    double pan_x{0.0};
    double pan_y{0.0};
    double pan_z{0.0};

    // План
    double plan_scale{1.0};
    double plan_center_x{0.0};
    double plan_center_y{0.0};

    // Вертикальная проекция
    double vertical_azimuth{0.0};
    double vertical_scale_h{1.0};
    double vertical_scale_v{1.0};
    double vertical_center_x{0.0};
    double vertical_center_y{0.0};
};

/// Данные проекта GUI
struct ProjectData {
    int version{1};
    QString name;
    QString description;
    QString author;
    QString created_date;
    QString modified_date;

    // Список скважин с путями к файлам
    struct WellEntry {
        QString file_path;
        QString format;
        bool visible{true};
        QColor color{Qt::blue};
        int line_width{2};
    };
    std::vector<WellEntry> well_entries;

    // Проектные точки
    std::vector<models::ProjectPoint> project_points;

    // Пункты возбуждения
    std::vector<models::ShotPoint> shot_points;

    // Настройки визуализации
    ViewSettings view_settings;

    // Параметры расчёта по умолчанию для проекта
    models::CalculationParams default_params;

    // Дополнительные данные (шапка, логотипы и т.п.)
    QString header_title;
    QString header_company;
    QString header_field;
    QString logo_path;
};

/// Менеджер проектов GUI
class ProjectManager : public QObject {
    Q_OBJECT

public:
    explicit ProjectManager(QObject* parent = nullptr);

    /// Создать новый пустой проект
    void newProject();

    /// Загрузить проект из файла
    bool loadProject(const QString& path);

    /// Сохранить проект в файл
    bool saveProject(const QString& path);

    /// Сохранить проект (в текущий файл)
    bool saveProject();

    /// Экспортировать проект в набор файлов
    bool exportProject(const QString& directory);

    /// Получить путь к текущему файлу проекта
    QString projectFilePath() const;

    /// Проверить, есть ли несохранённые изменения
    bool isDirty() const;

    /// Отметить проект как изменённый
    void setDirty(bool dirty = true);

    /// Получить данные проекта
    ProjectData& projectData();
    const ProjectData& projectData() const;

    /// Добавить скважину в проект
    void addWell(std::shared_ptr<models::WellData> well);

    /// Удалить скважину из проекта
    void removeWell(int index);

    /// Получить список загруженных скважин
    std::vector<std::shared_ptr<models::WellData>>& wells();
    const std::vector<std::shared_ptr<models::WellData>>& wells() const;

    /// Получить фильтр файлов проекта
    static QString getProjectFileFilter();

signals:
    /// Сигнал о создании нового проекта
    void projectCreated();

    /// Сигнал о загрузке проекта
    void projectLoaded(const QString& path);

    /// Сигнал о сохранении проекта
    void projectSaved(const QString& path);

    /// Сигнал об изменении состояния dirty
    void dirtyChanged(bool dirty);

    /// Сигнал об ошибке
    void errorOccurred(const QString& error);

    /// Сигнал об изменении списка скважин
    void wellsChanged();

private:
    bool writeProjectJson(const QString& path);
    bool readProjectJson(const QString& path);

    ProjectData data_;
    std::vector<std::shared_ptr<models::WellData>> wells_;
    QString project_file_path_;
    bool dirty_{false};
};

}  // namespace incline3d::core
