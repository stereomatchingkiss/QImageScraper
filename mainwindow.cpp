#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "core/adobe_stock_image_search.hpp"
#include "core/bing_image_search.hpp"
#include "core/deposit_photo_image_search.hpp"
#include "core/dream_time_image_search.hpp"
#include "core/google_image_search.hpp"
#include "core/image_downloader.hpp"
#include "core/pexels_image_search.hpp"
#include "core/shutter_stock_image_search.hpp"
#include "core/yahoo_image_search.hpp"
#include "core/global_constant.hpp"
#include "core/utility.hpp"

#include "ui/info_dialog.hpp"
#include "ui/general_settings.hpp"
#include "ui/proxy_settings.hpp"
#include "ui/settings_manager.hpp"

#include <qt_enhance/utility/qte_utility.hpp>

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    default_max_size_{maximumSize()},
    default_min_size_{minimumSize()},
    img_downloader_{new image_downloader{this}},
    img_search_{nullptr},
    settings_manager_{new settings_manager{this}}
{    
    ui->setupUi(this);
    settings_manager_ok_clicked();
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
    settings.setValue("version", "1.4");

    qsrand(std::time(nullptr));

    init_connection();
    check_new_version();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());

    delete ui;
}

void MainWindow::change_search_engine()
{            
    QLOG_INFO()<<__func__;
    if(img_search_){
        if(settings_manager_->get_general_settings().search_by_changed()){
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
    auto *manager = img_downloader_->get_network_manager();
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

    QString const search_engine = settings_manager_->get_general_settings().get_search_by();
    if(search_engine == global_constant::adobe_stock_name()){
        QLOG_INFO()<<__func__<<":create adobe engine";
        img_search_ = new adobe_stock_image_search(*ui->webView->page(), this);
    }else if(search_engine == global_constant::bing_search_name()){
        QLOG_INFO()<<__func__<<":create bing engine";
        img_search_ = new bing_image_search(*ui->webView->page(), this);
    }else if(search_engine == global_constant::deposit_photo_name()){
        QLOG_INFO()<<__func__<<":create deposit engine";
        img_search_ = new deposit_photo_image_search(*ui->webView->page(), this);
    }else if(search_engine == global_constant::dream_time_name()){
        QLOG_INFO()<<__func__<<":create dream engine";
        img_search_ = new dream_time_image_search(*ui->webView->page(), this);
    }else if(search_engine == global_constant::yahoo_search_name()){
        QLOG_INFO()<<__func__<<":create yahoo engine";
        img_search_ = new yahoo_image_search(*ui->webView->page(), this);
    }else if(search_engine == global_constant::shutter_stock_name()){
        QLOG_INFO()<<__func__<<":create shutter stock engine";
        img_search_ = new shutter_stock_image_search(*ui->webView->page(), this);
    }else if(search_engine == global_constant::pexels_search_name()){
        img_search_ = new pexels_image_search(*ui->webView->page(), this);
    }else{
        QLOG_INFO()<<__func__<<":create bing engine";
        img_search_ = new bing_image_search(*ui->webView->page(), this);
    }

    connect(img_search_, &image_search::go_to_search_page_done, this, &MainWindow::process_go_to_search_page);
    connect(img_search_, &image_search::go_to_gallery_page_done, this, &MainWindow::process_go_to_gallery_page);
    connect(img_search_, &image_search::show_more_images_done, this, &MainWindow::process_show_more_images_done);
    connect(img_search_, &image_search::search_error, this, &MainWindow::process_image_search_error);

    if(target.isEmpty()){
        QLOG_INFO()<<"target is empty";
        img_search_->go_to_search_page();
        QLOG_INFO()<<"go to serach page";
    }else{
        QLOG_INFO()<<"target is not empty:"<<target;
        img_search_->go_to_gallery_page(target);
        QLOG_INFO()<<"go to gallery page";
    }
}

void MainWindow::settings_manager_ok_clicked()
{    
    change_search_engine();
}

void MainWindow::init_connection()
{    
    connect(settings_manager_, &settings_manager::ok_clicked, this, &MainWindow::settings_manager_ok_clicked);

    connect(img_downloader_, &image_downloader::increment_progress, [this]()
    {
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        QLOG_INFO()<<"current progress:"<<ui->progressBar->value();
    });
    connect(img_downloader_, &image_downloader::set_statusbar_msg, [this](QString const &msg)
    {
        ui->statusBar->showMessage(msg);
    });
    connect(img_downloader_, &image_downloader::refresh_window, this, &MainWindow::refresh_window);
    connect(img_downloader_, &image_downloader::download_progress, this, &MainWindow::download_progress);
    connect(img_downloader_, &image_downloader::load_image, [this](QString const &url)
    {
        if(img_search_){
            img_search_->load(url);
        }
    });
}


bool MainWindow::is_download_finished() const
{
    return ui->progressBar->value() == ui->progressBar->maximum();
}

void MainWindow::process_go_to_search_page()
{
    QLOG_INFO()<<__func__;
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
            QLOG_INFO()<<"reload small img";
            QTimer::singleShot(qrand() % 4000 + 2000,
                               [this](){ img_search_->reload(); });
        }else{
            img_search_->go_to_search_page();
        }
    }
}

