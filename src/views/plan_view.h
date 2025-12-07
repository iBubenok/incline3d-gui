#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVector>
#include <QPointF>

namespace incline3d::models {
class WellTableModel;
class ProjectPointsModel;
class ShotPointsModel;
}  // namespace incline3d::models

namespace incline3d::views {

/// 2D вид "План" — горизонтальная проекция траекторий скважин
class PlanView : public QGraphicsView {
    Q_OBJECT

public:
    explicit PlanView(QWidget* parent = nullptr);
    ~PlanView() override;

    void setWellModel(models::WellTableModel* model);
    void setProjectPointsModel(models::ProjectPointsModel* model);
    void setShotPointsModel(models::ShotPointsModel* model);

    void setShowGrid(bool show);
    void setShowAxes(bool show);
    void setShowLabels(bool show);
    void setGridStep(double step);

    void fitToContent();
    void resetView();

public slots:
    void refresh();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void rebuildScene();
    void drawGrid(QPainter* painter, const QRectF& rect);
    void drawAxes(QPainter* painter, const QRectF& rect);
    void addWellTrajectories();
    void addProjectPoints();
    void addShotPoints();
    void addDepthLabels();

    QGraphicsScene* scene_{nullptr};

    models::WellTableModel* well_model_{nullptr};
    models::ProjectPointsModel* project_points_model_{nullptr};
    models::ShotPointsModel* shot_points_model_{nullptr};

    // Настройки отображения
    bool show_grid_{true};
    bool show_axes_{true};
    bool show_labels_{true};
    double grid_step_{100.0};

    // Состояние мыши для панорамирования
    QPoint last_mouse_pos_;
    bool is_panning_{false};

    // Масштаб (пикселей на метр)
    double scale_factor_{1.0};
};

}  // namespace incline3d::views
