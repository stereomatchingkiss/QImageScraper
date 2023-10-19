#include "utility.hpp"

#include <QFileInfo>
#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QRandomGenerator>

QNetworkRequest create_img_download_request(const QString &url)
{    
    QNetworkRequest request(url);
    QStringList const agents{"msnbot-media/1.1 (+http://search.msn.com/msnbot.htm)",
                             "Googlebot-Image/1.0",
                            "Mozilla/5.0 (compatible; Bingbot/2.0; +http://www.bing.com/bingbot.htm)",
                            "Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)"};    
    request.setHeader(QNetworkRequest::UserAgentHeader, agents[QRandomGenerator::global()->bounded(agents.size())]);

    return request;
}
