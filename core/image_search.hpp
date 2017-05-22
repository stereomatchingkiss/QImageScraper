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
    enum class color_option
    {
        all,
        color_only,
        black_and_white,
    };

    enum class image_type
    {
        all,
        photograph,
        clipart,
        line_drawing,
        animated_gif,
        transparent
    };

    enum class layout
    {
        all,
        square,
        wide,
        tall
    };

    enum class people
    {
        all,
        just_faces,
        head_and_shoulder
    };

    enum class safe_search
    {
        moderate,
        strict,
        off
    };

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

    virtual void set_color_filter(color_option option) noexcept;
    virtual void set_color_filter(QColor const &option) noexcept;
    virtual void set_image_type_filter(image_type option) noexcept;
    virtual void set_layout_filter(layout option) noexcept;
    virtual void set_maximum_size_filter(QSize const &size) noexcept;
    virtual void set_minimum_size_filter(QSize const &size) noexcept;
    virtual void set_people_filter(people option) noexcept;
    virtual void set_safe_search(safe_search option) noexcept;
    virtual void set_suffix_filter(QStringList const &type);

signals:
    void found_image_link(QString const &big_img_link, QString const &small_img_link);
    void parse_all_image_link();

protected:
    QWebEnginePage& get_web_page();

private:
    virtual void load_web_page_finished(bool ok) = 0;    

    QWebEnginePage &web_page_;
};

#endif // IMAGE_SEARCH_HPP
