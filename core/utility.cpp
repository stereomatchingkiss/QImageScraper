#include "utility.hpp"

#include <QFileInfo>
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

QNetworkRequest create_img_download_request(const QString &url, const QString &engine)
{
    if(engine == "Bing"){
        QNetworkRequest request(url);
        QString const header = "msnbot/2.0b (+http://search.msn.com/msnbot.htm)";
        //QString const header = "msnbot-media/1.1 (+http://search.msn.com/msnbot.htm)";
        //QString const header = "Mozilla/5.0 (compatible; bingbot/2.0; +http://www.bing.com/bingbot.htm)";
        request.setHeader(QNetworkRequest::UserAgentHeader, header);

        return request;
    }

    return QNetworkRequest(url);
}
