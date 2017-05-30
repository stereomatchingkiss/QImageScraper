#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "general_settings.hpp"

#include "core/bing_image_search.hpp"
#include "core/google_image_search.hpp"
#include "core/yahoo_image_search.hpp"
#include "core/global_constant.hpp"
#include "core/utility.hpp"

#include "ui/info_dialog.hpp"

#include <QsLog.h>

#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QSettings>
#include <QSizeGrip>

#include <ctime>
#include <future>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    default_max_size_{maximumSize()},
    default_min_size_{minimumSize()},    
    downloader_{new qte::net::download_supervisor(this)},
    general_settings_{new general_settings(this)},
    img_search_{nullptr}
{
    ui->setupUi(this);
    general_settings_ok_clicked();
    ui->actionDownload->setEnabled(false);
    ui->actionShowMoreImage->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionNew->setVisible(false);

    ui->labelProgress->setVisible(false);
    ui->progressBar->setVisible(false);

    QSettings settings;
    if(settings.contains("geometry")){
        restoreGeometry(settings.value("geometry").toByteArray());
    }

    settings.setValue("version", "1.1");

    qsrand(std::time(0));

    setMinimumSize(size());

    using namespace qte::net;

    connect(downloader_, &download_supervisor::error, this, &MainWindow::download_img_error);
    connect(downloader_, &download_supervisor::download_finished, this, &MainWindow::download_finished);
    connect(downloader_, &download_supervisor::download_progress, this, &MainWindow::download_progress);
    connect(general_settings_, &general_settings::ok_clicked, this, &MainWindow::general_settings_ok_clicked);

    connect(general_settings_, &general_settings::cannot_create_save_dir,
            [this](QString const &dir, QString const &write_able_path)
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot create directory to save image %1, please choose a new directory, "
                                                   "if not the images will save at %2").arg(dir).arg(write_able_path));
    });

    check_new_version();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());

    delete ui;
}

void MainWindow::found_img_link(const QString &big_img_link, const QString &small_img_link)
{
    static size_t img_link_count = 0;
    QLOG_INFO()<<"found image link count:"<<img_link_count++;
    download_img(std::make_tuple(big_img_link, small_img_link, link_choice::big));
}

void MainWindow::change_search_engine()
{            
    if(img_search_){
        if(general_settings_->search_by_changed()){
            img_search_->get_search_target([this](QString const &target)
            {
                create_search_engine(target);
            });
        }
    }else{
        create_search_engine("");
    }
}

void MainWindow::check_new_version()
{
    auto *manager = downloader_->get_network_manager();
    auto *reply = manager->get(QNetworkRequest(QUrl("https://dl.dropboxusercontent.com/s/1ajwbdrnv3omdtr/version_num.txt?dl=0")));
    connect(reply, static_cast<void(QNetworkReply::*)()>(&QNetworkReply::finished),
            reply, &QNetworkReply::deleteLater);
    connect(reply, static_cast<void(QNetworkReply::*)()>(&QNetworkReply::finished),
            this, &MainWindow::update_to_new_version);
}

void MainWindow::create_search_engine(const QString &target)
{
    if(img_search_){
        img_search_->deleteLater();
    }

    if(general_settings_->get_search_by() == global_constant::bing_search_name()){
        img_search_ = new bing_image_search(*ui->webView->page(), this);
    }else if(general_settings_->get_search_by() == global_constant::yahoo_search_name()){
        img_search_ = new yahoo_image_search(*ui->webView->page(), this);
    }else{
        img_search_ = new google_image_search(*ui->webView->page(), this);
    }

    connect(img_search_, &image_search::go_to_search_page_done, this, &MainWindow::process_go_to_search_page);
    connect(img_search_, &image_search::go_to_gallery_page_done, this, &MainWindow::process_go_to_gallery_page);
    connect(img_search_, &image_search::show_more_images_done, this, &MainWindow::process_show_more_images_done);
    connect(img_search_, &image_search::search_error, this, &MainWindow::process_image_search_error);

    if(target.isEmpty()){
        QLOG_INFO()<<"target is empty";
        img_search_->go_to_search_page();
    }else{
        QLOG_INFO()<<"target is not empty:"<<target;
        img_search_->go_to_gallery_page(target);
    }
}

