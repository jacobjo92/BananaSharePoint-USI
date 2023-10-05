#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QEventLoop>
#include <QDebug>
#include <QtNetworkAuth>
#include <QDesktopServices>
#include <QAbstractOAuth2>


class Authenticator : public QObject
{
    Q_OBJECT
public:
    explicit Authenticator(QObject *parent = nullptr);
    bool isPermanent() const;
    void setPermanent(bool value);

public slots:
    void grant();
    void putRequest();

private slots:
    //    void start();
    void handleAuthorizationCallback(const QVariantMap &parameters);
    void onManagerFinished(QNetworkReply *reply);
//    QString exchangeAuthorizationCodeForAccessToken(const QString &authorizationCode);
    //    void startAuthorizationFlow();
    //    void onReplyFinished(QNetworkReply *reply);
    //    QString buildAuthorizationUrl();
    //    QString exchangeAuthorizationCodeForAccessToken(const QString &authorizationCode);

signals:
    void authenticated();
    void subscribed(const QUrl &url);
private:
    QNetworkAccessManager m_manager;
    QOAuth2AuthorizationCodeFlow oauth2;
    bool permanent = false;
    void getAccessToken();
    QString m_accessToken;

};

#endif // SERVER_H
