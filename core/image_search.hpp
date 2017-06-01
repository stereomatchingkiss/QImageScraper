#ifndef IMAGE_SEARCH_HPP
#define IMAGE_SEARCH_HPP

#include "image_search_error.hpp"

#include <QObject>

#include <functional>

class QColor;
class QWebEnginePage;

/**
 * @brief Encapsulate process of scraping image link of
 * search engine.
 */
class image_search : public QObject
{
    Q_OBJECT
public:    
    /**
     * @param page The webpage of the search engine
     * @param parent self explain
     */
    explicit image_search(QWebEnginePage &page, QObject *parent = nullptr);

    /**
     * @brief Asynchronous method to get page links of second page(these pages
     * contain image links). Upon completion, result callback is called with the
     * page links.
     * @param callback Upon completion, result callback is called with the
     * page links.
     */
    virtual void get_page_link(std::function<void(QStringList const&)> callback) = 0;

    /**
     * @brief Asynchronous method to get image links by page link(this page
     * contain image links). Upon completion, result callback is called with the
     * image links.
     * @param page_link link of page which contain image links
     * @param callback Upon completion, result callback is called with the
     * image links(big image link, small image link).
     */
    virtual void get_imgs_link(QString const &page_link,
                               std::function<void(QStringList const&, QStringList const&)> callback) = 0;

    /**
     * @brief Asynchronous method to parse all of the image links from gallery page. Upon
     * completion, result callback is called with the image links
     * @param callback callback Upon completion, result callback is called with the
     * image links(big image links, small image links).
     */
    virtual void get_imgs_link_from_gallery_page(std::function<void(QStringList const&, QStringList const&)> callback) = 0;

    /**
     * @brief Asynchronous method to get the search target. Upon
     * completion, result callback is called with the search target
     * @param callback self explained
     */
    virtual void get_search_target(std::function<void(QString const &)> callback) = 0;

    /**
     * @brief Asynchronous method to get web view image. Upon
     * completion, result callback is called with the web view image
     * @param callback self explained
     * @warning this function is no re-entrant
     */
    virtual void get_web_view_image(std::function<void(QImage const&)> callback);

    /**
      * @brief go to the gallery page of the search engine. Will
      * emit signal "go_to_gallery_page_done" after second page is loaded.
      * @example Bing : "https://www.bing.com/images/search?q=" + target
      * Google : "https://www.google.co.in/search?q=" + searchtext + "&source=lnms&tbm=isch"
      */
    virtual void go_to_gallery_page(QString const &target ) = 0;

    /**
     * @brief Asynchronous method, go to the search page of the search engine. Will
     * emit signal "go_to_search_page_done" after search page is loaded.
     * @example Bing : https://www.bing.com/?scope=images&nr=1&FORM=NOFORM
     * Google : https://images.google.com/
     */
    virtual void go_to_search_page() = 0;

    /**
     * @brief load url
     * @param url self explained
     */
    virtual void load(QUrl const &url);

    /**
     * @brief reload current url
     */
    virtual void reload() = 0;

    /**
     * @brief Asynchronous method, let second page of the search engine show more image.
     * This function will emit signal "scroll_second_page_done" after scroll action
     * are all done; Will emit signal "second_page_scrolled" everytime
     * the page scroll(if oage scrolling action is trigger by scroll_second_page)
     * @param max_search_size size of the images want to scrape
     */
    virtual void show_more_images(size_t max_search_size) = 0;

    /**
     * @brief stop scrolling second page
     */
    virtual void stop_show_more_images() = 0;

signals:    
    /**
     * @brief emit when go_to_second_page done
     */
    void go_to_gallery_page_done();
    /**
     * @brief emit when go_to_first_page done
     */
    void go_to_search_page_done();

    /**
     * @brief emit when load url done
     */
    void load_url_done();
    /**
     * @brief emit when reload done
     */
    void reload_url_done();
    /**
     * @brief emit when show_more_images done
     */
    void show_more_images_done();

    void search_error(image_search_error::error error);

    /**
     * @brief emit whenever second page scrolled
     */
    void second_page_scrolled();

protected:
    QWebEnginePage& get_web_page();

private:
    void clipboard_data_changed();
    virtual void load_web_page_finished(bool ok) = 0;

    std::function<void(QImage const&)> get_web_view_img_callback_;
    QWebEnginePage &web_page_;
};

#endif // IMAGE_SEARCH_HPP
