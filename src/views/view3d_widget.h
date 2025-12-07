#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>

#include "views/view_settings.h"

namespace incline3d::models {
class WellTableModel;
class ProjectPointsModel;
class ShotPointsModel;
}  // namespace incline3d::models

namespace incline3d::views {

/// 3D виджет для аксонометрической визуализации траекторий скважин
class View3DWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit View3DWidget(QWidget* parent = nullptr);

    void setWellModel(models::WellTableModel* model);
    void setProjectPointsModel(models::ProjectPointsModel* model);
    void setShotPointsModel(models::ShotPointsModel* model);

    void resetView();

    ViewSettings& settings();
    const ViewSettings& settings() const;

    // Accessor методы для настроек отображения
    bool showGrid() const { return settings_.show_grid; }
    void setShowGrid(bool show) { settings_.show_grid = show; update(); }

    bool showLabels() const { return settings_.show_depth_labels; }
    void setShowLabels(bool show) { settings_.show_depth_labels = show; update(); }

    bool showAxes() const { return settings_.show_axes; }
    void setShowAxes(bool show) { settings_.show_axes = show; update(); }

    double gridStep() const { return settings_.grid_step; }
    void setGridStep(double step) { settings_.grid_step = step; update(); }

public slots:
    void setRotationX(double angle);
    void setRotationY(double angle);
    void setRotationZ(double angle);
    void setScale(double scale);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawGrid();
    void drawAxes();
    void drawWells();
    void drawProjectPoints();
    void drawShotPoints();
    void drawDepthLabels();

    void updateProjectionMatrix();

    models::WellTableModel* well_model_{nullptr};
    models::ProjectPointsModel* project_points_model_{nullptr};
    models::ShotPointsModel* shot_points_model_{nullptr};

    ViewSettings settings_;

    // Параметры вида
    double rotation_x_{30.0};
    double rotation_y_{-45.0};
    double rotation_z_{0.0};
    double scale_{1.0};
    QVector3D pan_{0, 0, 0};

    // Матрицы
    QMatrix4x4 projection_matrix_;
    QMatrix4x4 view_matrix_;

    // Состояние мыши
    QPoint last_mouse_pos_;
    bool is_rotating_{false};
    bool is_panning_{false};
};

}  // namespace incline3d::views
