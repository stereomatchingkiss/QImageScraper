#include "yahoo_image_search.hpp"

#include <QsLog.h>

#include <QSize>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QWebEnginePage>

namespace{

constexpr int scroll_page_duration = 2500;

}

yahoo_image_search::yahoo_image_search(QWebEnginePage &page, QObject *parent) :
    image_search(page, parent),
    scroll_count_{0},
    scroll_limit_{2},
    state_{state::to_search_page},
    stop_scroll_page_{false}
{

}

void yahoo_image_search::get_search_image_size(std::function<void (size_t)> callback)
{
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        callback(parse_page_link(contents).size());
    });
}

void yahoo_image_search::get_search_target(std::function<void (const QString &)> callback)
{
    get_web_page().runJavaScript("function jscmd(){return document.getElementById(\"yschsp\").value} jscmd()",
                                 [this, callback](QVariant const &contents)
    {
        QLOG_INFO()<<"gallery page target:"<<contents.toString();
        callback(contents.toString());
    });
}

void yahoo_image_search::go_to_gallery_page(QString const &target)
{
    state_ = state::to_gallery_page;
    get_web_page().load(QString("https://images.search.yahoo.com/search/images;?&p=%1&ei=UTF-8&iscqry=&fr=sfp").
                        arg(target));
}

void yahoo_image_search::go_to_search_page()
{
    state_ = state::to_search_page;
    get_web_page().load(QUrl("https://images.search.yahoo.com/"));
}

void yahoo_image_search::load(const QUrl &url)
{
    state_ = state::load_url;
    image_search::load(url);
}

void yahoo_image_search::reload()
{
    state_ = state::reload_url;
    get_web_page().load(get_web_page().url());
}

void yahoo_image_search::show_more_images(size_t max_search_size)
{            
    scroll_limit_ = (max_search_size) / 48 + 1;
    state_ = state::show_more_images;
    //we need to setup timer because web view may not able to update in time,
    //this may cause the page stop scrolling too early
    scroll_count_ = 0;
    stop_scroll_page_ = false;
    QTimer::singleShot(scroll_page_duration, [this](){scroll_web_page();});
}

void yahoo_image_search::stop_show_more_images()
{
    stop_scroll_page_ = true;
}

void yahoo_image_search::load_web_page_finished(bool ok)
{
    QLOG_INFO()<<"load web page finished:"<<ok<<", url:"<<get_web_page().url().toString();
    if(ok){
        if(get_web_page().url().toString().contains("https://images.search.yahoo.com/search/images;")){
            state_ = state::to_gallery_page;
            emit go_to_gallery_page_done();
            return;
        }

        switch(state_){
        case state::load_url:{
            QLOG_INFO()<<"state load_url";
            emit load_url_done();
            break;
        }
        case state::reload_url:{
            QLOG_INFO()<<"state reload_url";
            emit reload_url_done();
            break;
        }
        case state::to_search_page:{
            QLOG_INFO()<<"state to search page";
            emit go_to_search_page_done();
            break;
        }
        case state::to_gallery_page:{
            QLOG_INFO()<<"state to gallery page";
            emit go_to_gallery_page_done();
            break;
        }
        case state::show_more_images:{
            QLOG_INFO()<<"state more images";
            break;
        }
        case state::get_img_link_from_gallery_page:{
            QLOG_INFO()<<"state get_img_link_from_gallery_page";
            return;
        }
        default:
            QLOG_INFO()<<"default state";
            break;
        }
    }else{
        emit search_error(image_search_error::error::load_page_error);
    }
}

QStringList yahoo_image_search::parse_page_link(QString const &contents)
{
    QRegularExpression const reg("/images/view;[^\"]*");
    auto iter = reg.globalMatch(contents);
    QStringList links;
    while(iter.hasNext()){
        auto const match = iter.next();
        links.push_back("https://images.search.yahoo.com" + match.captured(0));
    }
    links.removeDuplicates();
    QLOG_INFO()<<"total match link:"<<links.size();

    return links;
}

void yahoo_image_search::scroll_web_page()
{
    QLOG_INFO()<<__func__<<":scroll_count:"<<scroll_count_;
    if(state_ != state::show_more_images){
        return;
    }

    if(stop_scroll_page_){
        stop_scroll_page_ = false;
        emit show_more_images_done();
        return;
    }

    if(scroll_count_ == scroll_limit_){
        emit show_more_images_done();
        return;
    }

    get_web_page().toPlainText([this](QString const &contents)
    {
        if(contents.contains("Show More Images")){
            QLOG_INFO()<<"found See more images";
            get_web_page().runJavaScript("document.getElementsByClassName(\"ygbt more-res\")[0].click();"
                                         "window.scrollTo(0, document.body.scrollHeight);");
        }else{
            QLOG_INFO()<<"cannot found See more images";
            get_web_page().runJavaScript("window.scrollTo(0, document.body.scrollHeight)");
        }
        ++scroll_count_;
        emit second_page_scrolled();
        QTimer::singleShot(scroll_page_duration, [this](){scroll_web_page();});
    });
}

void yahoo_image_search::get_imgs_link_from_gallery_page(std::function<void(const QStringList&, const QStringList&)> callback)
{
    state_ = state::get_img_link_from_gallery_page;
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        auto global_parser = [&contents](QRegularExpression const &re)
        {
            auto iter = re.globalMatch(contents);
            QStringList result;
            while(iter.hasNext()){
                QRegularExpressionMatch const match = iter.next();
                QString link = match.captured(1).replace("%2F", "//").replace("%3F", "?");
                link.replace("%3D", "=").replace("&amp;", "&");
                QLOG_INFO()<<"get_imgs_link_from_gallery_page:"<<link;
                result.push_back(link);
            }

            return result;
        };
        QStringList big_imgs = global_parser(QRegularExpression("imgurl=([^&]*)"));
        QStringList small_imgs = global_parser(QRegularExpression("img src='([^']*)"));
        while(small_imgs.size() < big_imgs.size()){
            small_imgs.push_back("");
        }
        while(big_imgs.size() < small_imgs.size()){
            big_imgs.push_back("");
        }
        for(auto &link : big_imgs){
            link.prepend("http://");
        }
        callback(big_imgs, small_imgs);
    });
}
