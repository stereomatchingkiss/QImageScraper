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
    max_search_size_(0),        
    scroll_count_(0),
    scroll_limit_(2),
    state_(state::load_first_page),    
    timer_(new QTimer(this))
{
    auto *web_page = &get_web_page();
    connect(web_page, &QWebEnginePage::loadProgress, [](int progress){ qDebug()<<"load progress:"<<progress;});
    connect(web_page, &QWebEnginePage::loadStarted, [](){ qDebug()<<"load started";});
}

void bing_image_search::find_image_links(const QString &target, size_t max_search_size)
{    
    max_search_size_ = max_search_size;
    scroll_limit_ = max_search_size_ * 2 / 35 + 1;
    state_ = state::load_first_page;
    get_web_page().load("https://www.bing.com/images/search?q=" + target);
}

void bing_image_search::load_web_page_finished(bool ok)
{
    qDebug()<<"load web page finished:"<<ok;
    if(ok){
        switch(state_){
        case state::load_first_page:{
            qDebug()<<"state load first page";
            state_ = state::parse_page_link;
            scroll_web_page();
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
            scroll_web_page();
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
            emit found_image_link(match.captured(2).replace("&amp;", "&"),
                                  match.captured(1).replace("&amp;", "&"));
            parse_imgs_link();
        }else{
            qDebug()<<"bing image search parse all imgs link";
            emit parse_all_image_link();
        }
    });
}

void bing_image_search::parse_page_link()
{
    ++scroll_count_;
    qDebug()<<__func__<<":"<<scroll_count_;
    if(state_ != state::parse_page_link){
        return;
    }

    if(scroll_count_ == scroll_limit_){
        timer_->stop();
        state_ = state::parse_img_link;
        parse_imgs_link();
    }

    get_web_page().toHtml([this](QString const &contents)
    {
        qDebug()<<"get image link contents";
        parse_page_link_by_regex(contents);
        if((size_t)img_page_links_.size() >= max_search_size_){
            timer_->stop();
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
                    get_web_page().runJavaScript("window.scrollTo(0, document.body.scrollHeight)");
                }
            });
        }
    });
}

void bing_image_search::parse_page_link_by_regex(const QString &contents)
{
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
}

void bing_image_search::scroll_web_page()
{
    //we need to setup timer if the web view are shown on the screen.
    //Because web view may not able to update in time, this may cause the signal scrollPositionChanged
    //never emit, because the web page do not have enough of space to scroll down
    if(state_ == state::parse_page_link){
        scroll_count_ = 0;
        connect(timer_, &QTimer::timeout, this, &bing_image_search::parse_page_link);
        timer_->start(2000);
    }
}
