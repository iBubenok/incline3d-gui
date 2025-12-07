#include "views/view3d_widget.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

#include "models/well_table_model.h"
#include "models/project_points_model.h"
#include "models/shot_points_model.h"

namespace incline3d::views {

View3DWidget::View3DWidget(QWidget* parent)
    : QOpenGLWidget(parent) {
    setMinimumSize(400, 300);
}

void View3DWidget::setWellModel(models::WellTableModel* model) {
    well_model_ = model;
    update();
}

void View3DWidget::setProjectPointsModel(models::ProjectPointsModel* model) {
    project_points_model_ = model;
    update();
}

void View3DWidget::setShotPointsModel(models::ShotPointsModel* model) {
    shot_points_model_ = model;
    update();
}

void View3DWidget::resetView() {
    rotation_x_ = 30.0;
    rotation_y_ = -45.0;
    rotation_z_ = 0.0;
    scale_ = 1.0;
    pan_ = QVector3D(0, 0, 0);
    update();
}

ViewSettings& View3DWidget::settings() {
    return settings_;
}

const ViewSettings& View3DWidget::settings() const {
    return settings_;
}

void View3DWidget::setRotationX(double angle) {
    rotation_x_ = angle;
    update();
}

void View3DWidget::setRotationY(double angle) {
    rotation_y_ = angle;
    update();
}

void View3DWidget::setRotationZ(double angle) {
    rotation_z_ = angle;
    update();
}

void View3DWidget::setScale(double scale) {
    scale_ = scale;
    update();
}

void View3DWidget::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void View3DWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    updateProjectionMatrix();
}

void View3DWidget::updateProjectionMatrix() {
    projection_matrix_.setToIdentity();
    float aspect = static_cast<float>(width()) / std::max(1, height());
    projection_matrix_.perspective(45.0f, aspect, 0.1f, 10000.0f);
}

void View3DWidget::paintGL() {
    const auto& bg = settings_.background_color;
    glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Настройка матрицы вида
    view_matrix_.setToIdentity();
    view_matrix_.translate(pan_.x(), pan_.y(), -1000.0f * scale_);
    view_matrix_.rotate(rotation_x_, 1, 0, 0);
    view_matrix_.rotate(rotation_y_, 0, 1, 0);
    view_matrix_.rotate(rotation_z_, 0, 0, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection_matrix_.constData());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view_matrix_.constData());

    if (settings_.show_grid) {
        drawGrid();
    }

    if (settings_.show_axes) {
        drawAxes();
    }

    drawWells();

    if (settings_.show_project_points) {
        drawProjectPoints();
    }

    if (settings_.show_shot_points) {
        drawShotPoints();
    }
}

void View3DWidget::drawGrid() {
    const auto& c = settings_.grid_color;
    glColor4f(c.redF(), c.greenF(), c.blueF(), 0.5f);
    glLineWidth(1.0f);

    double step = settings_.grid_step;
    int n = settings_.grid_divisions;
    double size = step * n;

    glBegin(GL_LINES);
    for (int i = -n; i <= n; ++i) {
        glVertex3f(i * step, -size, 0);
        glVertex3f(i * step, size, 0);
        glVertex3f(-size, i * step, 0);
        glVertex3f(size, i * step, 0);
    }
    glEnd();
}

void View3DWidget::drawAxes() {
    double len = settings_.axis_length;
    glLineWidth(2.0f);

    // X - красный (восток)
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(len, 0, 0);
    glEnd();

    // Y - зелёный (север)
    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, len, 0);
    glEnd();

    // Z - синий (глубина вниз)
    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, -len);
    glEnd();
}

void View3DWidget::drawWells() {
    if (!well_model_) return;

    for (int i = 0; i < well_model_->wellCount(); ++i) {
        auto well = well_model_->wellAt(i);
        if (!well || !well->visible || well->results.empty()) {
            continue;
        }

        const auto& color = well->display_color;
        glColor3f(color.redF(), color.greenF(), color.blueF());
        glLineWidth(well->line_width);

        glBegin(GL_LINE_STRIP);
        for (const auto& pt : well->results) {
            // X = восток, Y = север, Z = -TVD (глубина вниз)
            glVertex3f(pt.east_m, pt.north_m, -pt.tvd_m);
        }
        glEnd();

        // Точки замеров
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        for (const auto& pt : well->results) {
            glVertex3f(pt.east_m, pt.north_m, -pt.tvd_m);
        }
        glEnd();
    }
}

void View3DWidget::drawProjectPoints() {
    if (!project_points_model_) return;

    for (int i = 0; i < project_points_model_->pointCount(); ++i) {
        const auto& pt = project_points_model_->pointAt(i);
        if (!pt.visible) continue;

        const auto& color = pt.display_color;
        glColor3f(color.redF(), color.greenF(), color.blueF());

        // Рисуем точку
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        glVertex3f(pt.fact_east_m, pt.fact_north_m, -pt.fact_tvd_m);
        glEnd();

        // Круг допуска (упрощённо)
        if (settings_.show_tolerance_circles && pt.radius_m > 0) {
            glLineWidth(1.0f);
            glBegin(GL_LINE_LOOP);
            for (int j = 0; j < 36; ++j) {
                double angle = j * 10.0 * 3.14159 / 180.0;
                double x = pt.fact_east_m + pt.radius_m * std::cos(angle);
                double y = pt.fact_north_m + pt.radius_m * std::sin(angle);
                glVertex3f(x, y, -pt.fact_tvd_m);
            }
            glEnd();
        }
    }
}

void View3DWidget::drawShotPoints() {
    if (!shot_points_model_) return;

    for (int i = 0; i < shot_points_model_->pointCount(); ++i) {
        const auto& pt = shot_points_model_->pointAt(i);
        if (!pt.visible) continue;

        const auto& color = pt.display_color;
        glColor3f(color.redF(), color.greenF(), color.blueF());

        // Рисуем маркер (треугольник)
        float size = pt.marker_size;
        float x = pt.x_m;
        float y = pt.y_m;
        float z = -pt.z_m;

        glBegin(GL_TRIANGLES);
        glVertex3f(x, y + size, z);
        glVertex3f(x - size * 0.866f, y - size * 0.5f, z);
        glVertex3f(x + size * 0.866f, y - size * 0.5f, z);
        glEnd();
    }
}

void View3DWidget::drawDepthLabels() {
    // Реализация через QPainter overlay или OpenGL текст
    // Пока заглушка
}

void View3DWidget::mousePressEvent(QMouseEvent* event) {
    last_mouse_pos_ = event->pos();

    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier) {
            is_panning_ = true;
        } else {
            is_rotating_ = true;
        }
    }
}

void View3DWidget::mouseMoveEvent(QMouseEvent* event) {
    QPoint delta = event->pos() - last_mouse_pos_;
    last_mouse_pos_ = event->pos();

    if (is_rotating_) {
        rotation_y_ += delta.x() * 0.5;
        rotation_x_ += delta.y() * 0.5;
        update();
    } else if (is_panning_) {
        pan_.setX(pan_.x() + delta.x() * scale_);
        pan_.setY(pan_.y() - delta.y() * scale_);
        update();
    }
}

void View3DWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    is_rotating_ = false;
    is_panning_ = false;
}

void View3DWidget::wheelEvent(QWheelEvent* event) {
    double delta = event->angleDelta().y() / 120.0;
    scale_ *= (1.0 - delta * 0.1);
    if (scale_ < 0.01) scale_ = 0.01;
    if (scale_ > 100.0) scale_ = 100.0;
    update();
}

}  // namespace incline3d::views
