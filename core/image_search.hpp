#ifndef IMAGE_SEARCH_HPP
#define IMAGE_SEARCH_HPP

#include <QObject>

class QColor;
class QWebEnginePage;

/**
 * @brief Encapsulate process of scraping image link of
 * search engine. This class allow user to setup filter,
 * since every search engine offer different filters, not
 * all of the options can apply on every search engines.
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
     * @brief find_image_links
     * @param target
     * @param num
     */
    virtual void find_image_links(QString const &target, size_t max_search_size = 0) = 0;    

signals:
    void found_page_link(QString const &link);
    void found_image_link(QString const &big_img_link, QString const &small_img_link);
    void parse_all_image_link();

protected:
    QWebEnginePage& get_web_page();

private:
    virtual void load_web_page_finished(bool ok) = 0;    

    QWebEnginePage &web_page_;
};

#endif // IMAGE_SEARCH_HPP
