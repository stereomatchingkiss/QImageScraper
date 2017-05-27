#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "general_settings.hpp"

#include "core/global_constant.hpp"
#include "core/utility.hpp"

#include "ui/info_dialog.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QSizeGrip>

#include "core/bing_image_search.hpp"
#include "core/google_image_search.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    default_max_size_{maximumSize()},
    default_min_size_{minimumSize()},
    download_finished_{false},
    downloader_{new qte::net::download_supervisor(this)},
    general_settings_{new general_settings(this)},
    img_search_{nullptr}
{
    ui->setupUi(this);
    general_settings_ok_clicked();
    ui->actionDownload->setEnabled(false);
    ui->actionScroll->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionNew->setVisible(false);

    ui->labelProgress->setVisible(false);
    ui->progressBar->setVisible(false);

    setMinimumSize(size());

    using namespace qte::net;

    connect(downloader_, &download_supervisor::error, this, &MainWindow::download_img_error);
    connect(downloader_, &download_supervisor::download_finished, this, &MainWindow::download_finished);
    connect(downloader_, &download_supervisor::download_progress, this, &MainWindow::download_progress);
    connect(general_settings_, &general_settings::ok_clicked, this, &MainWindow::general_settings_ok_clicked);

    connect(general_settings_, &general_settings::cannot_create_save_dir,
            [this](QString const &dir, QString const &write_able_path)
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot create save at dir %1, please choose a new dir, "
                                                   "if not the images will save at %2").arg(dir).arg(write_able_path));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::found_img_link(const QString &big_img_link, const QString &small_img_link)
{
    static size_t img_link_count = 0;
    qDebug()<<"found image link count:"<<img_link_count++;
    download_img(std::make_tuple(big_img_link, small_img_link, link_choice::big));
}

void MainWindow::change_search_engine()
{
    if(general_settings_->get_search_by() == global_constant::bing_search_name()){
        img_search_ = new bing_image_search(*ui->webView->page(), this);
    }else{
        img_search_ = new google_image_search(*ui->webView->page(), this);
    }

    connect(img_search_, &image_search::go_to_search_page_done, this, &MainWindow::process_go_to_search_page);
    connect(img_search_, &image_search::go_to_gallery_page_done, this, &MainWindow::process_go_to_gallery_page);
    connect(img_search_, &image_search::show_more_images_done, this, &MainWindow::process_show_more_images_done);
    connect(img_search_, &image_search::search_error, this, &MainWindow::process_image_search_error);

    img_search_->go_to_search_page();
}

void MainWindow::general_settings_ok_clicked()
{
    if(img_search_){
        if(general_settings_->search_by_changed()){
            img_search_->deleteLater();
            change_search_engine();
        }
    }else{
        change_search_engine();
    }
}

bool MainWindow::is_download_finished() const
{
    return ui->progressBar->value() == ui->progressBar->maximum();
}

void MainWindow::process_go_to_search_page()
{
    ui->actionScroll->setEnabled(false);
    ui->actionDownload->setEnabled(false);
    ui->actionStop->setEnabled(false);
}

void MainWindow::process_go_to_gallery_page()
{
    ui->actionScroll->setEnabled(true);
    ui->actionDownload->setEnabled(true);
    ui->actionStop->setEnabled(false);
}

void MainWindow::process_image_search_error(image_search_error::error error)
{
    if(error == image_search_error::error::load_page_error){
        if(!is_download_finished()){
            ui->webView->page()->load(ui->webView->page()->url());
        }else{
            img_search_->go_to_search_page();
        }
    }
}

