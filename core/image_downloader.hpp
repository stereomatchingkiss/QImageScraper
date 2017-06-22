#ifndef IMAGE_DOWNLOADER_HPP
#define IMAGE_DOWNLOADER_HPP

#include <qt_enhance/network/download_supervisor.hpp>

#include <QNetworkProxy>
#include <QObject>

#include <memory>

class tor_controller;

class image_downloader : public QObject
{
    Q_OBJECT

    using download_img_task = std::shared_ptr<qte::net::download_supervisor::download_task>;
public:    
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

    /**
     * @This api will begin to download the image, before you call this api, remember to
     * set_download_request and proxy state.
     * @code
     * downloader->set_download_request(big_img_link, small_img_link, 1, save_at);
     * downloader->set_proxy_state(proxy_state);
     * if(proxy_state == proxy_settings::proxy_state::tor_proxy){
     *     downloader->set_tor_proxy(host, control_port, port, your_password);
     * }else if(proxy_state == proxy_settings::proxy_state::manual_proxy){
     *     downloader->set_manual_proxy(proxies);
     * }
     * downloader->download_next_image();
     * @endcode
     */
    void download_next_image();       
    QNetworkAccessManager* get_network_manager() const;
    download_statistic get_statistic() const;
    bool image_links_empty() const;

    /**
     * @brief setup images links, desire max download target and where to save the images
     * @param big_image_links links of the big image(image with original size) scraped by search engine
     * @param small_image_links thumbnails of big imag
     * @param max_download maximum image want to download, even big_image_links.size() > max_download,
     * downloader would download at most max_download size of image only
     * @param save_at self explained
     * @warning big_image_links.size() must equal to small_image_links.size()
     */
    void set_download_request(QStringList big_image_links, QStringList small_image_links,
                              size_t max_download, QString const &save_at);
    /**
     * @brief set the proxies info of manual proxies
     * @param proxy self explained
     */
    void set_manual_proxy(std::vector<QNetworkProxy> const &proxy);
    /**
     * @brief there are three states to set, please refer to enum class
     * proxy_settings::proxy_state(no_proxy, manual_proxy, tor_proxy)
     * @param state self explain, remmeber to cast the enum class to interger
     */
    void set_proxy_state(int state);    

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
                            size_t retry_num = 0, size_t timeout_retry_num_ = 0);
        QString big_img_link_;
        link_choice choice_;
        size_t retry_num_;        
        QString small_img_link_;        
        size_t timeout_retry_num_;
    };    

    bool can_download_image(download_img_task const &task, img_links_map_value const &img_info);    
    void download_finished(download_img_task task);
    void download_image(img_links_map_value info);
    void download_image_error(download_img_task task, QString const &error_msg);
    void download_small_img(img_links_map_value img_info);    
    QString get_valid_image_name(QString const &save_as, QString const &img_format);
    void increase_progress();
    void process_download_image(download_img_task task, img_links_map_value img_info);
    bool reach_download_target() const;
    bool remove_file(QString const &debug_msg, download_img_task task) const;
    void spawn_download_request(img_links_map_value info);
    void start_download(QString const &big_img_link, QString const &small_img_link);

    QStringList big_img_links_;    
    qte::net::download_supervisor *downloader_;
    image_downloader::img_links_map_value img_info_;
    std::map<size_t, img_links_map_value> img_links_map_;
    std::vector<QNetworkProxy> proxy_list_;
    int proxy_state_;
    QString save_at_;
    QStringList small_img_links_;
    download_statistic statistic_;        
};

#endif // IMG_DOWNLOADER_HPP
