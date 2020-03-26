#include <QCoreApplication>

#include <QNetworkAccessManager>
#include <QtWebSockets>
#include <QAbstractSocket>
#include <QFile>
#include <QRegExp>
#include <QVector>

#include "json.hpp"

using json = nlohmann::json;

QString getTime() { return QTime::currentTime().toString("HH:mm:ss:zzz"); }

static QVector<QString> tokens = {};
static QString mainToken;

void messageReceived(const QString &message, QWebSocket &ws, QNetworkAccessManager &mgr, int tokenId)
{
    QString token = tokens.at(tokenId);
    const json payload = json::parse(message.toStdString());
    const json data = payload["d"];
    const QString event = QString::fromUtf8(payload["t"].dump().c_str()).replace("\"", "");
    const int op = payload["op"];
    int sequence_number;

    if (payload["s"] != nullptr)
        sequence_number = payload["s"];
    else
        sequence_number = 0;

    switch (op) {
    case 0: // Events
        if (event == "READY")
            qDebug().noquote().nospace() << getTime() << " -> Nitro Sniper ready : "
                                         << QString::fromUtf8(data["user"]["username"].dump().c_str()).replace("\"", "")
                    << "#"
                    << QString::fromUtf8(data["user"]["discriminator"].dump().c_str()).replace("\"", "")
                    << " ("
                    << QString::fromUtf8(data["user"]["id"].dump().c_str()).replace("\"", "")
                    << ")";
        if (event == "MESSAGE_CREATE")
        {
            QString content = QString::fromUtf8(data["content"].dump().c_str()).replace("\"", "").replace("\\n", " ");
            if (!content.contains("discord.gift/")) return;

            QRegExp reg("(?:discord.gift/\\S+)");
            reg.indexIn(content);
            QString code = reg.cap().split("/").back();
            QString url = "https://discordapp.com/api/v6/entitlements/gift-codes/"+code+"/redeem";
            QNetworkRequest *req = new QNetworkRequest(QUrl(url));
            req->setRawHeader("Authorization", mainToken.toUtf8());
            req->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            req->setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.130 Safari/537.36 OPR/66.0.3515.115");
            QString data = "";
            mgr.post(*req, data.toUtf8());
            qDebug().noquote().nospace() << getTime() << " -> Nitro gift detected : https://discord.gift/" + code;
        }
        break;

    case 10: // Hello
        QTimer *timer = new QTimer();
        QObject::connect(timer, &QTimer::timeout, [&] {
            ws.sendTextMessage("{\"op\":1,\"d\":" +
                               QString::number(sequence_number)
                               + "}");
        });
        timer->start(int(data["heartbeat_interval"]));
        ws.sendTextMessage("{\"op\":2,\"d\":{\"token\":\"" + token + "\",\"properties\":{\"$os\":\"Windows\",\"$browser\":\"Chrome\",\"$device\":\"\"},\"intents\":4608}}");
        break;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QNetworkAccessManager *mgr = new QNetworkAccessManager();

    QObject::connect(mgr, &QNetworkAccessManager::finished, &a, [&](QNetworkReply *reply) {
        switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) {
        case 200:
            qDebug().noquote().nospace() << getTime() << " -> Congratulations ! You now have Discord Nitro !";
            a.exit(0);
            break;
        default:
            qDebug().noquote().nospace() << getTime() << " -> Invalid Nitro gift code";
            break;
        }
    });

    QFile f(QCoreApplication::applicationDirPath() + "/tokens.txt");
    if (f.open(QIODevice::ReadOnly))
    {
        QTextStream in(&f);
        int i = 0;
        while (!in.atEnd())
        {
            QString token = in.readLine();
            if (i == 0) mainToken = token;
            tokens.push_back(token);

            ++i;
        }
        f.close();

    } else {
        qDebug().noquote().nospace() << getTime() << " -> No tokens provided ; Please create \"" << QCoreApplication::applicationDirPath() + "/tokens.txt" << "\". The first token is your main account, and the following ones are the snipers.";
        exit(0);
    }
    if (tokens.size() == 0)
    {
        qDebug().noquote().nospace() << getTime() << " -> No tokens provided ; Please create \"" << QCoreApplication::applicationDirPath() + "/tokens.txt" << "\". The first token is your main account, and the following ones are the snipers.";
        exit(0);
    }

    int i = 0;
    QTimer *login = new QTimer();
    QObject::connect(login, &QTimer::timeout, &a, [&]{
        if (i >= tokens.size())
        {
            delete login;
            return;
        }
        QWebSocket *ws = new QWebSocket();
        ws->open(QUrl("wss://gateway.discord.gg/?encoding=json&v=6"));

        QObject::connect(ws, &QWebSocket::textMessageReceived, &a, [=](QString const &message) { messageReceived(message, *ws, *mgr, i); });
        QObject::connect(ws, &QWebSocket::connected, &a, [&]{ qDebug().noquote().nospace() << getTime() << " -> Connection etablished."; });
        QObject::connect(ws, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                         [=](QAbstractSocket::SocketError error){ qDebug().noquote().nospace() << getTime() << " -> Error : " << error; });
        QObject::connect(ws, &QWebSocket::disconnected, &a, [&]{ qDebug().noquote().nospace() << getTime() << " -> Connection terminated."; });
        ++i;
    });
    login->start(5000);

    return a.exec();
}
