#ifndef BING_IMAGE_SEARCH_HPP
#define BING_IMAGE_SEARCH_HPP

#include "image_search.hpp"

#include <QColor>
#include <QSize>

class bing_image_search : public image_search
{
    Q_OBJECT    
public:
    explicit bing_image_search(QWebEnginePage &page, QObject *parent = nullptr);

    void find_image_links(QString const &target, size_t max_search_size = 0) override;

    void set_color_filter(color_option option) noexcept override;
    void set_color_filter(QColor const &option) noexcept override;
    void set_image_type_filter(image_type option) noexcept override;
    void set_layout_filter(layout option) noexcept override;
    void set_maximum_size_filter(QSize const &size) noexcept override;
    void set_minimum_size_filter(QSize const &size) noexcept override;
    void set_people_filter(people option) noexcept override;
    void set_safe_search(safe_search option) noexcept override;
    void set_suffix_filter(QStringList const &type) override;

private:
    enum class state{
       load_first_page,
       scroll_page,
       parse_page_link,
       parse_image_link
    };

    void load_web_page_finished(bool ok) override;
    void parse_page_link(QPointF const &point);
    void scroll_web_page(QPointF const &point);
    void web_page_scroll_position_changed(QPointF const &point);

    QColor color_;
    color_option color_option_;
    QStringList img_page_links_;
    image_type image_type_;
    layout layout_;
    size_t max_search_size_;
    QSize max_size_;
    QSize min_size_;
    people people_;
    safe_search safe_search_;
    bool scroll_pos_changed_;
    state state_;
    QStringList suffix_;
    qreal ypos_;
};

#endif // BING_IMAGE_SEARCH_HPP
