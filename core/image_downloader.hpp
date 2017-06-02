#ifndef IMAGE_DOWNLOADER_HPP
#define IMAGE_DOWNLOADER_HPP

#include <qt_enhance/network/download_supervisor.hpp>

#include <QNetworkProxy>
#include <QObject>

#include <memory>

class image_downloader : public QObject
{
    Q_OBJECT

    using download_img_task = std::shared_ptr<qte::net::download_supervisor::download_task>;
public:
    enum class download_state
    {
        normal,
        show_big_img_by_web_view
    };

    struct download_statistic
    {
        void clear();

        size_t fail() const;
        size_t success() const;

        size_t big_img_download_ = 0;
        size_t downloaded_file_ = 0;
        size_t small_img_download_ = 0;
        size_t total_download_ = 0;
    };

    explicit image_downloader(QObject *parent = nullptr);

    void download_next_image();
    void download_web_view_img();
    download_state get_download_state() const;
    QNetworkAccessManager* get_network_manager() const;
    download_statistic get_statistic() const;
    bool image_links_empty() const;    
    void set_download_request(QStringList big_image_links, QStringList small_image_links,
                              size_t max_download, QString const &save_at);    

signals:
    void download_progress(download_img_task task,
                           qint64 bytesReceived, qint64 bytesTotal);
    void increment_progress();
    void load_image(QString const &link);
    void set_statusbar_msg(QString const &msg);
    void refresh_window();

private:
    enum class link_choice
    {
        big,
        small
    };    

    struct img_links_map_value
    {
        img_links_map_value();
        img_links_map_value(QString big_img_link, QString small_img_link, link_choice choice,
                            size_t retry_num = 0);
        QString big_img_link_;
        link_choice choice_;
        size_t retry_num_;
        QString small_img_link_;
    };

    std::vector<QNetworkProxy> create_proxy() const;
    void download_finished(download_img_task task);
    void download_image(img_links_map_value info);
    void download_image_error(download_img_task task, QString const &error_msg);
    void download_small_img(img_links_map_value img_info);    
    QString get_valid_image_name(QString const &save_as, QString const &img_format);
    void increase_progress();
    void process_download_image(download_img_task task, img_links_map_value img_info);
    bool reach_download_target() const;
    bool remove_file(QString const &debug_msg, download_img_task task) const;
    void start_download(QString const &big_img_link, QString const &small_img_link);

    QStringList big_img_links_;
    download_state download_state_;
    qte::net::download_supervisor *downloader_;
    std::map<size_t, img_links_map_value> img_links_map_;
    std::vector<QNetworkProxy> proxy_list_;
    QString save_at_;
    QStringList small_img_links_;
    download_statistic statistic_;    
};

#endif // IMG_DOWNLOADER_HPP
