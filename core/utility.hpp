#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <QString>

class QNetworkRequest;

QString convert_url_to_filename(QString const &url, QString const &save_at);

QNetworkRequest create_img_download_request(QString const &url, QString const &engine);

/**
 * @brief check the os connect to network or not
 * @return true, may connect to network;false, do not connect to network
 */
bool is_connected_to_network();

#endif // UTILITY_HPP
