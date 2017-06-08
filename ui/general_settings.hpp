#ifndef GENERAL_SETTINGS_HPP
#define GENERAL_SETTINGS_HPP

#include <QDialog>

namespace Ui {
class general_settings;
}

class general_settings : public QDialog
{
    Q_OBJECT

public:
    explicit general_settings(QWidget *parent = nullptr);
    ~general_settings();

    void accept_settings();
    int get_max_download_img() const;
    QString get_save_at() const;
    QString get_search_by() const;
    bool search_by_changed() const;
    void reject_settings();

private slots:
    void on_pushButtonSaveAt_clicked();

private:
    void closeEvent(QCloseEvent *event) override;

    void restore_value();

    Ui::general_settings *ui;

    bool search_by_changed_;
    QString const write_able_path_;
};

#endif // GENERAL_SETTINGS_HPP
