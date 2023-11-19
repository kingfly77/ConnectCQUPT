#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    messageTitle_ = "Connect CQUPT";
    destAddress_ = "http://192.168.200.2:801";

    localIP_ = getLocalIP();
    ui->lineIP->setText(localIP_);

    ui->linePassword->setEchoMode(QLineEdit::Password);

    networkManager_ = new QNetworkAccessManager(this);
    connect(ui->btnLogin, &QPushButton::clicked, this, &MainWindow::login);
    connect(ui->btnLogout, &QPushButton::clicked, this, &MainWindow::logout);
    connect(ui->btnCancel, &QPushButton::clicked, this, &MainWindow::close);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::login()
{
    QString netServer = getNetServer();
    if (netServer == "")
    {
        QMessageBox::information(this, messageTitle_, "请选择网络运营商");
        return;
    }
    QString ip = ui->lineIP->text();
    if (ip == "")
    {
        QMessageBox::information(this, messageTitle_, "请输入要登录的IP");
        return;
    }
    QString account = ui->lineAccount->text();
    if (account == "")
    {
        QMessageBox::information(this, messageTitle_, "请输入账号");
        return;
    }
    QString password = ui->linePassword->text();
    if (password == "")
    {
        QMessageBox::information(this, messageTitle_, "请输入密码");
        return;
    }
    QString url = getLoginURL(destAddress_, ip, netServer, account, password);

    getRequest_.setUrl(QUrl(url));
    getRequest_.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    getRequest_.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    loginReply_ = networkManager_->get(getRequest_);
    connect(loginReply_, &QNetworkReply::finished, this, &MainWindow::loginReplyFinished);
}

void MainWindow::logout()
{
    QString ip = ui->lineIP->text();
    if (ip == "")
    {
        QMessageBox::information(this, messageTitle_, "请输入要注销的IP");
        return;
    }
    QString url = getLogoutURL(destAddress_, ip);
    getRequest_.setUrl(QUrl(url));
    getRequest_.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    getRequest_.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    logoutReply_ = networkManager_->get(getRequest_);
    connect(logoutReply_, &QNetworkReply::finished, this, &MainWindow::logoutReplyFinished);
}

QString MainWindow::getNetServer()
{
    QString netServer = "";
    if (ui->rBtn1->isChecked())
    {
        netServer = "telecom";
    }
    else if (ui->rBtn2->isChecked())
    {
        netServer = "cmcc";
    }
    else if (ui->rBtn3->isChecked())
    {
        netServer = "unicom";
    }
    else if (ui->rBtn4->isChecked() || ui->rBtn5->isChecked())
    {
        netServer = "xyw";
    }
    return netServer;
}

QString MainWindow::getLoginURL(QString destAddress, QString loginIP, QString netServer, QString account, QString password)
{
    QString url = destAddress;
    url += "/eportal/?c=Portal&a=login&callback=dr1003&login_method=1&user_account=%2C0%2C" + account;
    url += "%40" + netServer;
    url += "&user_password=" + password;
    url += "&wlan_user_ip=" + loginIP;
    int randNum = 1000 + rand() % 5000;
    url += "&wlan_user_ipv6=&wlan_user_mac=000000000000&wlan_ac_ip=&wlan_ac_name=&jsVersion=3.3.3&v=" + QString::number(randNum, 10);
    qDebug() << url;
    return url;
}

QString MainWindow::getLogoutURL(QString destAddress, QString ip)
{
    QString url = destAddress;
    url += "/eportal/?c=Portal&a=logout&callback=dr1003&login_method=1&user_account=drcom&user_password=123&ac_logout=1&register_mode=1&";
    url += "wlan_user_ip=" + ip;
    int randNum = 1000 + rand() % 5000;
    url += "&wlan_user_ipv6=&wlan_vlan_id=1&wlan_user_mac=000000000000&wlan_ac_ip=&wlan_ac_name=&jsVersion=3.3.3&v=" + QString::number(randNum, 10);
    return url;
}

void MainWindow::loginReplyFinished()
{
    if (loginReply_->error()){
        QString error = loginReply_->errorString();
        qDebug() << error;
        QMessageBox::warning(this, messageTitle_, "请求失败：" + error);
        loginReply_->deleteLater();
        return;
    }

    int statusCode = loginReply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "statusCode:" << statusCode;

    if (statusCode == 200)
    {
        QTextCodec *codec = QTextCodec::codecForName("utf8");
        QString allBack = codec->toUnicode(loginReply_->readAll());
//        QByteArray bytes =allBack.toUtf8();
        qDebug() << "接收到的数据: " << allBack;
        QString result = allBack.section('"', 3, 3);
        qDebug() << result;
        if (result == "1")
        {
            QMessageBox::information(this, "", "登录成功");
            return;
        }
        else
        {
            QString retCode = allBack.section('"', 10, 10);
            if (retCode == ":1})")
            {
                QMessageBox::warning(this, messageTitle_, "登录失败，请检查运营商、账号及密码");
                return;
            }
            else if (retCode == ":2})")
            {
                QMessageBox::warning(this, messageTitle_, "终端IP已在线");
                return;
            }
        }
        qDebug() << "接收到的数据: " << allBack;
    }
    else
    {
        QMessageBox::warning(this, messageTitle_, "网络连接错误，请重试：" + statusCode);
        return;
    }

}

void MainWindow::logoutReplyFinished()
{
    qDebug() << "logout finish";
    qDebug() << logoutReply_->error();
    if (logoutReply_->error()){
        QString error = logoutReply_->errorString();
        qDebug() << error;
        QMessageBox::warning(this, messageTitle_, "请求失败：" + error);
        logoutReply_->deleteLater();
        return;
    }
    int statusCode = logoutReply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "statusCode:" << statusCode;

    if (statusCode == 200)
    {
        QTextCodec *codec = QTextCodec::codecForName("utf8");
        QString allBack = codec->toUnicode(logoutReply_->readAll());
        qDebug() << "接收到的数据: " << allBack;
        QString result = allBack.section('"', 3, 3);
        qDebug() << result;
        // todo
        if (result == "1")
        {
            QMessageBox::information(this, messageTitle_, "注销成功");
            return;
        }
        else
        {
            QMessageBox::warning(this, messageTitle_, "注销失败，可能已注销");
            return;
        }
        qDebug() << "接收到的数据: " << allBack;
    }
    else
    {
        QMessageBox::warning(this, messageTitle_, "网络连接错误，请重试：" + statusCode);
        return;
    }
}

QString MainWindow::getLocalIP()
{
    QList<QHostAddress> list=QNetworkInterface::allAddresses();
    foreach(QHostAddress address,list)
    {
        if(address.protocol()==QAbstractSocket::IPv4Protocol)
        {
            QString ip = address.toString();
            if (ip.startsWith("10."))
            {
                return ip;
            }
        }
    }
    return "";
}


