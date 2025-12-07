#include "ui/about_dialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace incline3d::ui {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("О программе"));
    setFixedSize(400, 250);

    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel(tr("<h2>Incline3D</h2>"));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* version = new QLabel(tr("Версия 1.0.0"));
    version->setAlignment(Qt::AlignCenter);
    layout->addWidget(version);

    auto* desc = new QLabel(tr(
        "<p>Модуль инклинометрии комплекса «ПРАЙМ»</p>"
        "<p>Расчёт траекторий скважин, визуализация, анализ сближения и отхода.</p>"
    ));
    desc->setWordWrap(true);
    desc->setAlignment(Qt::AlignCenter);
    layout->addWidget(desc);

    layout->addStretch();

    auto* author = new QLabel(tr(
        "<p><b>Автор:</b> Yan Bubenok<br>"
        "<b>Email:</b> yan@bubenok.com<br>"
        "<b>Telegram:</b> @iBubenok</p>"
    ));
    author->setAlignment(Qt::AlignCenter);
    layout->addWidget(author);

    auto* copyright = new QLabel(tr("НПЦ «ГеоТЭК» (PrimeGeo)"));
    copyright->setAlignment(Qt::AlignCenter);
    layout->addWidget(copyright);

    layout->addStretch();

    auto* close_btn = new QPushButton(tr("Закрыть"));
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(close_btn, 0, Qt::AlignCenter);
}

}  // namespace incline3d::ui
