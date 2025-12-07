#include "views/vertical_view.h"

#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QScrollBar>
#include <cmath>
#include <algorithm>

#include "models/well_table_model.h"
#include "models/project_points_model.h"

namespace incline3d::views {

namespace {
constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
}

VerticalView::VerticalView(QWidget* parent)
    : QGraphicsView(parent) {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    // Ось Y направлена вниз (глубина увеличивается вниз)
    // Не инвертируем, оставляем стандартную ориентацию

    setMinimumSize(400, 300);
    setBackgroundBrush(Qt::white);
}

VerticalView::~VerticalView() = default;

void VerticalView::setWellModel(models::WellTableModel* model) {
    well_model_ = model;
    rebuildScene();
}

void VerticalView::setProjectPointsModel(models::ProjectPointsModel* model) {
    project_points_model_ = model;
    rebuildScene();
}

void VerticalView::setProfileAzimuth(double azimuth_deg) {
    // Нормализуем в диапазон [0, 360)
    while (azimuth_deg < 0) azimuth_deg += 360.0;
    while (azimuth_deg >= 360.0) azimuth_deg -= 360.0;

    if (std::abs(profile_azimuth_ - azimuth_deg) > 0.01) {
        profile_azimuth_ = azimuth_deg;
        rebuildScene();
        emit profileAzimuthChanged(profile_azimuth_);
    }
}

void VerticalView::autoFitAzimuth() {
    if (!well_model_ || well_model_->wellCount() == 0) {
        return;
    }

    // Находим азимут, при котором максимальное смещение по профилю
    // Используем первую видимую скважину для определения

    for (int i = 0; i < well_model_->wellCount(); ++i) {
        auto well = well_model_->wellAt(i);
        if (!well || !well->visible || well->results.size() < 2) {
            continue;
        }

        // Вычисляем азимут от устья до забоя
        const auto& first = well->results.front();
        const auto& last = well->results.back();

        double delta_e = last.east_m - first.east_m;
        double delta_n = last.north_m - first.north_m;

        // Азимут от устья к забою
        double azimuth = std::atan2(delta_e, delta_n) / DEG_TO_RAD;
        if (azimuth < 0) azimuth += 360.0;

        setProfileAzimuth(azimuth);
        return;
    }
}

void VerticalView::setShowGrid(bool show) {
    show_grid_ = show;
    viewport()->update();
}

void VerticalView::setShowLabels(bool show) {
    show_labels_ = show;
    rebuildScene();
}

void VerticalView::setGridStep(double step) {
    grid_step_ = step;
    viewport()->update();
}

void VerticalView::fitToContent() {
    if (scene_->items().isEmpty()) {
        return;
    }

    QRectF bounds = scene_->itemsBoundingRect();
    double margin = std::max(bounds.width(), bounds.height()) * 0.1;
    bounds.adjust(-margin, -margin, margin, margin);
    fitInView(bounds, Qt::KeepAspectRatio);
}

void VerticalView::resetView() {
    resetTransform();
    scale_factor_ = 1.0;
    fitToContent();
}

void VerticalView::refresh() {
    rebuildScene();
}

void VerticalView::rebuildScene() {
    scene_->clear();

    addWellProfiles();
    addProjectPoints();

    if (show_labels_) {
        addDepthScale();
    }
}

double VerticalView::projectToProfile(double east, double north) const {
    // Проекция точки на линию профиля
    // Профиль идёт в направлении profile_azimuth_ от начала координат
    double az_rad = profile_azimuth_ * DEG_TO_RAD;

    // Единичный вектор направления профиля
    double dir_e = std::sin(az_rad);
    double dir_n = std::cos(az_rad);

    // Скалярное произведение — проекция на направление
    return east * dir_e + north * dir_n;
}

void VerticalView::wheelEvent(QWheelEvent* event) {
    double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
    scale_factor_ *= factor;
    event->accept();
}

void VerticalView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        is_panning_ = true;
        last_mouse_pos_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void VerticalView::mouseMoveEvent(QMouseEvent* event) {
    if (is_panning_) {
        QPoint delta = event->pos() - last_mouse_pos_;
        last_mouse_pos_ = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void VerticalView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && is_panning_) {
        is_panning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void VerticalView::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect);

    if (show_grid_) {
        drawGrid(painter, rect);
    }
}

void VerticalView::drawGrid(QPainter* painter, const QRectF& rect) {
    painter->save();

    QPen gridPen(QColor(200, 200, 200), 0);
    painter->setPen(gridPen);

    double step = grid_step_;

    double left = std::floor(rect.left() / step) * step;
    double right = std::ceil(rect.right() / step) * step;
    double top = std::floor(rect.top() / step) * step;
    double bottom = std::ceil(rect.bottom() / step) * step;

    // Вертикальные линии (смещение по профилю)
    for (double x = left; x <= right; x += step) {
        painter->drawLine(QPointF(x, top), QPointF(x, bottom));
    }

    // Горизонтальные линии (глубина)
    for (double y = top; y <= bottom; y += step) {
        painter->drawLine(QPointF(left, y), QPointF(right, y));
    }

    // Ось глубины
    QPen axisPen(Qt::black, 0);
    axisPen.setWidthF(1.5);
    painter->setPen(axisPen);
    painter->drawLine(QPointF(0, top), QPointF(0, bottom));

    // Нулевая глубина
    painter->drawLine(QPointF(left, 0), QPointF(right, 0));

    painter->restore();
}

void VerticalView::addWellProfiles() {
    if (!well_model_) return;

    for (int i = 0; i < well_model_->wellCount(); ++i) {
        auto well = well_model_->wellAt(i);
        if (!well || !well->visible || well->results.empty()) {
            continue;
        }

        // Строим путь профиля
        QPainterPath path;
        bool first = true;

        for (const auto& pt : well->results) {
            // X = проекция на профиль, Y = TVD (глубина вниз)
            double x = projectToProfile(pt.east_m, pt.north_m);
            double y = pt.tvd_m;

            if (first) {
                path.moveTo(x, y);
                first = false;
            } else {
                path.lineTo(x, y);
            }
        }

        auto* pathItem = new QGraphicsPathItem(path);
        QPen pen(well->display_color, well->line_width);
        pen.setCosmetic(true);
        pathItem->setPen(pen);
        pathItem->setToolTip(QString::fromStdString(well->metadata.well_name));
        scene_->addItem(pathItem);

        // Точки замеров
        for (const auto& pt : well->results) {
            double x = projectToProfile(pt.east_m, pt.north_m);
            double y = pt.tvd_m;

            auto* point = new QGraphicsEllipseItem(x - 2, y - 2, 4, 4);
            point->setBrush(well->display_color);
            point->setPen(Qt::NoPen);
            point->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            scene_->addItem(point);
        }

        // Подпись скважины
        if (show_labels_ && !well->results.empty()) {
            const auto& first_pt = well->results.front();
            double x = projectToProfile(first_pt.east_m, first_pt.north_m);

            auto* label = new QGraphicsTextItem(
                QString::fromStdString(well->metadata.well_name));
            label->setPos(x + 5, first_pt.tvd_m - 15);
            label->setDefaultTextColor(well->display_color);
            label->setFlag(QGraphicsItem::ItemIgnoresTransformations);

            QFont font = label->font();
            font.setBold(true);
            label->setFont(font);

            scene_->addItem(label);
        }
    }
}

void VerticalView::addProjectPoints() {
    if (!project_points_model_) return;

    for (int i = 0; i < project_points_model_->pointCount(); ++i) {
        const auto& pt = project_points_model_->pointAt(i);
        if (!pt.visible) continue;

        double x = projectToProfile(pt.fact_east_m, pt.fact_north_m);
        double y = pt.fact_tvd_m;

        // Точка
        auto* point = new QGraphicsEllipseItem(x - 4, y - 4, 8, 8);
        point->setBrush(pt.display_color);
        point->setPen(Qt::NoPen);
        point->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        point->setToolTip(QString::fromStdString(pt.name) +
                          QString("\nГлубина: %1 м").arg(pt.fact_tvd_m, 0, 'f', 1));
        scene_->addItem(point);

        // Горизонтальная линия допуска (упрощённо)
        if (pt.radius_m > 0) {
            auto* tolLine = scene_->addLine(
                x - pt.radius_m, y, x + pt.radius_m, y,
                QPen(pt.display_color, 1, Qt::DashLine));
            Q_UNUSED(tolLine)
        }

        // Подпись
        if (show_labels_) {
            auto* label = new QGraphicsTextItem(QString::fromStdString(pt.name));
            label->setPos(x + 8, y - 5);
            label->setDefaultTextColor(pt.display_color);
            label->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            scene_->addItem(label);
        }
    }
}

void VerticalView::addDepthScale() {
    // Добавляем шкалу глубины слева
    QRectF bounds = scene_->itemsBoundingRect();
    if (bounds.isEmpty()) return;

    double step = grid_step_;
    double min_depth = std::floor(bounds.top() / step) * step;
    double max_depth = std::ceil(bounds.bottom() / step) * step;

    for (double depth = min_depth; depth <= max_depth; depth += step) {
        // Подпись глубины
        auto* label = new QGraphicsTextItem(QString("%1").arg(depth, 0, 'f', 0));
        label->setPos(bounds.left() - 40, depth - 8);
        label->setDefaultTextColor(Qt::darkGray);
        label->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        scene_->addItem(label);
    }

    // Заголовок азимута профиля
    auto* azLabel = new QGraphicsTextItem(
        QString("Профиль Аз=%1°").arg(profile_azimuth_, 0, 'f', 1));
    azLabel->setPos(bounds.left(), bounds.top() - 25);
    azLabel->setDefaultTextColor(Qt::black);
    azLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    QFont font = azLabel->font();
    font.setBold(true);
    azLabel->setFont(font);

    scene_->addItem(azLabel);
}

}  // namespace incline3d::views
