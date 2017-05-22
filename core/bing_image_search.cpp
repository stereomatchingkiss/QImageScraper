#include "bing_image_search.hpp"
#include "js_function.hpp"

#include <QDebug>
#include <QSize>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QWebEnginePage>

bing_image_search::bing_image_search(QWebEnginePage &page, QObject *parent) :
    image_search(page, parent),
    color_option_(color_option::all),
    image_type_(image_type::all),
    layout_(layout::all),
    max_search_size_(0),
    people_(people::all),
    safe_search_(safe_search::moderate),
    scroll_pos_changed_(true),
    state_(state::load_first_page),
    suffix_({"jpg", "jpeg", "png"}),
    ypos_(0)
{
    auto *web_page = &get_web_page();
    connect(web_page, &QWebEnginePage::scrollPositionChanged,
            this, &bing_image_search::web_page_scroll_position_changed);
    connect(web_page, &QWebEnginePage::loadProgress, [](int progress){ qDebug()<<"load progress:"<<progress;});
    connect(web_page, &QWebEnginePage::loadStarted, [](){ qDebug()<<"load started";});
}

void bing_image_search::find_image_links(const QString &target, size_t max_search_size)
{    
    max_search_size_ = max_search_size;
    state_ = state::load_first_page;
    get_web_page().load("https://www.bing.com/images/search?q=" + target);
}

void bing_image_search::set_color_filter(image_search::color_option option) noexcept
{
    color_option_ = option;
}

void bing_image_search::set_color_filter(const QColor &option) noexcept
{
    color_ = option;
}

void bing_image_search::set_layout_filter(image_search::layout option) noexcept
{
    layout_ = option;
}

void bing_image_search::set_maximum_size_filter(const QSize &size) noexcept
{
    max_size_ = size;
}

void bing_image_search::set_minimum_size_filter(const QSize &size) noexcept
{
    min_size_ = size;
}

void bing_image_search::set_people_filter(image_search::people option) noexcept
{
    people_ = option;
}

void bing_image_search::set_safe_search(image_search::safe_search option) noexcept
{
    safe_search_ = option;
}

void bing_image_search::set_image_type_filter(image_search::image_type option) noexcept
{
    image_type_ = option;
}

void bing_image_search::set_suffix_filter(const QStringList &type)
{
    suffix_ = type;
}

void bing_image_search::load_web_page_finished(bool ok)
{
    qDebug()<<"load web page finished:"<<ok;
    if(ok){
        switch(state_){
        case state::load_first_page:{
            qDebug()<<"state load first page";
            state_ = state::parse_page_link;
            scroll_web_page({});
            break;
        }
        case state::parse_page_link:{
            qDebug()<<"parse page link";
            break;
        }
        case state::parse_img_link:{
            qDebug()<<"parse image link";
            parse_imgs_link_content();
            break;
        }
        case state::scroll_page:{
            qDebug()<<"state scroll page";
            scroll_web_page({});
            break;
        }
        default:
            qDebug()<<"default state";
            break;
        }
    }
}

void bing_image_search::parse_imgs_link()
{
    if(!img_page_links_.empty()){
        get_web_page().load(img_page_links_[0]);
    }
}

void bing_image_search::parse_imgs_link_content()
{
    get_web_page().toHtml([this](QString const &contents)
    {
        QRegularExpression reg("<img class=\"mainImage\" src=\"([^\"]*)\" src2=\"([^\"]*)");
        auto match = reg.match(contents);
        if(match.hasMatch()){
            qDebug()<<"img link:"<<match.captured(1)<<"\n"<<match.captured(2);

        }else{
            qDebug()<<"cannot capture img link";
        }
        if(!img_page_links_.isEmpty()){
            img_page_links_.pop_front();
            emit found_image_link(match.captured(2), match.captured(1));
            parse_imgs_link();
        }else{
            qDebug()<<"bing image search parse all imgs link";
            emit parse_all_image_link();
        }
    });
}

void bing_image_search::parse_page_link(QPointF const &point)
{
    if(state_ != state::parse_page_link){
        return;
    }

    get_web_page().toHtml([this, point](QString const &contents)
    {
        qDebug()<<"get image link contents";
        QRegularExpression reg("(search\\?view=detailV2[^\"]*)");
        auto iter = reg.globalMatch(contents);
        QStringList links;
        while(iter.hasNext()){
            QRegularExpressionMatch match = iter.next();
            if(match.captured(1).right(20) != "ipm=vs#enterinsights"){
                QString url = QUrl("https://www.bing.com/images/" + match.captured(1)).toString();
                url.replace("&amp;", "&");
                links.push_back(url);
            }
        }
        links.removeDuplicates();
        qDebug()<<"total match link:"<<links.size();
        if(links.size() > img_page_links_.size()){
            links.swap(img_page_links_);
        }
        if((size_t)img_page_links_.size() >= max_search_size_){
            state_ = state::parse_img_link;
            parse_imgs_link();
        }else{
            get_web_page().findText("See more images", QWebEnginePage::FindFlag(), [this](bool found)
            {
                if(found){
                    qDebug()<<"found See more images";
                    get_web_page().runJavaScript("document.getElementsByClassName(\"btn_seemore\")[0].click();"
                                                 "window.scrollTo(0, document.body.scrollHeight);");
                }else{
                    qDebug()<<"cannot found See more images";
                    get_web_page().runJavaScript(js_scroll_to_window_height_2(100), [this](QVariant const &result)
                    {
                        qDebug()<<"scroll page result:"<<result;
                        if(!result.toList()[0].toBool()){
                            state_ = state::parse_img_link;
                            qDebug()<<"parse imgs link start";
                            parse_imgs_link();
                        }
                    });
                }
            });
        }
    });
}

void bing_image_search::scroll_web_page(QPointF const &point)
{
    //we need to setup timer if the web view are shown on the screen.
    //Because web view may not able to update in time, this may cause the signal scrollPositionChanged
    //never emit, because the web page do not have enough of space to scroll down
    QTimer::singleShot(2000, [=]()
    {
        if(state_ == state::parse_page_link){
            parse_page_link(point);
        }
    });
}

void bing_image_search::web_page_scroll_position_changed(const QPointF &point)
{
    static size_t index = 0;
    qDebug()<<index++<<":"<<point.y();
    scroll_web_page(point);
}
