#ifndef TOR_CONTROLLER_HPP
#define TOR_CONTROLLER_HPP

#include <QObject>
#include <QAbstractSocket>

class QNetworkAccessManager;
class QNetworkReply;
class QTcpSocket;
class QTimer;

class tor_controller : public QObject
{
    Q_OBJECT
public:
    explicit tor_controller(QObject *parent = nullptr);

    void check_tor_validation(QString const &host, quint16 port, quint16 control_port, QString const &password);
    void renew_ip(QString const &host, quint16 control_port, QString const &password);

signals:    
    void error_happen(QString const &msg);
    void renew_ip_success();
    void validate_tor_fail(QString const &msg);
    void validate_tor_success();

private:
    enum class tor_state{
        check_tor_validation,
        renew_ip
    };

    void access_network_by_tor(QString const &link);
    void handle_error(QAbstractSocket::SocketError error);
    void handle_ready_read();
    void handle_network_finished();
    void renew_ip_impl(QString const &host, quint16 control_port, QString const &password);

    quint16 control_port_;
    QString host_;
    QNetworkAccessManager *network_manager_;
    QString password_;
    quint16 port_;
    QNetworkReply *reply_;
    QTcpSocket *socket_;
    tor_state state_;
    QTimer *timer_;
};

#endif // TOR_CONTROLLER_HPP
