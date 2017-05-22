#ifndef WEB_VIEW_HPP
#define WEB_VIEW_HPP

#include <QWebEngineView>

class web_view : public QWebEngineView
{    
public:
    explicit web_view(QWidget *parent = nullptr);
};

#endif // WEB_VIEW_HPP
