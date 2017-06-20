#ifndef TOR_CONTROLLER_HPP
#define TOR_CONTROLLER_HPP

#include <QObject>
#include <QAbstractSocket>

class QTcpSocket;
class QTimer;

class tor_controller : public QObject
{
    Q_OBJECT
public:
    explicit tor_controller(QObject *parent = nullptr);

    void renew_ip(QString const &host, quint16 port, QString const &password);

signals:
    void authenticate_success();
    void error_happen(QString const &msg);
    void renew_ip_success();

private:
    enum class tor_state{
        authenticate,
        renew_ip
    };

    void handle_error(QAbstractSocket::SocketError error);
    void handle_ready_read();

    QString host_;
    quint16 port_;
    QString password_;
    QTcpSocket *socket_;
    tor_state state_;
    QTimer *timer_;
};

#endif // TOR_CONTROLLER_HPP
