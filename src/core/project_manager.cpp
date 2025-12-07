#include "core/project_manager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/file_io.h"

namespace incline3d::core {

ProjectManager::ProjectManager(QObject* parent)
    : QObject(parent) {
}

void ProjectManager::newProject() {
    data_ = ProjectData{};
    data_.created_date = QDateTime::currentDateTime().toString(Qt::ISODate);
    data_.modified_date = data_.created_date;
    wells_.clear();
    project_file_path_.clear();
    dirty_ = false;

    emit projectCreated();
    emit wellsChanged();
    emit dirtyChanged(false);
}

bool ProjectManager::loadProject(const QString& path) {
    if (!readProjectJson(path)) {
        return false;
    }

    project_file_path_ = path;
    dirty_ = false;

    emit projectLoaded(path);
    emit wellsChanged();
    emit dirtyChanged(false);

    return true;
}

bool ProjectManager::saveProject(const QString& path) {
    data_.modified_date = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!writeProjectJson(path)) {
        return false;
    }

    project_file_path_ = path;
    dirty_ = false;

    emit projectSaved(path);
    emit dirtyChanged(false);

    return true;
}

bool ProjectManager::saveProject() {
    if (project_file_path_.isEmpty()) {
        emit errorOccurred(tr("Путь к файлу проекта не задан"));
        return false;
    }
    return saveProject(project_file_path_);
}

bool ProjectManager::exportProject(const QString& directory) {
    QDir dir(directory);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit errorOccurred(tr("Не удалось создать каталог: %1").arg(directory));
            return false;
        }
    }

    FileIO io;

    // Экспорт всех скважин
    for (size_t i = 0; i < wells_.size(); ++i) {
        const auto& well = wells_[i];
        QString filename = QString::fromStdString(well->metadata.well_name) + ".ws";
        QString filepath = dir.filePath(filename);
        auto result = io.saveWell(filepath, *well, FileFormat::kWs);
        if (!result.success) {
            emit errorOccurred(tr("Ошибка экспорта скважины %1: %2")
                .arg(QString::fromStdString(well->metadata.well_name))
                .arg(result.error_message));
        }
    }

    // Экспорт проектных точек
    if (!data_.project_points.empty()) {
        QString pp_path = dir.filePath("project_points.txt");
        io.saveProjectPoints(pp_path, data_.project_points);
    }

    // Экспорт пунктов возбуждения
    if (!data_.shot_points.empty()) {
        QString sp_path = dir.filePath("shot_points.txt");
        io.saveShotPoints(sp_path, data_.shot_points);
    }

    return true;
}

QString ProjectManager::projectFilePath() const {
    return project_file_path_;
}

bool ProjectManager::isDirty() const {
    return dirty_;
}

void ProjectManager::setDirty(bool dirty) {
    if (dirty_ != dirty) {
        dirty_ = dirty;
        emit dirtyChanged(dirty);
    }
}

ProjectData& ProjectManager::projectData() {
    return data_;
}

const ProjectData& ProjectManager::projectData() const {
    return data_;
}

void ProjectManager::addWell(std::shared_ptr<models::WellData> well) {
    wells_.push_back(well);

    // Добавляем запись в данные проекта
    ProjectData::WellEntry entry;
    entry.file_path = QString::fromStdString(well->source_file_path);
    entry.format = QString::fromStdString(well->source_format);
    entry.visible = well->visible;
    entry.color = well->display_color;
    entry.line_width = well->line_width;
    data_.well_entries.push_back(entry);

    setDirty(true);
    emit wellsChanged();
}

void ProjectManager::removeWell(int index) {
    if (index < 0 || index >= static_cast<int>(wells_.size())) {
        return;
    }

    wells_.erase(wells_.begin() + index);
    if (index < static_cast<int>(data_.well_entries.size())) {
        data_.well_entries.erase(data_.well_entries.begin() + index);
    }

    setDirty(true);
    emit wellsChanged();
}

std::vector<std::shared_ptr<models::WellData>>& ProjectManager::wells() {
    return wells_;
}

const std::vector<std::shared_ptr<models::WellData>>& ProjectManager::wells() const {
    return wells_;
}

QString ProjectManager::getProjectFileFilter() {
    return QObject::tr(
        "Проекты Incline3D (*.inclproj);;"
        "JSON файлы (*.json);;"
        "Все файлы (*)");
}

