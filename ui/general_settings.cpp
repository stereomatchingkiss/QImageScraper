#include "general_settings.hpp"
#include "ui_general_settings.h"

#include "../core/global_constant.hpp"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>

namespace{

struct setting_keys
{
    static QString const general_save_at;
    static QString const general_search_by;
    static QString const max_download_img;
};

QString const setting_keys::general_save_at("general/save_at");
QString const setting_keys::general_search_by("general/search_by");
QString const setting_keys::max_download_img("general/max_download_img");

}

general_settings::general_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::general_settings),
    search_by_changed_{false},
    write_able_path_(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/QImageScraperDownload")
{
    ui->setupUi(this);

    ui->comboBoxSearchBy->addItem(global_constant::adobe_stock_name());
    ui->comboBoxSearchBy->addItem(global_constant::bing_search_name());
    ui->comboBoxSearchBy->addItem(global_constant::deposit_photo_name());
    ui->comboBoxSearchBy->addItem(global_constant::dream_time_name());
    ui->comboBoxSearchBy->addItem(global_constant::google_search_name());
    ui->comboBoxSearchBy->addItem(global_constant::shutter_stock_name());
    ui->comboBoxSearchBy->addItem(global_constant::yahoo_search_name());
    ui->comboBoxSearchBy->addItem(global_constant::pexels_search_name());

    QSettings settings;
    if(settings.contains(setting_keys::general_save_at)){
        if(settings.value(setting_keys::general_save_at).toString().isEmpty()){
            ui->lineEditSaveAt->setText(write_able_path_);
        }else{
            ui->lineEditSaveAt->setText(settings.value(setting_keys::general_save_at).toString());
        }
    }else{
        ui->lineEditSaveAt->setText(write_able_path_);
        settings.setValue(setting_keys::general_save_at, ui->lineEditSaveAt->text());
    }

    if(settings.contains(setting_keys::general_search_by)){
        ui->comboBoxSearchBy->setCurrentText(settings.value(setting_keys::general_search_by).toString());
    }else{
        settings.setValue(setting_keys::general_search_by, ui->comboBoxSearchBy->currentText());
    }

    if(settings.contains(setting_keys::max_download_img)){
        ui->spinBoxMaxDownloadImg->setValue(settings.value(setting_keys::max_download_img).toInt());
    }else{
        settings.setValue(setting_keys::max_download_img, ui->spinBoxMaxDownloadImg->value());
    }
}

general_settings::~general_settings()
{        
    delete ui;
}

void general_settings::accept_settings()
{
    QSettings settings;
    if(ui->comboBoxSearchBy->currentText() != settings.value(setting_keys::general_search_by).toString()){
        search_by_changed_ = true;
    }else{
        search_by_changed_ = false;
    }

    settings.setValue(setting_keys::general_search_by, ui->comboBoxSearchBy->currentText());
    settings.setValue(setting_keys::max_download_img, ui->spinBoxMaxDownloadImg->value());

    QDir dir;
    if(!dir.mkpath(ui->lineEditSaveAt->text())){
        QMessageBox::warning(this, tr("Error"), tr("Cannot create directory to save image %1, please choose a new directory, "
                                                   "if not the images will save at %2").
                             arg(ui->lineEditSaveAt->text()).
                             arg(write_able_path_));
        ui->lineEditSaveAt->setText(write_able_path_);
    }

    settings.setValue(setting_keys::general_save_at, ui->lineEditSaveAt->text());
}

int general_settings::get_max_download_img() const
{
    return ui->spinBoxMaxDownloadImg->value();
}

QString general_settings::get_save_at() const
{
    return ui->lineEditSaveAt->text();
}

QString general_settings::get_search_by() const
{
    return ui->comboBoxSearchBy->currentText();
}

bool general_settings::search_by_changed() const
{
    return search_by_changed_;
}

void general_settings::reject_settings()
{
    restore_value();
}

void general_settings::restore_value()
{
    QSettings settings;
    ui->lineEditSaveAt->setText(settings.value(setting_keys::general_save_at).toString());
    ui->comboBoxSearchBy->setCurrentText(settings.value(setting_keys::general_search_by).toString());
    ui->spinBoxMaxDownloadImg->setValue(settings.value(setting_keys::max_download_img).toInt());
}

void general_settings::closeEvent(QCloseEvent *event)
{
    if(event){
        restore_value();
        QDialog::closeEvent(event);
    }
}

void general_settings::on_pushButtonSaveAt_clicked()
{
    QString const dir = QFileDialog::getExistingDirectory(this, tr("Open/Create Save At directory"),
                                                          QSettings().value(setting_keys::general_save_at).toString(),
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);
    ui->lineEditSaveAt->setText(dir);
}
