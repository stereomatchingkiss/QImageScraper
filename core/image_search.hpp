#ifndef IMAGE_SEARCH_HPP
#define IMAGE_SEARCH_HPP

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
     * @brief Asynchronous method, go to the first page of the search engine. Will
     * emit signal "go_to_first_page_done" after fist page is loaded.
     * Bing : https://www.bing.com/?scope=images&nr=1&FORM=NOFORM
     * Google : https://images.google.com/
     */
    virtual void go_to_first_page() = 0;
    /**
     * @brief go to the second page of the search engine. Will
     * emit signal "go_to_second_page_done" after second page is loaded.
     * Bing : "https://www.bing.com/images/search?q=" + target
     * Google : "https://www.google.co.in/search?q=" + searchtext + "&source=lnms&tbm=isch"
     */
    virtual void go_to_second_page() = 0;

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
                                 std::function<void(QString const&, QString const&)> callback) = 0;

    /**
     * @brief Asynchronous method to parse all of the image links from second page. Upon
     * completion, result callback is called with the image links
     * @param callback callback Upon completion, result callback is called with the
     * image links(big image links, small image links).
     */
    virtual void get_imgs_link_from_second_page(std::function<void(QStringList const&, QStringList const&)> callback) = 0;

    /**
     * @brief Asynchronous method, scroll second page of the search engine. This function
     * will emit signal "scroll_second_page_done" after scroll action
     * are all done; Will emit signal "second_page_scrolled" everytime
     * the page scroll(if oage scrolling action is trigger by scroll_second_page)
     * @param max_search_size size of the images want to scrape
     */
    virtual void scroll_second_page(size_t max_search_size) = 0;

    /**
     * @brief stop scrolling second page
     */
    virtual void stop_scroll_second_page() = 0;

signals:
    void error_msg(QString const &msg);
    /**
     * @brief emit when go_to_first_page done
     */
    void go_to_first_page_done();
    /**
     * @brief emit when go_to_second_page done
     */
    void go_to_second_page_done();
    /**
     * @brief emit when scroll_second_page done
     */
    void scroll_second_page_done();
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