bool ProjectManager::writeProjectJson(const QString& path) {
    QJsonObject root;

    root["version"] = data_.version;
    root["name"] = data_.name;
    root["description"] = data_.description;
    root["author"] = data_.author;
    root["created_date"] = data_.created_date;
    root["modified_date"] = data_.modified_date;

    // Скважины
    QJsonArray wells_array;
    for (const auto& entry : data_.well_entries) {
        QJsonObject well_obj;
        well_obj["file_path"] = entry.file_path;
        well_obj["format"] = entry.format;
        well_obj["visible"] = entry.visible;
        well_obj["color"] = entry.color.name();
        well_obj["line_width"] = entry.line_width;
        wells_array.append(well_obj);
    }
    root["wells"] = wells_array;

    // Проектные точки
    QJsonArray pp_array;
    for (const auto& pt : data_.project_points) {
        QJsonObject pt_obj;
        pt_obj["name"] = QString::fromStdString(pt.name);
        pt_obj["azimuth"] = pt.azimuth_geogr_deg;
        pt_obj["shift"] = pt.shift_m;
        pt_obj["depth"] = pt.depth_m;
        pt_obj["abs_depth"] = pt.abs_depth_m;
        pt_obj["radius"] = pt.radius_m;
        pt_obj["color"] = pt.display_color.name();
        pt_obj["visible"] = pt.visible;
        pp_array.append(pt_obj);
    }
    root["project_points"] = pp_array;

    // Пункты возбуждения
    QJsonArray sp_array;
    for (const auto& pt : data_.shot_points) {
        QJsonObject pt_obj;
        pt_obj["name"] = QString::fromStdString(pt.name);
        pt_obj["x"] = pt.x_m;
        pt_obj["y"] = pt.y_m;
        pt_obj["z"] = pt.z_m;
        pt_obj["color"] = pt.display_color.name();
        pt_obj["visible"] = pt.visible;
        pt_obj["marker"] = QString::fromStdString(models::marker_to_string(pt.marker));
        sp_array.append(pt_obj);
    }
    root["shot_points"] = sp_array;

    // Настройки визуализации
    QJsonObject view_obj;
    view_obj["rotation_x"] = data_.view_settings.rotation_x;
    view_obj["rotation_y"] = data_.view_settings.rotation_y;
    view_obj["rotation_z"] = data_.view_settings.rotation_z;
    view_obj["scale"] = data_.view_settings.scale;
    view_obj["pan_x"] = data_.view_settings.pan_x;
    view_obj["pan_y"] = data_.view_settings.pan_y;
    view_obj["pan_z"] = data_.view_settings.pan_z;
    view_obj["plan_scale"] = data_.view_settings.plan_scale;
    view_obj["plan_center_x"] = data_.view_settings.plan_center_x;
    view_obj["plan_center_y"] = data_.view_settings.plan_center_y;
    view_obj["vertical_azimuth"] = data_.view_settings.vertical_azimuth;
    view_obj["vertical_scale_h"] = data_.view_settings.vertical_scale_h;
    view_obj["vertical_scale_v"] = data_.view_settings.vertical_scale_v;
    root["view_settings"] = view_obj;

    // Параметры расчёта
    QJsonObject params_obj;
    params_obj["method"] = QString::fromStdString(
        models::method_to_string(data_.default_params.method));
    params_obj["declination"] = data_.default_params.magnetic_declination_deg;
    params_obj["meridian"] = data_.default_params.meridian_convergence_deg;
    params_obj["intensity_interval"] = data_.default_params.intensity_interval_m;
    root["calculation_params"] = params_obj;

    // Шапка
    QJsonObject header_obj;
    header_obj["title"] = data_.header_title;
    header_obj["company"] = data_.header_company;
    header_obj["field"] = data_.header_field;
    header_obj["logo_path"] = data_.logo_path;
    root["header"] = header_obj;

    // Запись в файл
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(tr("Не удалось открыть файл для записи: %1").arg(path));
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));

    return true;
}

