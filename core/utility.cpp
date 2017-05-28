#include "utility.hpp"

#include "global_constant.hpp"

#include <QFileInfo>
#include <QNetworkInterface>
#include <QNetworkRequest>

QString convert_url_to_filename(QString const &url, QString const &save_at)
{
    QFileInfo file_info(url);
    QString file_name = file_info.fileName();
    if(QFile::exists(save_at + "/" + file_name)){
        QString const base_name = file_info.baseName();
        QString complete_suffix = file_info.completeSuffix();
        if(complete_suffix.isEmpty()){
            complete_suffix = "txt";
        }
        QString new_file_name = base_name + "(0)." + complete_suffix;
        for(size_t i = 1; QFile::exists(save_at + "/" + new_file_name); ++i){
            new_file_name = base_name + "(" + QString::number(i) + ")." + complete_suffix;
        }

        return new_file_name;
    }

    return file_name;
}

QNetworkRequest create_img_download_request(const QString &url, const QString &)
{    
    QNetworkRequest request(url);
    QStringList const agents{"msnbot-media/1.1 (+http://search.msn.com/msnbot.htm)",
                             "Googlebot-Image/1.0",
                            "Mozilla/5.0 (compatible; Bingbot/2.0; +http://www.bing.com/bingbot.htm)",
                            "Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)"};
    request.setHeader(QNetworkRequest::UserAgentHeader, agents[qrand()%agents.size()]);

    return request;
}

bool is_connected_to_network()
{
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;

    for (int i = 0; i < ifaces.count(); ++i){
        if(result){
            break;
        }
        QNetworkInterface iface = ifaces.at(i);
        if(iface.flags().testFlag(QNetworkInterface::IsUp)
             && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)){

            // we have an interface that is up, and has an ip address
            // therefore the link is present
            // we will only enable this check on first positive,
            // all later results are incorrect
            if(!iface.addressEntries().empty()){
                if(!result){
                    result = true;
                }
            }
        }
    }

    return result;
}
