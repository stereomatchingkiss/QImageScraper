#ifndef IMAGE_SEARCH_HPP
#define IMAGE_SEARCH_HPP

#include "image_search_error.hpp"

#include <QObject>

#include <functional>

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
     * @brief Asynchronous method to parse all of the image links from gallery page. Upon
     * completion, result callback is called with the image links
     * @param callback callback Upon completion, result callback is called with the
     * image links(big image links, small image links).
     */
    virtual void get_imgs_link_from_gallery_page(std::function<void(QStringList const&, QStringList const&)> callback) = 0;

    /**
     * @brief Asynchronous method to searched image size of gallery page(s).
     * Upon completion, result callback is called with the page links.
     * @param callback Upon completion, result callback is called with the
     * page links.
     */
    virtual void get_search_image_size(std::function<void(size_t)> callback) = 0;

    /**
     * @brief Asynchronous method to get the search target. Upon
     * completion, result callback is called with the search target
     * @param callback self explained
     */
    virtual void get_search_target(std::function<void(QString const &)> callback) = 0;

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
     * This function will emit signal "show_more_images_done" after action
     * are all done
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
    virtual void load_web_page_finished(bool ok) = 0;

    QWebEnginePage &web_page_;
};

#endif // IMAGE_SEARCH_HPP
