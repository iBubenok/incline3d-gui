#include "views/plan_view.h"

#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QScrollBar>
#include <cmath>

#include "models/well_table_model.h"
#include "models/project_points_model.h"
#include "models/shot_points_model.h"

namespace incline3d::views {

PlanView::PlanView(QWidget* parent)
    : QGraphicsView(parent) {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    // Инвертируем Y для геодезической системы координат
    // (север — вверх, восток — вправо)
    scale(1, -1);

    setMinimumSize(400, 300);
    setBackgroundBrush(Qt::white);
}

PlanView::~PlanView() = default;

void PlanView::setWellModel(models::WellTableModel* model) {
    well_model_ = model;
    rebuildScene();
}

void PlanView::setProjectPointsModel(models::ProjectPointsModel* model) {
    project_points_model_ = model;
    rebuildScene();
}

void PlanView::setShotPointsModel(models::ShotPointsModel* model) {
    shot_points_model_ = model;
    rebuildScene();
}

void PlanView::setShowGrid(bool show) {
    show_grid_ = show;
    viewport()->update();
}

void PlanView::setShowAxes(bool show) {
    show_axes_ = show;
    viewport()->update();
}

void PlanView::setShowLabels(bool show) {
    show_labels_ = show;
    rebuildScene();
}

void PlanView::setGridStep(double step) {
    grid_step_ = step;
    viewport()->update();
}

void PlanView::fitToContent() {
    if (scene_->items().isEmpty()) {
        return;
    }

    QRectF bounds = scene_->itemsBoundingRect();
    // Добавляем отступы
    double margin = std::max(bounds.width(), bounds.height()) * 0.1;
    bounds.adjust(-margin, -margin, margin, margin);
    fitInView(bounds, Qt::KeepAspectRatio);
}

void PlanView::resetView() {
    resetTransform();
    scale(1, -1);  // Восстанавливаем инверсию Y
    scale_factor_ = 1.0;
    fitToContent();
}

void PlanView::refresh() {
    rebuildScene();
}

void PlanView::rebuildScene() {
    scene_->clear();

    addWellTrajectories();
    addProjectPoints();
    addShotPoints();

    if (show_labels_) {
        addDepthLabels();
    }
}

void PlanView::wheelEvent(QWheelEvent* event) {
    double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
    scale_factor_ *= factor;
    event->accept();
}

void PlanView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        is_panning_ = true;
        last_mouse_pos_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void PlanView::mouseMoveEvent(QMouseEvent* event) {
    if (is_panning_) {
        QPoint delta = event->pos() - last_mouse_pos_;
        last_mouse_pos_ = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());  // Инверсия Y
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void PlanView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && is_panning_) {
        is_panning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void PlanView::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect);

    if (show_grid_) {
        drawGrid(painter, rect);
    }

    if (show_axes_) {
        drawAxes(painter, rect);
    }
}

void PlanView::drawGrid(QPainter* painter, const QRectF& rect) {
    painter->save();

    QPen gridPen(QColor(200, 200, 200), 0);
    painter->setPen(gridPen);

    double step = grid_step_;

    // Определяем границы сетки
    double left = std::floor(rect.left() / step) * step;
    double right = std::ceil(rect.right() / step) * step;
    double bottom = std::floor(rect.bottom() / step) * step;
    double top = std::ceil(rect.top() / step) * step;

    // Вертикальные линии
    for (double x = left; x <= right; x += step) {
        painter->drawLine(QPointF(x, bottom), QPointF(x, top));
    }

    // Горизонтальные линии
    for (double y = bottom; y <= top; y += step) {
        painter->drawLine(QPointF(left, y), QPointF(right, y));
    }

    painter->restore();
}

void PlanView::drawAxes(QPainter* painter, const QRectF& rect) {
    painter->save();

    // Ось X (восток) — красная
    QPen xPen(Qt::red, 0);
    xPen.setWidthF(2.0 / scale_factor_);
    painter->setPen(xPen);
    painter->drawLine(QPointF(0, 0), QPointF(rect.right(), 0));

    // Стрелка X
    double arrowSize = 10.0 / scale_factor_;
    painter->drawLine(QPointF(rect.right() - arrowSize, -arrowSize),
                      QPointF(rect.right(), 0));
    painter->drawLine(QPointF(rect.right() - arrowSize, arrowSize),
                      QPointF(rect.right(), 0));

    // Ось Y (север) — зелёная
    QPen yPen(Qt::green, 0);
    yPen.setWidthF(2.0 / scale_factor_);
    painter->setPen(yPen);
    painter->drawLine(QPointF(0, 0), QPointF(0, rect.top()));

    // Стрелка Y
    painter->drawLine(QPointF(-arrowSize, rect.top() - arrowSize),
                      QPointF(0, rect.top()));
    painter->drawLine(QPointF(arrowSize, rect.top() - arrowSize),
                      QPointF(0, rect.top()));

    painter->restore();
}