void MainWindow::general_settings_ok_clicked()
{    
    change_search_engine();
}

bool MainWindow::is_download_finished() const
{
    return ui->progressBar->value() == ui->progressBar->maximum();
}

void MainWindow::process_go_to_search_page()
{
    ui->actionShowMoreImage->setEnabled(false);
    ui->actionDownload->setEnabled(false);
    ui->actionStop->setEnabled(false);
}

void MainWindow::process_go_to_gallery_page()
{
    ui->actionShowMoreImage->setEnabled(true);
    ui->actionDownload->setEnabled(true);
    ui->actionStop->setEnabled(false);

    QSettings settings;
    bool show_tutorial = true;
    if(settings.contains("tutorial/show_gallery_basic")){
        show_tutorial = settings.value("tutorial/show_gallery_basic").toBool();
    }

    if(show_tutorial){
        QMessageBox msgbox;
        QCheckBox *cb = new QCheckBox(tr("Do not show this again"));
        msgbox.setText(tr("Press %1 if you want to show more images.<br>"
                          "Press %2 if you want to start download.<br>"
                          "Press %3 if you want to select search engine and do other settings.<br>"
                          "Press %4 if you want to back to the first page.<br>"
                          "Press %5 if you want to reload this page.").
                       arg("<img src = ':/icons/show_more_image.png' style='vertical-align:middle' />").
                       arg("<img src = ':/icons/download.png' style='vertical-align:middle' />").
                       arg("<img src = ':/icons/settings.png' style='vertical-align:middle' />").
                       arg("<img src = ':/icons/home.png' style='vertical-align:middle' />").
                       arg("<img src = ':/icons/refresh.png' style='vertical-align:middle' />"));
        msgbox.setWindowTitle(tr("Tutorial"));
        msgbox.setIcon(QMessageBox::Icon::Information);
        msgbox.setDefaultButton(QMessageBox::Ok);
        msgbox.setCheckBox(cb);

        connect(cb, &QCheckBox::stateChanged, this, &MainWindow::set_show_gallery_tutorial);

        msgbox.exec();
    }
}

void MainWindow::process_image_search_error(image_search_error::error error)
{
    if(error == image_search_error::error::load_page_error){
        if(!is_download_finished()){
            static int reload_small_img = 0;
            QLOG_INFO()<<"reload small img:"<<reload_small_img;
            QTimer::singleShot(qrand() % 4000 + 2000,
                               [this](){ui->webView->page()->load(ui->webView->page()->url());});
        }else{
            img_search_->go_to_search_page();
        }
    }
}

void MainWindow::process_show_more_images_done()
{
    if(img_search_){
        QLOG_INFO()<<__func__<<":get_page_link start";
        img_search_->get_page_link([this](QStringList const &links)
        {
            set_enabled_main_window_except_stop(true);
            setMaximumSize(default_max_size_);
            setMinimumSize(default_min_size_);
            QMessageBox::information(this, tr("Auto scroll end"),
                                     tr("Found %1 images<br>"
                                        "Press %2 to scroll the page automatic<br>"
                                        "press %3 if you want to download the images<br> "
                                        "press %4 if you want to configure your options").
                                     arg(links.size()).
                                     arg("<img src = ':/icons/show_more_image.png' style='vertical-align:middle' />").
                                     arg("<img src = ':/icons/download.png' style='vertical-align:middle' />").
                                     arg("<img src = ':/icons/settings.png' style='vertical-align:middle' />"));
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
    ui->actionShowMoreImage->setEnabled(value);
    ui->actionSettings->setEnabled(value);
    ui->actionStop->setEnabled(!value);
}

void MainWindow::set_show_gallery_tutorial(int state)
{
    if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
        QSettings settings;
        settings.setValue("tutorial/show_gallery_basic", true);
    }
}

void MainWindow::update_to_new_version()
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        QLOG_INFO()<<"check new version:"<<reply->error();
        QByteArray const results = reply->readAll();
        QLOG_INFO()<<"version number:"<<results<<","<<QSettings().value("version").toString();
        if(results > QSettings().value("version").toString()){
            ui->actionNew->setVisible(true);
            QMessageBox::information(this, tr("Update"),
                                     tr("Press %1 to install new version").
                                     arg("<img src = ':/icons/new.png' style='vertical-align:middle' />"));
        }
    }
}

