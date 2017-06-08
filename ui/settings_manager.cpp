#include "settings_manager.hpp"
#include "ui_settings_manager.h"

#include <QsLog.h>

#include <QSettings>

settings_manager::settings_manager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
    connect(ui->listWidgetStrategy, &QListWidget::clicked, [this](QModelIndex const &index)
    {
        if(index.isValid()){
            QLOG_INFO()<<"list index is valid:"<<index.row();
            ui->stackedWidgetSettings->setCurrentIndex(index.row());
        }
    });

    ui->listWidgetStrategy->setCurrentRow(0);

    QSettings settings;
    if(settings.contains("settings_manager/geometry")){
        restoreGeometry(settings.value("settings_manager/geometry").toByteArray());
    }
}

settings_manager::~settings_manager()
{
    QSettings settings;
    settings.setValue("settings_manager/geometry", saveGeometry());

    delete ui;
}

general_settings const& settings_manager::get_general_settings() const noexcept
{    
    return *qobject_cast<general_settings*>(ui->stackedWidgetSettings->widget(0));
}

proxy_settings const &settings_manager::get_proxy_settings() const noexcept
{
    return *qobject_cast<proxy_settings*>(ui->stackedWidgetSettings->widget(1));
}

general_settings &settings_manager::get_pri_general_settings() noexcept
{
    return *qobject_cast<general_settings*>(ui->stackedWidgetSettings->widget(0));
}

proxy_settings &settings_manager::get_pri_proxy_settings() noexcept
{
    return *qobject_cast<proxy_settings*>(ui->stackedWidgetSettings->widget(1));
}

void settings_manager::on_settings_accepted()
{
    QLOG_INFO()<<__func__;
    get_pri_general_settings().accept_settings();
    emit ok_clicked();
}

void settings_manager::on_settings_rejected()
{
    get_pri_general_settings().reject_settings();
}