void MainWindow::process_show_more_images_done()
{
    if(img_search_){
        qDebug()<<__func__<<":get_page_link start";
        img_search_->get_page_link([this](QStringList const &links)
        {
            set_enabled_main_window_except_stop(true);
            setMaximumSize(default_max_size_);
            setMinimumSize(default_min_size_);
            QMessageBox::information(this, tr("Auto scroll end"),
                                     tr("Found %1 images."
                                        "<p>Press <img src = ':/icons/scroll.png' style='vertical-align:middle' /> "
                                        "or scroll manually if you want to scroll further,"
                                        "press <img src = ':/icons/download.png' style='vertical-align:middle' /> "
                                        "if you want to download the images</p>, press "
                                        "<img src = ':/icons/settings.png' style='vertical-align:middle' /> if "
                                        "you want to configure your options").arg(links.size()));
        });
    }
}

void MainWindow::set_enabled_main_window_except_stop(bool value)
{
    centralWidget()->setEnabled(value);
    ui->actionDownload->setEnabled(value);
    ui->actionHome->setEnabled(value);
    ui->actionInfo->setEnabled(value);
    ui->actionNew->setEnabled(value);
    ui->actionRefresh->setEnabled(value);
    ui->actionScroll->setEnabled(value);
    ui->actionSettings->setEnabled(value);
    ui->actionStop->setEnabled(!value);
}

void MainWindow::refresh_window()
{
    qDebug()<<__func__<<":"<<img_links_map_.size()<<","<<big_img_links_.size();
    if(!download_finished_ && img_links_map_.empty() && big_img_links_.empty()){
        ui->labelProgress->setVisible(false);
        ui->progressBar->setVisible(false);
        set_enabled_main_window_except_stop(true);
        img_search_->go_to_search_page();
        statusBar()->showMessage("");
        download_finished_ = true;

        QMessageBox::information(this, tr("Download finished"),
                                 tr("Total download %1\n"
                                    "Success %2 , Fail %3\n"
                                    "big images %4, small images %5").arg(statistic_.total_download_).
                                 arg(statistic_.success()).arg(statistic_.fail()).
                                 arg(statistic_.big_img_download_).arg(statistic_.small_img_download_));
    }
}

void MainWindow::download_small_img(QString const &save_as,
                                    std::tuple<QString, QString, link_choice> const &img_info)
{
    QFile::remove(save_as);
    if(std::get<2>(img_info) == link_choice::big && std::get<0>(img_info) != std::get<1>(img_info) &&
            !std::get<1>(img_info).isEmpty()){
        download_img(std::make_tuple(std::get<0>(img_info), std::get<1>(img_info), link_choice::small));
    }else{
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        download_next_image();
    }
}

void MainWindow::download_finished(download_img_task task)
{
    qDebug()<<__func__<<":"<<task->get_unique_id()<<":"<<task->get_network_error_code();
    qDebug()<<"save as:"<<task->get_save_as();
    auto it = img_links_map_.find(task->get_unique_id());
    if(it != std::end(img_links_map_)){
        auto img_info = it->second;
        img_links_map_.erase(it);
        if(task->get_is_timeout()){
            qDebug()<<__func__<<":"<<task->get_save_as()<<","<<task->get_url()<<": timeout";
            QFile::remove(task->get_save_as());
            download_img(std::move(img_info));
            return;
        }

        QImageReader img(task->get_save_as());
        img.setDecideFormatFromContent(true);
        if(task->get_network_error_code() == QNetworkReply::NoError && img.canRead()){
            QFileInfo file_info(task->get_save_as());
            QFile::rename(task->get_save_as(),
                          file_info.absolutePath() + "/" + file_info.completeBaseName() +
                          "." + img.format());
            qDebug()<<"can save image choice:"<<(int)std::get<2>(img_info);
            ui->progressBar->setValue(ui->progressBar->value() + 1);
            qDebug()<<"set progressBar value:"<<ui->progressBar->value();
            if(std::get<2>(img_info) == link_choice::big){
                ++statistic_.big_img_download_;
            }else{
                ++statistic_.small_img_download_;
            }
            download_next_image();
        }else{
            qDebug()<<"cannot save image choice:"<<(int)std::get<2>(img_info);
            download_small_img(task->get_save_as(), std::move(img_info));
        }

        refresh_window();
    }else{
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        QFile::remove(task->get_save_as());
        download_next_image();
    }
}

