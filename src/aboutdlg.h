#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
class AboutDlg;
}

class AboutDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDlg(QWidget *parent = nullptr);
    ~AboutDlg();

private:
    void checkUpdate();
    void httpReplied(QNetworkReply* reply);

private:
    Ui::AboutDlg *ui;
    QNetworkAccessManager mNetwork;
};

#endif // ABOUTDLG_H
