﻿#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "core/image_search_error.hpp"

#include <QMainWindow>
#include <QNetworkReply>

#include <map>
#include <memory>

#include <qt_enhance/network/download_supervisor.hpp>

namespace Ui {
class MainWindow;
}

class general_settings;
class info_dialog;
class image_downloader;
class image_search;
class settings_manager;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:    
    void on_actionDownload_triggered();

    void on_actionSettings_triggered();

    void on_actionRefresh_triggered();

    void on_actionHome_triggered();

    void on_actionInfo_triggered();

    void on_actionStop_triggered();

    void on_actionShowMoreImage_triggered();

    void on_actionNew_triggered();

    void on_actionBrowserTop_triggered();

    void on_actionBrowserBottom_triggered();

private:
    using download_img_task = std::shared_ptr<qte::net::download_supervisor::download_task>;

    void change_search_engine();
    void check_new_version();
    void create_search_engine(QString const &target);
    void download_progress(download_img_task task,
                           qint64 bytesReceived, qint64 bytesTotal);    
    void settings_manager_ok_clicked();
    void init_connection();
    bool is_download_finished() const;    
    void process_go_to_gallery_page();
    void process_go_to_search_page();    
    void process_image_search_error(image_search_error::error error);
    void process_show_more_images_done();
    void refresh_window();
    void remove_file(QString const &debug_msg, download_img_task task);
    void set_enabled_main_window_except_stop(bool value);
    void set_show_gallery_tutorial(int state);
    void update_to_new_version();

    Ui::MainWindow *ui;
    QSize default_max_size_;
    QSize default_min_size_;    
    image_downloader *img_downloader_;            
    image_search *img_search_;
    std::unique_ptr<info_dialog> info_dialog_;
    settings_manager *settings_manager_;
};

#endif // MAINWINDOW_HPP
