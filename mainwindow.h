#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHostInfo>
#include <QMessageBox>
#include <QTextCodec>
#include <QNetworkInterface>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QString messageTitle_;
    QString localIP_;
    QString destAddress_;
    QNetworkRequest getRequest_;
    QNetworkAccessManager *networkManager_;
    QNetworkReply *loginReply_;
    QNetworkReply *logoutReply_;

    QString getNetServer();
    QString getLoginURL(QString destAddress, QString loginIP, QString netServer, QString account, QString password);
    QString getLogoutURL(QString destAddress, QString ip);
    QString getLocalIP();

private slots:
    void login();
    void logout();
    void loginReplyFinished();
    void logoutReplyFinished();
};

#endif // MAINWINDOW_H
