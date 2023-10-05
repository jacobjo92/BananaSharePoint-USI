#include "authenticator.h"
#include "secrets.h"

// Necessary URLs for oauth2 and Put request
constexpr char c_urlGetToken[] = "https://login.microsoftonline.com/687e1310-0790-4edc-8391-b528803559fc/oauth2/v2.0/token";
constexpr char  putUrl[] = "https://graph.microsoft.com/v1.0/Drives/b!cCo7iPB_3kmN1pe_6XINv2tdCZiTZ7tPokmekPhi9XFxwLteMxliRLxqhneeofXe/root/Children/roots.txt/content";


// Authenticator constructor with all the needed appropriate methods.
Authenticator::Authenticator(QObject *parent) : QObject(parent), m_manager(this)
{

    // Create a QOAuthHttpServerReplyHandler to handle the OAuth2 replies. listens on port 3000 for incoming HTTP requests.
    auto authReply = new QOAuthHttpServerReplyHandler(3000, this);

    // Configure the OAuth2 settings for Microsoft authentication
    // Set the authorization and access token URLs, as well as the desired scope.
    oauth2.setReplyHandler(authReply);
    oauth2.setAuthorizationUrl(QUrl("http://localhost:3000/authdone"));
    oauth2.setAccessTokenUrl(QUrl("https://login.microsoftonline.com/687e1310-0790-4edc-8391-b528803559fc/oauth2/v2.0/token"));
    //    oauth2.setScope("User.Read");
    oauth2.setScope("https://graph.microsoft.com/.default");
    //    oauth2.setScope("api://c4686722-538a-4ddf-ae75-e2f4480788be/.default");

    // Connecct a slot to the statusChanged signal of the OAuth2 object.
    // When the OAuth2 status changes to Granted, it emits the authenticated signal.
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged,this, [=](QAbstractOAuth::Status status) {
        if (status == QAbstractOAuth::Status::Granted){
            emit authenticated();
        }
    });

    // Configure a function to modify the OAuth2 parameters before sending a request.
    // In this case, if the stage is requesting authorization and isPermanent() is true,
    // the "duration" parameter is added with the value "permanent".
    oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters) {
        parameters->insert("client_id", "c4686722-538a-4ddf-ae75-e2f4480788be");
        parameters->insert("client_secret", Secrets::MySecret);

    });

    // Connect the authorizeWithBrowser signal of the OAuth2 object to QDesktopServices::openUrl,
    // which will open the authorization URL in the user's default web browser.
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);


    connect(&oauth2, &QAbstractOAuth2::authorizationCallbackReceived, this, &Authenticator::handleAuthorizationCallback);


    connect(&m_manager, &QNetworkAccessManager::finished, this, &Authenticator::onManagerFinished);
}


// Once login is complete begin the exchange for accessToken
void Authenticator::handleAuthorizationCallback(const QVariantMap &data){
//    qDebug() << "Authorization callback received:" << data;
    getAccessToken();
}

// Bool to check if connection is kept alive permanent or nor
bool Authenticator::isPermanent() const{
    return permanent;
}

// Grant starts the authentication process
void Authenticator::grant(){
    oauth2.grant();
}


// Function to to handle replies based on which Urls the request was made out to
void Authenticator::onManagerFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << "Error_l69:" << reply->error();
    } else {
        QByteArray data = reply->readAll();
        QString responseString = QString::fromUtf8(data);

        // Parse the JSON response
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseString.toUtf8());

        // Check if the JSON is an object
        if (reply->url().toString() == QString(c_urlGetToken)){
            if (!jsonResponse.isObject()) {
                qDebug() << "Invalid JSON format.";
            } else {
                // Extract the access token from the JSON object
                QJsonObject jsonObject = jsonResponse.object();
                if (jsonObject.contains("access_token") && jsonObject["access_token"].isString()) {
                    this->m_accessToken = jsonObject["access_token"].toString();
                    qDebug() << "Access Token:" << m_accessToken;

//                    putRequest();
                } else {
                    qDebug() << "Access token not found or not a string.";
                }
            }
        } else if(reply->url().toString() == QString(c_urlGetToken)){
            if (reply->error() == QNetworkReply::NoError) {
                qDebug() << "Upload successful!";
            } else {
                qDebug() << "Error_l99:" << reply->errorString();
            }
            reply->deleteLater();
        }
    }
    reply->deleteLater();
}


// Function to get the access token
// !!Remember authorization callback must be handled before calling this function.
void Authenticator::getAccessToken() {

    QUrl url("https://login.microsoftonline.com/687e1310-0790-4edc-8391-b528803559fc/oauth2/v2.0/token");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray postData;
    postData.append("grant_type=client_credentials");
    postData.append("&client_id=c4686722-538a-4ddf-ae75-e2f4480788be");
    postData.append("&client_secret=sd.8Q~gVOre9L2YW.0SfnSIffV-xUY_1XmnfrblL");
    postData.append("&scope=https://graph.microsoft.com/.default");

    m_manager.post(request, postData);
}

// Function to do a Put request to upload a test file.
void Authenticator::putRequest(){

    QUrl url("https://graph.microsoft.com/v1.0/Drives/b!cCo7iPB_3kmN1pe_6XINv2tdCZiTZ7tPokmekPhi9XFxwLteMxliRLxqhneeofXe/root/Children/helloWorld.txt/content");
    QFile file("C:/Users/Johan.Jacob/Documents/GitHub/SharePoint_Banana/server/trial.txt");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error opening file:" << file.errorString();
        return ;
    }
    QByteArray fileData = file.readAll();
    file.close();

    QNetworkRequest request(url);
    // Set headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray authHeaderValue = "Bearer " + m_accessToken.toUtf8();
    request.setRawHeader("Authorization", authHeaderValue);
    m_manager.put(request, fileData);
}
