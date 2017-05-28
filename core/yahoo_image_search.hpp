#ifndef YAHOO_IMAGE_SEARCH_HPP
#define YAHOO_IMAGE_SEARCH_HPP

#include "image_search.hpp"

class yahoo_image_search : public image_search
{
public:
    explicit yahoo_image_search(QWebEnginePage &page, QObject *parent = nullptr);

    /**
     * @brief this function is unfinished and will throw runtime_error  if called
     */
    void get_imgs_link(QString const &page_link,
                       std::function<void(QString const&, QString const&)> callback) override;
    void get_imgs_link_from_gallery_page(std::function<void(QStringList const&, QStringList const&)> callback) override;
    /**
     * @brief this function is unfinished and will throw runtime_error  if called
     */
    void get_page_link(std::function<void(QStringList const&)> callback) override;
    void go_to_search_page() override;
    void show_more_images(size_t max_search_size) override;
    void stop_show_more_images() override;

private:
    enum class state{
        to_search_page,
        to_gallery_page,
        show_more_images,
        get_img_link_from_gallery_page
    };

    void load_web_page_finished(bool ok) override;
    void parse_page_link(QString const &contents);
    void scroll_web_page();
    void scroll_web_page_impl();

    QStringList img_page_links_;
    size_t max_search_size_;
    std::function<void(QString const&, QString const&)> parse_img_link_callback_;
    size_t scroll_count_;
    size_t scroll_limit_;
    state state_;
    bool stop_scroll_page_;
};

#endif // YAHOO_IMAGE_SEARCH_HPP
