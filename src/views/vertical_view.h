#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>

namespace incline3d::models {
class WellTableModel;
class ProjectPointsModel;
}  // namespace incline3d::models

namespace incline3d::views {

/// 2D вид "Вертикальная проекция" — профиль траектории скважины
class VerticalView : public QGraphicsView {
    Q_OBJECT

public:
    explicit VerticalView(QWidget* parent = nullptr);
    ~VerticalView() override;

    void setWellModel(models::WellTableModel* model);
    void setProjectPointsModel(models::ProjectPointsModel* model);

    /// Установить азимут профиля (градусы от севера)
    void setProfileAzimuth(double azimuth_deg);
    double profileAzimuth() const { return profile_azimuth_; }

    /// Автоподбор азимута для максимального смещения
    void autoFitAzimuth();

    void setShowGrid(bool show);
    bool showGrid() const { return show_grid_; }
    void setShowLabels(bool show);
    bool showLabels() const { return show_labels_; }
    void setGridStep(double step);
    double gridStep() const { return grid_step_; }

    void fitToContent();
    void resetView();

public slots:
    void refresh();

signals:
    void profileAzimuthChanged(double azimuth_deg);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void rebuildScene();
    void drawGrid(QPainter* painter, const QRectF& rect);
    void addWellProfiles();
    void addProjectPoints();
    void addDepthScale();

    /// Проецировать точку (east, north) на профиль с заданным азимутом
    double projectToProfile(double east, double north) const;

    QGraphicsScene* scene_{nullptr};

    models::WellTableModel* well_model_{nullptr};
    models::ProjectPointsModel* project_points_model_{nullptr};

    // Настройки отображения
    double profile_azimuth_{0.0};  // Азимут профиля в градусах от севера
    bool show_grid_{true};
    bool show_labels_{true};
    double grid_step_{100.0};

    // Состояние мыши
    QPoint last_mouse_pos_;
    bool is_panning_{false};

    double scale_factor_{1.0};
};

}  // namespace incline3d::views