void MainWindow::download_img(img_links_map_value info)
{
    QString const &img_link = std::get<2>(info) == link_choice::big ?
                std::get<0>(info) : std::get<1>(info);
    QNetworkRequest const request = create_img_download_request(img_link,
                                                                general_settings_->get_search_by());
    auto const unique_id = downloader_->append(request, general_settings_->get_save_at(),
                                               global_constant::network_reply_timeout());
    img_links_map_.emplace(unique_id, std::move(info));
    downloader_->start_download_task(unique_id);
}

void MainWindow::download_progress(download_img_task task, qint64 bytesReceived, qint64 bytesTotal)
{    
    qDebug()<<__func__<<":"<<task->get_unique_id()<<":"<<bytesReceived<<":"<<bytesTotal;
    if(bytesTotal > 0){
        statusBar()->showMessage(tr("%1 : %2/%3").arg(QFileInfo(task->get_save_as()).fileName()).
                                 arg(bytesReceived).arg(bytesTotal));
    }else{
        statusBar()->showMessage(tr("%1 : %2").arg(QFileInfo(task->get_save_as()).fileName()).
                                 arg(bytesReceived));
    }
}

void MainWindow::on_actionScroll_triggered()
{
    set_enabled_main_window_except_stop(false);
    setMaximumSize(size());
    img_search_->show_more_images(1000);
}

void MainWindow::download_next_image()
{
    if(!big_img_links_.empty()){
        auto const big_img = big_img_links_[0];
        auto const small_img = small_img_links_[0];
        if(!is_download_finished()){
            big_img_links_.pop_front();
            small_img_links_.pop_front();
            ui->webView->page()->load(small_img);
            found_img_link(big_img, small_img);
        }else{
            big_img_links_.clear();
            small_img_links_.clear();
            img_links_map_.clear();
            refresh_window();
        }
    }
}

void MainWindow::on_actionDownload_triggered()
{
    set_enabled_main_window_except_stop(false);
    ui->actionStop->setEnabled(false);
    img_search_->get_imgs_link_from_second_page([this](QStringList const &big_img_link, QStringList const &small_img_link)
    {
        statistic_.clear();
        img_links_map_.clear();
        download_finished_ = false;
        statistic_.total_download_ = std::min(static_cast<size_t>(big_img_link.size()),
                                              static_cast<size_t>(general_settings_->get_max_download_img()));
        qDebug()<<"big img links size:"<<big_img_link.size()<<",max download size:"
               <<general_settings_->get_max_download_img();
        ui->labelProgress->setVisible(true);
        ui->progressBar->setVisible(true);
        ui->progressBar->setRange(0, static_cast<int>(statistic_.total_download_));
        ui->progressBar->setValue(0);
        qDebug()<<"progress bar min:"<<ui->progressBar->minimum()<<",max:"<<ui->progressBar->maximum();
        big_img_links_ = big_img_link;
        small_img_links_ = small_img_link;
        download_next_image();
    });
}

void MainWindow::on_actionSettings_triggered()
{
    general_settings_->exec();
}

void MainWindow::on_actionRefresh_triggered()
{
    ui->webView->page()->load(ui->webView->page()->url());
}

void MainWindow::on_actionHome_triggered()
{
    img_search_->go_to_search_page();
}

void MainWindow::download_img_error(download_img_task task, const QString &error_msg)
{
    qDebug()<<__func__<<":"<<task->get_unique_id()<<":"<<error_msg;
}

void MainWindow::on_actionInfo_triggered()
{
    info_dialog().exec();
}

void MainWindow::on_actionStop_triggered()
{
    img_search_->stop_show_more_images();
}

void MainWindow::download_statistic::clear()
{
    big_img_download_ = 0;
    small_img_download_ = 0;
    total_download_ = 0;
}

size_t MainWindow::download_statistic::fail() const
{
    return total_download_ - success();
}

size_t MainWindow::download_statistic::success() const
{
    return big_img_download_ + small_img_download_;
}