void MainWindow::refresh_window()
{
    QLOG_INFO()<<__func__<<":"<<img_links_map_.size()<<","<<big_img_links_.size();
    if(is_download_finished() || (img_links_map_.empty() && big_img_links_.empty())){
        ui->labelProgress->setVisible(false);
        ui->progressBar->setVisible(false);
        set_enabled_main_window_except_stop(true);        

        int const ret = QMessageBox::information(this, tr("Download finished"),
                                                 tr("Total download %1\n"
                                                    "Success %2. Fail %3\n"
                                                    "Big images %4. Thumbnail %5").arg(statistic_.total_download_).
                                                 arg(statistic_.success()).arg(statistic_.fail()).
                                                 arg(statistic_.big_img_download_).arg(statistic_.small_img_download_));

        auto future = std::async(std::launch::async, [this]()
        {
            for(auto const &pair: img_cannot_rename_){
                bool const can_rename = QFile::rename(pair.first, pair.second);
                QLOG_INFO()<<"can rename in refresh window:"<<can_rename;
            }
            for(auto const &img : img_cannot_remove_){
                bool const can_remove = QFile::remove(img);
                QLOG_INFO()<<"can remove in refresh window:"<<can_remove;
            }
            img_cannot_remove_.clear();
            img_cannot_rename_.clear();
        });

        if(ret == QMessageBox::Ok){
            statusBar()->showMessage("");
            img_search_->go_to_search_page();
        }
        future.get();
    }
}

void MainWindow::remove_file(const QString &debug_msg, MainWindow::download_img_task task)
{
    bool const can_remove = QFile::remove(task->get_save_as());
    QLOG_INFO()<<__func__<<":"<<debug_msg<<task->get_save_as()<<":"<<can_remove;
    if(!can_remove){
        img_cannot_remove_.push_back(task->get_save_as());
    }
}

void MainWindow::download_small_img(img_links_map_value img_info)
{    
    if(std::get<2>(img_info) == link_choice::big && std::get<0>(img_info) != std::get<1>(img_info) &&
            !std::get<1>(img_info).isEmpty()){
        download_img(std::make_tuple(std::get<0>(img_info), std::get<1>(img_info), link_choice::small));
    }else{
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        download_next_image();
    }
}

void MainWindow::process_download_image(download_img_task task, img_links_map_value img_info)
{
    QImageReader img(task->get_save_as());
    img.setDecideFormatFromContent(true);
    if(task->get_network_error_code() == QNetworkReply::NoError && img.canRead()){
        QFileInfo file_info(task->get_save_as());
        bool const change_suffix = file_info.suffix() != img.format();
        QLOG_INFO()<<"can save image choice:"<<(int)std::get<2>(img_info);
        if(change_suffix){
            QString const new_name = file_info.absolutePath() + "/" +
                    file_info.completeBaseName() + "." + img.format();
            bool const can_rename_img = QFile::rename(task->get_save_as(), new_name);
            //bool const can_rename_img = task->rename(file_info.completeBaseName() + "." + img.format());
            QLOG_INFO()<<"QFile::rename, can rename image from:"<<task->get_save_as()<<", to:"<<
                         new_name<<":"<<img.format()<<":"<<can_rename_img;
            if(!can_rename_img){
                img_cannot_rename_.emplace_back(task->get_save_as(), new_name);
            }
        }
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        QLOG_INFO()<<"set progressBar value:"<<ui->progressBar->value();
        if(std::get<2>(img_info) == link_choice::big){
            ++statistic_.big_img_download_;
        }else{
            ++statistic_.small_img_download_;
        }
        download_next_image();
    }else{
        QLOG_INFO()<<"cannot save image choice:"<<(int)std::get<2>(img_info);
        //bool const can_remove = QFile::remove(task->get_save_as());
        //QLOG_INFO()<<__func__<<":big image, can remove file:"<<task->get_save_as()<<":"<<can_remove;
        remove_file("big image, can remove file:", task);
        download_small_img(std::move(img_info));
    }
}

