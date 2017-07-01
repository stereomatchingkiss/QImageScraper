#ifndef GOOGLE_IMAGE_SEARCH_HPP
#define GOOGLE_IMAGE_SEARCH_HPP

#include "image_search.hpp"

class google_image_search : public image_search
{
    Q_OBJECT
public:
    explicit google_image_search(QWebEnginePage &page, QObject *parent = nullptr);

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
        reload_url,
        load_url,
        to_search_page,
        to_gallery_page,
        show_more_images,
        get_img_link_from_gallery_page
    };

    QString decode_link_char(QString link);
    void load_web_page_finished(bool ok) override;    
    QStringList parse_page_link(QString const &contents) const;
    void scroll_web_page();

    size_t scroll_count_;
    size_t scroll_limit_;
    state state_;
    bool stop_scroll_page_;
};

#endif // GOOGLE_IMAGE_SEARCH_HPP
