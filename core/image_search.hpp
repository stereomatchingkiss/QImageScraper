#ifndef IMAGE_SEARCH_HPP
#define IMAGE_SEARCH_HPP

#include <QObject>

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
     * @brief go to the first page of the search engine. Will
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
     * @brief get page link of second page, this page
     * contain the image link
     * @return page link
     */
    virtual QStringList get_page_link() = 0;

    /**
     * @brief parse image link in page link,this function will
     * try to obtain the page link before parsing image link.
     * Will emit signal "found_image_link" after each page is parsed.
     */
    virtual void parse_imgs_link() = 0;

    /**
     * @brief scroll second page of the search engine. This function
     * will emit signal "scroll_second_page_done" after scroll action done
     * @param max_search_size size of the images want to scrape
     */
    virtual void scroll_second_page(size_t max_search_size) = 0;

signals:
    void error_msg(QString const &msg);
    void found_page_link(QString const &link);
    void found_image_link(QString const &big_img_link, QString const &small_img_link);
    void go_to_first_page_done();
    void go_to_second_page_done();
    void parse_all_image_link();
    void scroll_second_page_done();

protected:
    QWebEnginePage& get_web_page();

private:
    virtual void load_web_page_finished(bool ok) = 0;    

    QWebEnginePage &web_page_;
};

#endif // IMAGE_SEARCH_HPP