void MainWindow::process_show_more_images_done()
{
    if(img_search_){
        QLOG_INFO()<<__func__<<":get_page_link start";
        img_search_->get_search_image_size([this](size_t search_img_size)
        {
            set_enabled_main_window_except_stop(true);
            setMaximumSize(default_max_size_);
            setMinimumSize(default_min_size_);
            QMessageBox::information(this, tr("Auto scroll end"),
                                     tr("Found %1 images<br>"
                                        "Press %2 to scroll the page automatic<br>"
                                        "press %3 if you want to download the images<br> "
                                        "press %4 if you want to configure your options<br>"
                                        "press %5 if you want to go to the top of the page").
                                     arg(search_img_size).
                                     arg("<img src = ':/icons/show_more_image.png' style='vertical-align:middle' />").
                                     arg("<img src = ':/icons/download.png' style='vertical-align:middle' />").
                                     arg("<img src = ':/icons/settings.png' style='vertical-align:middle' />").
                                     arg("<img src = ':/icons/browser_top.png' style='vertical-align:middle' />"));
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
    ui->actionBrowserTop->setEnabled(value);
    ui->actionBrowserBottom->setEnabled(value);
}

void MainWindow::set_show_gallery_tutorial(int state)
{
    if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
        QSettings settings;
        settings.setValue("tutorial/show_gallery_basic", false);
    }
}

void MainWindow::update_to_new_version()
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        QLOG_INFO()<<"check new version:"<<reply->error();
        QString const results = QString(reply->readAll()).trimmed();
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
    if(is_download_finished() || img_downloader_->image_links_empty()){
        QLOG_INFO()<<__func__<<":"<<ui->progressBar->value();
        ui->labelProgress->setVisible(false);
        ui->progressBar->setVisible(false);
        set_enabled_main_window_except_stop(true);

        auto const statistic = img_downloader_->get_statistic();
        int const ret = QMessageBox::information(this, tr("Download finished"),
                                                 tr("Total download %1\n"
                                                    "Success %2. Fail %3\n"
                                                    "Big images %4. Thumbnail %5").arg(statistic.total_download_).
                                                 arg(statistic.success()).arg(statistic.fail()).
                                                 arg(statistic.big_img_download_).arg(statistic.small_img_download_));

        if(ret == QMessageBox::Ok){
            statusBar()->showMessage("");
            ui->webView->page()->runJavaScript("window.scrollTo(0,0)");
        }
    }
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

void MainWindow::on_actionDownload_triggered()
{
    set_enabled_main_window_except_stop(false);
    ui->actionStop->setEnabled(false);
    img_search_->get_imgs_link_from_gallery_page([this](QStringList const &big_img_link,
                                                 QStringList const &small_img_link)
    {
        auto const total_download_ = std::min(static_cast<size_t>(big_img_link.size()),
                                              static_cast<size_t>(settings_manager_->get_general_settings().
                                                                  get_max_download_img()));
        QLOG_INFO()<<"big img links size:"<<big_img_link.size()<<",max download size:"
                  <<settings_manager_->get_general_settings().get_max_download_img();
        //QLOG_INFO()<<big_img_link;
        //setEnabled(true);
        ui->labelProgress->setVisible(true);
        ui->progressBar->setVisible(true);
        ui->progressBar->setRange(0, static_cast<int>(total_download_));
        ui->progressBar->setValue(0);
        QLOG_INFO()<<"progress bar min:"<<ui->progressBar->minimum()<<",max:"<<ui->progressBar->maximum();
        std::vector<QNetworkProxy> proxies;
        proxy_settings::proxy_state const pstate = settings_manager_->get_proxy_settings().state();
        if(pstate == proxy_settings::proxy_state::manual_proxy){
            proxies = settings_manager_->get_proxy_settings().get_proxies();
            img_downloader_->set_manual_proxy(proxies);
        }
        img_downloader_->set_proxy_state(static_cast<int>(pstate));
        img_downloader_->set_download_request(big_img_link, small_img_link, total_download_,
                                              settings_manager_->get_general_settings().get_save_at());
        img_downloader_->download_next_image();//*/
    });
}

void MainWindow::on_actionSettings_triggered()
{
    settings_manager_->exec();
}

void MainWindow::on_actionRefresh_triggered()
{
    ui->webView->page()->load(ui->webView->page()->url());
}

void MainWindow::on_actionHome_triggered()
{
    img_search_->go_to_search_page();
}

void MainWindow::on_actionInfo_triggered()
{
    info_dialog().exec();
}

void MainWindow::on_actionStop_triggered()
{
    img_search_->stop_show_more_images();
}

void MainWindow::on_actionShowMoreImage_triggered()
{
    set_enabled_main_window_except_stop(false);
    setMaximumSize(size());
    img_search_->show_more_images(static_cast<size_t>(
                                      settings_manager_->get_general_settings().get_max_download_img())
                                  );
}

void MainWindow::on_actionNew_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/stereomatchingkiss/QImageScraper/blob/master/VERSION_INFO.md"));
}

void MainWindow::on_actionBrowserTop_triggered()
{
    ui->webView->page()->runJavaScript("window.scrollTo(0,0)");
}

void MainWindow::on_actionBrowserBottom_triggered()
{
    ui->webView->page()->runJavaScript("window.scrollTo(0, document.body.scrollHeight)");
}