void MainWindow::download_finished(download_img_task task)
{
    QLOG_INFO()<<__func__<<":"<<task->get_unique_id()<<":"<<task->get_network_error_code();
    QLOG_INFO()<<__func__<<"save as:"<<task->get_save_as()<<",url:"<<task->get_url();    
    auto it = img_links_map_.find(task->get_unique_id());
    if(it != std::end(img_links_map_)){
        auto img_info = it->second;
        img_links_map_.erase(it);
        if(task->get_is_timeout()){
            QLOG_INFO()<<__func__<<":"<<task->get_save_as()<<","<<task->get_url()<<": timeout";
            //bool const can_remove = QFile::remove(task->get_save_as());
            //QLOG_INFO()<<__func__<<":time out, can remove file:"<<can_remove;
            statusBar()->showMessage(tr("Waiting reply from the server, please give some patient"));
            remove_file("time out issue:", task);
            download_img(std::move(img_info));
            return;
        }

        process_download_image(task, std::move(img_info));

        refresh_window();
    }else{
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        //bool const can_remove = QFile::remove(task->get_save_as());
        //QLOG_INFO()<<__func__<<":cannot find id in img_links_map_, can remove file:"<<can_remove;
        remove_file("cannot find id in img_links_map, can remove file:", task);
        download_next_image();
    }
}

void MainWindow::download_img(img_links_map_value info)
{
    QString const &img_link = std::get<2>(info) == link_choice::big ?
                std::get<0>(info) : std::get<1>(info);
    QNetworkRequest const request = create_img_download_request(img_link);
    auto const unique_id = downloader_->append(request, general_settings_->get_save_at(),
                                               global_constant::network_reply_timeout());
    img_links_map_.emplace(unique_id, std::move(info));
    QTimer::singleShot(qrand() % 1000 + 500, [this, unique_id](){downloader_->start_download_task(unique_id);});
}

void MainWindow::download_progress(download_img_task task, qint64 bytesReceived, qint64 bytesTotal)
{    
    QLOG_INFO()<<__func__<<":"<<task->get_unique_id()<<":"<<bytesReceived<<":"<<bytesTotal;
    if(bytesTotal > 0){
        statusBar()->showMessage(tr("%1 : %2/%3").arg(QFileInfo(task->get_save_as()).fileName()).
                                 arg(bytesReceived).arg(bytesTotal));
    }else{
        statusBar()->showMessage(tr("%1 : %2").arg(QFileInfo(task->get_save_as()).fileName()).
                                 arg(bytesReceived));
    }
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
    img_search_->get_imgs_link_from_gallery_page([this](QStringList const &big_img_link, QStringList const &small_img_link)
    {
        statistic_.clear();
        img_links_map_.clear();        
        statistic_.total_download_ = std::min(static_cast<size_t>(big_img_link.size()),
                                              static_cast<size_t>(general_settings_->get_max_download_img()));
        QLOG_INFO()<<"big img links size:"<<big_img_link.size()<<",max download size:"
                  <<general_settings_->get_max_download_img();
        ui->labelProgress->setVisible(true);
        ui->progressBar->setVisible(true);
        ui->progressBar->setRange(0, static_cast<int>(statistic_.total_download_));
        ui->progressBar->setValue(0);
        QLOG_INFO()<<"progress bar min:"<<ui->progressBar->minimum()<<",max:"<<ui->progressBar->maximum();
        big_img_links_ = big_img_link;
        small_img_links_ = small_img_link;
        img_cannot_remove_.clear();
        img_cannot_rename_.clear();
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
    QLOG_ERROR()<<__func__<<":"<<task->get_unique_id()<<":"<<error_msg;
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

void MainWindow::on_actionShowMoreImage_triggered()
{
    set_enabled_main_window_except_stop(false);
    setMaximumSize(size());
    img_search_->show_more_images(general_settings_->get_max_download_img());
}

void MainWindow::on_actionNew_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/stereomatchingkiss/QImageScraper/blob/master/VERSION_INFO.md"));
}
