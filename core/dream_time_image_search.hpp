#ifndef DREAM_TIME_IMAGE_SEARCH_HPP
#define DREAM_TIME_IMAGE_SEARCH_HPP


#include "image_search.hpp"

class dream_time_image_search : public image_search
{
    Q_OBJECT
public:
    explicit dream_time_image_search(QWebEnginePage &page, QObject *parent = nullptr);

    void get_search_image_size(std::function<void(size_t)> callback) override;
    void get_imgs_link_from_gallery_page(std::function<void(QStringList const&, QStringList const&)> callback) override;
    void get_search_target(std::function<void(QString const &)> callback) override;
    void go_to_gallery_page(QString const &target) override;
    void go_to_search_page() override;
    void load(QUrl const &url) override;
    void reload() override;
    void show_more_images(size_t max_search_size) override;
    void stop_show_more_images() override;

private:
    enum class state{
        load_url,
        reload_url,
        to_search_page,
        to_gallery_page,
        show_more_images,
        get_img_link_from_gallery_page
    };

    size_t get_current_page_num();
    void load_web_page_finished(bool ok) override;
    void parse_img_link(std::function<void(QString const&)> callback);
    void show_more_page();
    void to_next_page(size_t max_page);

    QStringList big_img_links_;
    size_t max_search_size_;    
    QStringList small_img_links_;
    state state_;
    bool stop_show_more_image_;    
};

#endif // DREAM_TIME_IMAGE_SEARCH_HPP
