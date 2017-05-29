#ifndef MAINWINDOW_HPP
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
class image_search;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:    
    void on_actionDownload_triggered();

    void on_actionSettings_triggered();

    void on_actionRefresh_triggered();

    void on_actionHome_triggered();

    void on_actionInfo_triggered();

    void on_actionStop_triggered();

    void on_actionShowMoreImage_triggered();

private:
    enum class link_choice
    {
        big,
        small
    };

    using download_img_task = std::shared_ptr<qte::net::download_supervisor::download_task>;
    using img_links_map_value = std::tuple<QString, QString, link_choice>;

    void change_search_engine();
    void check_new_version();
    void create_search_engine(QString const &target);
    void download_img_error(download_img_task task, QString const &error_msg);
    void download_finished(download_img_task task);
    void download_img(img_links_map_value info);
    void download_next_image();
    void download_progress(download_img_task task,
                           qint64 bytesReceived, qint64 bytesTotal);
    void download_small_img(QString const &save_as,
                            img_links_map_value const &img_info);    
    void found_img_link(QString const &big_img_link, QString const &small_img_link);
    void general_settings_ok_clicked();
    bool is_download_finished() const;
    void process_download_image(download_img_task task, img_links_map_value img_info);
    void process_go_to_gallery_page();
    void process_go_to_search_page();    
    void process_image_search_error(image_search_error::error error);
    void process_show_more_images_done();
    void refresh_window();
    void set_enabled_main_window_except_stop(bool value);
    void set_show_gallery_tutorial(int state);

    Ui::MainWindow *ui;

    struct download_statistic
    {
        void clear();

        size_t fail() const;
        size_t success() const;

        size_t big_img_download_ = 0;
        size_t small_img_download_ = 0;
        size_t total_download_ = 0;
    };

    QStringList big_img_links_;    
    QSize default_max_size_;
    QSize default_min_size_;
    bool download_finished_;
    qte::net::download_supervisor *downloader_;    
    general_settings *general_settings_;
    std::map<size_t, img_links_map_value> img_links_map_;
    image_search *img_search_;
    QStringList small_img_links_;
    download_statistic statistic_;        
};

#endif // MAINWINDOW_HPP
