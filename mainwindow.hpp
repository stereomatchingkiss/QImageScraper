#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QNetworkReply>

#include <vector>

class image_search;

namespace Ui {
class MainWindow;
}

namespace qte{

namespace net{

class download_supervisor;

}}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:    
    void on_comboBoxSearchBy_activated(const QString &arg1);

    void on_actionGo_triggered();

private:
    void found_img_link(QString const &big_img_link, QString const &small_img_link);
    void download_finished(size_t unique_id, QNetworkReply::NetworkError code, QByteArray data);
    void download_image();
    void download_progress(size_t unique_id, qint64 bytesReceived, qint64 bytesTotal);

    Ui::MainWindow *ui;

    qte::net::download_supervisor *downloader_;
    std::vector<std::pair<QString, QString>> img_links_;
    image_search *img_search_;
};

#endif // MAINWINDOW_HPP
