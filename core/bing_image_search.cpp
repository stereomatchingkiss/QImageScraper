#include "bing_image_search.hpp"
#include "js_function.hpp"

#include <QDebug>
#include <QSize>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QWebEnginePage>

namespace{

constexpr int scroll_page_duration = 1500;

}

bing_image_search::bing_image_search(QWebEnginePage &page, QObject *parent) :
    image_search{page, parent},
    max_search_size_{0},
    scroll_count_{0},
    scroll_limit_{2},
    state_{state::to_first_page},
    stop_scroll_page_{false}
{
    auto *web_page = &get_web_page();
    connect(web_page, &QWebEnginePage::loadProgress, [](int progress){ qDebug()<<"load progress:"<<progress;});
    connect(web_page, &QWebEnginePage::loadStarted, [](){ qDebug()<<"load started";});
}

void bing_image_search::get_page_link(std::function<void (const QStringList &)> callback)
{
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        parse_page_link(contents);
        callback(img_page_links_);
    });
}

void bing_image_search::go_to_first_page()
{
    state_ = state::to_first_page;
    img_page_links_.clear();
    get_web_page().load(QUrl("https://www.bing.com/?scope=images&nr=1&FORM=NOFORM"));
}

void bing_image_search::go_to_second_page()
{
    state_ = state::to_second_page;
    get_web_page().runJavaScript("function jscmd(){return document.getElementById(\"sb_form_q\").value} jscmd()",
                                 [this](QVariant const &contents)
    {
        if(contents.isValid()){
            get_web_page().load("https://www.bing.com/images/search?q=" + contents.toString());
        }else{
            emit error_msg(tr("Please enter the target you want to search"));
        }
    });
}

void bing_image_search::scroll_second_page(size_t max_search_size)
{
    max_search_size_ = max_search_size;
    if((int)max_search_size_ > img_page_links_.size()){
        scroll_limit_ = (max_search_size_ - img_page_links_.size()) / 35 + 1;
    }else{
        scroll_limit_ = 4;
    }
    state_ = state::scroll_page;
    scroll_web_page();
}

void bing_image_search::stop_scroll_second_page()
{
    stop_scroll_page_ = true;
}

void bing_image_search::load_web_page_finished(bool ok)
{
    qDebug()<<"load web page finished:"<<ok<<", url:"<<get_web_page().url().toString();
    if(ok){
        if(get_web_page().url().toString().contains("https://www.bing.com/images/search?q=")){
            state_ = state::to_second_page;
            emit go_to_second_page_done();
            return;
        }

        switch(state_){
        case state::to_first_page:{
            qDebug()<<"state to first page";
            emit go_to_first_page_done();
            break;
        }
        case state::to_second_page:{
            qDebug()<<"state to second page";
            emit go_to_second_page_done();
            break;
        }
        case state::scroll_page:{
            qDebug()<<"state scroll page";
            break;
        }
        case state::parse_img_link:{
            qDebug()<<"state parse img link";
            parse_imgs_link_content();
            break;
        }
        case state::get_img_link_from_sec_page:{
            qDebug()<<"state get_img_link_from_sec_page";
            return;
        }
        default:
            qDebug()<<"default state";
            break;
        }
    }else{
        emit error_msg(tr("Failed to load the web page"));
    }
}

void bing_image_search::get_imgs_link(const QString &page_link,
                                        std::function<void (const QString &, const QString &)> callback)
{
    parse_img_link_callback_ = callback;
    state_ = state::parse_img_link;
    get_web_page().load(page_link);
}

void bing_image_search::get_imgs_link_from_second_page(std::function<void(const QStringList &, const QStringList &)> callback)
{
    state_ = state::get_img_link_from_sec_page;
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        QRegularExpression const reg("m=\"{[^,]*([^;]*;){7}([^&]*)[^\"]*\" "
                                     "mad=\"{([^;]*;){3}([^;]*;)");
        auto iter = reg.globalMatch(contents);
        QStringList big_img, small_img;
        while(iter.hasNext()){
            QRegularExpressionMatch const match = iter.next();
            qDebug()<<"parse_imgs_link_from_second_page:"<<match.captured(2)<<","<<match.captured(4);
            big_img.push_back(match.captured(2).replace("&amp;", "&"));
            small_img.push_back(match.captured(4).replace("&amp;", "&"));
        }
        callback(big_img, small_img);
    });
}

void bing_image_search::parse_imgs_link_content()
{
    get_web_page().toHtml([this](QString const &contents)
    {
        QRegularExpression const reg("<img class=\"mainImage\" src=\"([^\"]*)\" src2=\"([^\"]*)");
        auto const match = reg.match(contents);
        if(match.hasMatch()){
            qDebug()<<"img link:"<<match.captured(1)<<"\n"<<match.captured(2);
        }else{
            qDebug()<<"cannot capture img link";
        }
        parse_img_link_callback_(match.captured(2).replace("&amp;", "&"),
                                 match.captured(1).replace("&amp;", "&"));
    });
}

void bing_image_search::scroll_web_page_impl()
{    
    qDebug()<<__func__<<":scroll_count:"<<scroll_count_;
    if(state_ != state::scroll_page){
        return;
    }

    if(stop_scroll_page_){
        stop_scroll_page_ = false;
        emit scroll_second_page_done();
        return;
    }

    if(scroll_count_ == scroll_limit_){
        emit scroll_second_page_done();
        return;
    }

    get_web_page().toPlainText([this](QString const &contents)
    {
        if(contents.contains("See more images")){
            qDebug()<<"found See more images";
            get_web_page().runJavaScript("document.getElementsByClassName(\"btn_seemore\")[0].click();"
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

void bing_image_search::parse_page_link(const QString &contents)
{
    QRegularExpression const reg("(search\\?view=detailV2[^\"]*)");
    auto iter = reg.globalMatch(contents);
    QStringList links;
    while(iter.hasNext()){
        QRegularExpressionMatch const match = iter.next();
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
}

void bing_image_search::scroll_web_page()
{
    //we need to setup timer because web view may not able to update in time,
    //this may cause the page stop scrolling too early
    if(state_ == state::scroll_page){
        scroll_count_ = 0;
        stop_scroll_page_ = false;
        QTimer::singleShot(scroll_page_duration, [this](){scroll_web_page_impl();});
    }
}