bool ProjectManager::readProjectJson(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Не удалось открыть файл: %1").arg(path));
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        emit errorOccurred(tr("Ошибка парсинга JSON: %1").arg(error.errorString()));
        return false;
    }

    QJsonObject root = doc.object();

    // Очистка текущих данных
    data_ = ProjectData{};
    wells_.clear();

    // Основные поля
    data_.version = root["version"].toInt(1);
    data_.name = root["name"].toString();
    data_.description = root["description"].toString();
    data_.author = root["author"].toString();
    data_.created_date = root["created_date"].toString();
    data_.modified_date = root["modified_date"].toString();

    // Скважины
    QJsonArray wells_array = root["wells"].toArray();
    QDir project_dir = QFileInfo(path).absoluteDir();
    FileIO io;

    for (const auto& well_val : wells_array) {
        QJsonObject well_obj = well_val.toObject();
        ProjectData::WellEntry entry;
        entry.file_path = well_obj["file_path"].toString();
        entry.format = well_obj["format"].toString();
        entry.visible = well_obj["visible"].toBool(true);
        entry.color = QColor(well_obj["color"].toString("#0000ff"));
        entry.line_width = well_obj["line_width"].toInt(2);
        data_.well_entries.push_back(entry);

        // Загрузка данных скважины
        QString abs_path = entry.file_path;
        if (QFileInfo(abs_path).isRelative()) {
            abs_path = project_dir.filePath(entry.file_path);
        }

        if (QFile::exists(abs_path)) {
            auto result = io.loadWell(abs_path, FileIO::stringToFormat(entry.format));
            if (result.success && result.well) {
                result.well->visible = entry.visible;
                result.well->display_color = entry.color;
                result.well->line_width = entry.line_width;
                wells_.push_back(result.well);
            }
        }
    }

    // Проектные точки
    QJsonArray pp_array = root["project_points"].toArray();
    for (const auto& pt_val : pp_array) {
        QJsonObject pt_obj = pt_val.toObject();
        models::ProjectPoint pt;
        pt.name = pt_obj["name"].toString().toStdString();
        pt.azimuth_geogr_deg = pt_obj["azimuth"].toDouble();
        pt.shift_m = pt_obj["shift"].toDouble();
        pt.depth_m = pt_obj["depth"].toDouble();
        pt.abs_depth_m = pt_obj["abs_depth"].toDouble();
        pt.radius_m = pt_obj["radius"].toDouble();
        pt.display_color = QColor(pt_obj["color"].toString("#ff0000"));
        pt.visible = pt_obj["visible"].toBool(true);
        data_.project_points.push_back(pt);
    }

    // Пункты возбуждения
    QJsonArray sp_array = root["shot_points"].toArray();
    for (const auto& pt_val : sp_array) {
        QJsonObject pt_obj = pt_val.toObject();
        models::ShotPoint pt;
        pt.name = pt_obj["name"].toString().toStdString();
        pt.x_m = pt_obj["x"].toDouble();
        pt.y_m = pt_obj["y"].toDouble();
        pt.z_m = pt_obj["z"].toDouble();
        pt.display_color = QColor(pt_obj["color"].toString("#00ff00"));
        pt.visible = pt_obj["visible"].toBool(true);
        pt.marker = models::string_to_marker(pt_obj["marker"].toString().toStdString());
        data_.shot_points.push_back(pt);
    }

    // Настройки визуализации
    QJsonObject view_obj = root["view_settings"].toObject();
    data_.view_settings.rotation_x = view_obj["rotation_x"].toDouble(30.0);
    data_.view_settings.rotation_y = view_obj["rotation_y"].toDouble(-45.0);
    data_.view_settings.rotation_z = view_obj["rotation_z"].toDouble(0.0);
    data_.view_settings.scale = view_obj["scale"].toDouble(1.0);
    data_.view_settings.pan_x = view_obj["pan_x"].toDouble(0.0);
    data_.view_settings.pan_y = view_obj["pan_y"].toDouble(0.0);
    data_.view_settings.pan_z = view_obj["pan_z"].toDouble(0.0);
    data_.view_settings.plan_scale = view_obj["plan_scale"].toDouble(1.0);
    data_.view_settings.plan_center_x = view_obj["plan_center_x"].toDouble(0.0);
    data_.view_settings.plan_center_y = view_obj["plan_center_y"].toDouble(0.0);
    data_.view_settings.vertical_azimuth = view_obj["vertical_azimuth"].toDouble(0.0);
    data_.view_settings.vertical_scale_h = view_obj["vertical_scale_h"].toDouble(1.0);
    data_.view_settings.vertical_scale_v = view_obj["vertical_scale_v"].toDouble(1.0);

    // Параметры расчёта
    QJsonObject params_obj = root["calculation_params"].toObject();
    data_.default_params.method = models::string_to_method(
        params_obj["method"].toString().toStdString());
    data_.default_params.magnetic_declination_deg = params_obj["declination"].toDouble(0.0);
    data_.default_params.meridian_convergence_deg = params_obj["meridian"].toDouble(0.0);
    data_.default_params.intensity_interval_m = params_obj["intensity_interval"].toDouble(30.0);

    // Шапка
    QJsonObject header_obj = root["header"].toObject();
    data_.header_title = header_obj["title"].toString();
    data_.header_company = header_obj["company"].toString();
    data_.header_field = header_obj["field"].toString();
    data_.logo_path = header_obj["logo_path"].toString();

    return true;
}

}  // namespace incline3d::core