void PlanView::addWellTrajectories() {
    if (!well_model_) return;

    for (int i = 0; i < well_model_->wellCount(); ++i) {
        auto well = well_model_->wellAt(i);
        if (!well || !well->visible || well->results.empty()) {
            continue;
        }

        // Строим путь траектории
        QPainterPath path;
        bool first = true;

        for (const auto& pt : well->results) {
            // X = восток, Y = север
            if (first) {
                path.moveTo(pt.east_m, pt.north_m);
                first = false;
            } else {
                path.lineTo(pt.east_m, pt.north_m);
            }
        }

        auto* pathItem = new QGraphicsPathItem(path);
        QPen pen(well->display_color, well->line_width);
        pen.setCosmetic(true);  // Толщина не зависит от масштаба
        pathItem->setPen(pen);
        pathItem->setToolTip(QString::fromStdString(well->metadata.well_name));
        scene_->addItem(pathItem);

        // Точка устья
        if (!well->results.empty()) {
            const auto& first_pt = well->results.front();
            auto* wellhead = new QGraphicsEllipseItem(
                first_pt.east_m - 3, first_pt.north_m - 3, 6, 6);
            wellhead->setBrush(well->display_color);
            wellhead->setPen(Qt::NoPen);
            wellhead->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            scene_->addItem(wellhead);
        }
    }
}

void PlanView::addProjectPoints() {
    if (!project_points_model_) return;

    for (int i = 0; i < project_points_model_->pointCount(); ++i) {
        const auto& pt = project_points_model_->pointAt(i);
        if (!pt.visible) continue;

        // Фактическая точка
        auto* point = new QGraphicsEllipseItem(
            pt.fact_east_m - 4, pt.fact_north_m - 4, 8, 8);
        point->setBrush(pt.display_color);
        point->setPen(Qt::NoPen);
        point->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        point->setToolTip(QString::fromStdString(pt.name) +
                          QString("\nГлубина: %1 м").arg(pt.fact_tvd_m, 0, 'f', 1));
        scene_->addItem(point);

        // Круг допуска
        if (pt.radius_m > 0) {
            auto* tolerance = new QGraphicsEllipseItem(
                pt.fact_east_m - pt.radius_m,
                pt.fact_north_m - pt.radius_m,
                pt.radius_m * 2, pt.radius_m * 2);
            QPen tolPen(pt.display_color, 1);
            tolPen.setCosmetic(true);
            tolPen.setStyle(Qt::DashLine);
            tolerance->setPen(tolPen);
            tolerance->setBrush(Qt::NoBrush);
            scene_->addItem(tolerance);
        }

        // Показываем направление до проектной точки (базовое смещение)
        if (pt.shift_m > 0 && pt.base_shift_m > 0) {
            // Вычисляем плановую позицию по азимуту и смещению
            double az_rad = pt.azimuth_geogr_deg * 3.14159265358979323846 / 180.0;
            double plan_east = pt.shift_m * std::sin(az_rad);
            double plan_north = pt.shift_m * std::cos(az_rad);

            // Линия от фактической к плановой позиции
            auto* line = scene_->addLine(
                pt.fact_east_m, pt.fact_north_m,
                plan_east, plan_north,
                QPen(pt.display_color, 1, Qt::DotLine));
            Q_UNUSED(line)
        }
    }
}

void PlanView::addShotPoints() {
    if (!shot_points_model_) return;

    for (int i = 0; i < shot_points_model_->pointCount(); ++i) {
        const auto& pt = shot_points_model_->pointAt(i);
        if (!pt.visible) continue;

        // Треугольник маркер
        QPolygonF triangle;
        float size = pt.marker_size;
        triangle << QPointF(pt.x_m, pt.y_m + size)
                 << QPointF(pt.x_m - size * 0.866, pt.y_m - size * 0.5)
                 << QPointF(pt.x_m + size * 0.866, pt.y_m - size * 0.5);

        auto* marker = new QGraphicsPolygonItem(triangle);
        marker->setBrush(pt.display_color);
        marker->setPen(Qt::NoPen);
        marker->setToolTip(QString::fromStdString(pt.name));
        scene_->addItem(marker);
    }
}

void PlanView::addDepthLabels() {
    if (!well_model_) return;

    for (int i = 0; i < well_model_->wellCount(); ++i) {
        auto well = well_model_->wellAt(i);
        if (!well || !well->visible || well->results.empty()) {
            continue;
        }

        // Подпись через каждые N метров глубины
        double label_step = 500.0;  // каждые 500 м
        double last_labeled_tvd = -label_step;

        for (const auto& pt : well->results) {
            if (pt.tvd_m >= last_labeled_tvd + label_step) {
                auto* label = new QGraphicsTextItem(
                    QString("%1").arg(pt.tvd_m, 0, 'f', 0));
                label->setPos(pt.east_m + 5, pt.north_m);
                label->setDefaultTextColor(well->display_color);

                // Инвертируем текст обратно для читаемости
                QTransform t;
                t.scale(1, -1);
                label->setTransform(t);
                label->setFlag(QGraphicsItem::ItemIgnoresTransformations);

                scene_->addItem(label);
                last_labeled_tvd = pt.tvd_m;
            }
        }

        // Подпись имени скважины у устья
        if (!well->results.empty()) {
            const auto& first_pt = well->results.front();
            auto* nameLabel = new QGraphicsTextItem(
                QString::fromStdString(well->metadata.well_name));
            nameLabel->setPos(first_pt.east_m + 10, first_pt.north_m);
            nameLabel->setDefaultTextColor(well->display_color);

            QTransform t;
            t.scale(1, -1);
            nameLabel->setTransform(t);
            nameLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);

            QFont font = nameLabel->font();
            font.setBold(true);
            nameLabel->setFont(font);

            scene_->addItem(nameLabel);
        }
    }
}

}  // namespace incline3d::views
