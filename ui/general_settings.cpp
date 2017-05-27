#include "general_settings.hpp"
#include "ui_general_settings.h"

#include "../core/global_constant.hpp"

#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>

general_settings::general_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::general_settings),
    search_by_changed_{false},
    write_able_path_(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/QImageScraperDownload")
{
    ui->setupUi(this);

    ui->comboBoxSearchBy->addItem(global_constant::bing_search_name());
    ui->comboBoxSearchBy->addItem(global_constant::google_search_name());

    QSettings settings;
    if(settings.contains("general/save_at")){
        if(settings.value("general/save").toString().isEmpty()){
            ui->lineEditSaveAt->setText(write_able_path_);
        }else{
            ui->lineEditSaveAt->setText(settings.value("general/save_at").toString());
        }
    }else{
        ui->lineEditSaveAt->setText(write_able_path_);
        settings.setValue("general/save_at", ui->lineEditSaveAt->text());
    }

    if(settings.contains("general/search_by")){
        ui->comboBoxSearchBy->setCurrentText(settings.value("general/search_by").toString());
    }else{
        settings.setValue("general/search_by", ui->comboBoxSearchBy->currentText());
    }

    if(settings.contains("general/max_download_img")){
        ui->spinBoxMaxDownloadImg->setValue(settings.value("general/max_download_img").toInt());
    }else{
        settings.setValue("general/max_download_img", ui->spinBoxMaxDownloadImg->value());
    }
}

general_settings::~general_settings()
{        
    delete ui;
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

void general_settings::on_buttonBox_accepted()
{
    QSettings settings;
    if(ui->comboBoxSearchBy->currentText() != settings.value("general/search_by").toString()){
        search_by_changed_ = true;
    }else{
        search_by_changed_ = false;
    }

    settings.setValue("general/search_by", ui->comboBoxSearchBy->currentText());
    settings.setValue("general/max_download_img", ui->spinBoxMaxDownloadImg->value());

    QDir dir;
    if(!dir.mkpath(ui->lineEditSaveAt->text())){
        emit cannot_create_save_dir(ui->lineEditSaveAt->text(), write_able_path_);
        ui->lineEditSaveAt->setText(write_able_path_);
    }

    settings.setValue("general/save_at", ui->lineEditSaveAt->text());

    emit ok_clicked();

    close();
}

void general_settings::restore_value()
{
    QSettings settings;
    ui->lineEditSaveAt->setText(settings.value("general/save_at").toString());
    ui->comboBoxSearchBy->setCurrentText(settings.value("general/search_by").toString());
    ui->spinBoxMaxDownloadImg->setValue(settings.value("general/max_download_img").toInt());
}

void general_settings::on_buttonBox_rejected()
{
    restore_value();

    close();
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
                                                          QSettings().value("general/save_at").toString(),
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);
    ui->lineEditSaveAt->setText(dir);
}
