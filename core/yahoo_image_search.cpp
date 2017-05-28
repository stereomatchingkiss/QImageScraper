#include "yahoo_image_search.hpp"

#include <QDebug>
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
    max_search_size_{0},
    scroll_count_{0},
    scroll_limit_{2},
    state_{state::to_search_page},
    stop_scroll_page_{false}
{

}

void yahoo_image_search::get_page_link(std::function<void (const QStringList&)> callback)
{
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        parse_page_link(contents);
        callback(img_page_links_);
    });
}

void yahoo_image_search::go_to_search_page()
{
    state_ = state::to_search_page;
    img_page_links_.clear();
    get_web_page().load(QUrl("https://images.search.yahoo.com/"));
}

void yahoo_image_search::show_more_images(size_t max_search_size)
{
    max_search_size_ = max_search_size;
    if((int)max_search_size_ > img_page_links_.size()){
        scroll_limit_ = (max_search_size_ - img_page_links_.size()) / 48 + 1;
    }else{
        scroll_limit_ = 4;
    }
    state_ = state::show_more_images;
    scroll_web_page();
}

void yahoo_image_search::stop_show_more_images()
{
    stop_scroll_page_ = true;
}

void yahoo_image_search::load_web_page_finished(bool ok)
{
    qDebug()<<"load web page finished:"<<ok<<", url:"<<get_web_page().url().toString();
    if(ok){
        if(get_web_page().url().toString().contains("https://images.search.yahoo.com/search/images;")){
            state_ = state::to_gallery_page;
            emit go_to_gallery_page_done();
            return;
        }

        switch(state_){
        case state::to_search_page:{
            qDebug()<<"state to search page";
            emit go_to_search_page_done();
            break;
        }
        case state::to_gallery_page:{
            qDebug()<<"state to gallery page";
            emit go_to_gallery_page_done();
            break;
        }
        case state::show_more_images:{
            qDebug()<<"state more images";
            break;
        }
        case state::get_img_link_from_gallery_page:{
            qDebug()<<"state get_img_link_from_gallery_page";
            return;
        }
        default:
            qDebug()<<"default state";
            break;
        }
    }else{
        emit search_error(image_search_error::error::load_page_error);
    }
}

void yahoo_image_search::parse_page_link(QString const &contents)
{
    QRegularExpression const reg("/images/view;[^\"]*");
    auto iter = reg.globalMatch(contents);
    QStringList links;
    while(iter.hasNext()){
        auto const match = iter.next();
        links.push_back("https://images.search.yahoo.com" + match.captured(0));
    }
    links.removeDuplicates();
    qDebug()<<"total match link:"<<links.size();
    if(links.size() > img_page_links_.size()){
        links.swap(img_page_links_);
    }
}

void yahoo_image_search::scroll_web_page()
{
    //we need to setup timer because web view may not able to update in time,
    //this may cause the page stop scrolling too early
    if(state_ == state::show_more_images){
        scroll_count_ = 0;
        stop_scroll_page_ = false;
        QTimer::singleShot(scroll_page_duration, [this](){scroll_web_page_impl();});
    }
}

void yahoo_image_search::scroll_web_page_impl()
{
    qDebug()<<__func__<<":scroll_count:"<<scroll_count_;
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
            qDebug()<<"found See more images";
            get_web_page().runJavaScript("document.getElementsByClassName(\"ygbt more-res\")[0].click();"
                                         "window.scrollTo(0, document.body.scrollHeight);");
        }else{
            qDebug()<<"cannot found See more images";
            get_web_page().runJavaScript("window.scrollTo(0, document.body.scrollHeight)");
        }
        ++scroll_count_;
        emit second_page_scrolled();
        QTimer::singleShot(scroll_page_duration, [this](){scroll_web_page_impl();});
    });
}

void yahoo_image_search::get_imgs_link(const QString&,
                                       std::function<void (const QString&, const QString&)>)
{
    throw std::runtime_error("void yahoo_image_search::get_imgs_link haven't implemented yet");
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
                qDebug()<<"get_imgs_link_from_gallery_page:"<<link;
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
