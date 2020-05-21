#ifndef DISCORDCLIENT_H
#define DISCORDCLIENT_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QWebSocket>

#include "constants.h"

struct User
{
	QString discriminator;
	QString id;
	QString username;
};

struct Message
{
	User author;
	QString channelId;
	QString content;
};

class DiscordClient : public QObject
{
	Q_OBJECT
public:
	inline explicit DiscordClient(QNetworkAccessManager* networkManager)
		: m_loggedIn(false),
		  m_networkManager(networkManager),
		  m_heartbeat(new QTimer),
		  m_websocket(new QWebSocket)
	{
		m_sequenceNumber = 0;

		connect(m_websocket, &QWebSocket::disconnected, this, &DiscordClient::connectionClosed);
		connect(m_websocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &DiscordClient::connectionError);
		connect(m_websocket, &QWebSocket::textMessageReceived, this, &DiscordClient::messageReceived);

		connect(m_heartbeat, &QTimer::timeout, this, &DiscordClient::sendHeartbeat);
	};

	void login(const QString& token);

	inline bool loggedIn() { return m_loggedIn; }
	inline const User user() { return m_user; }

private:
	bool m_loggedIn;
	int m_sequenceNumber;
	QString m_sessionId;
	QString m_token;
	User m_user;

	QNetworkAccessManager* m_networkManager;
	QTimer* m_heartbeat;
	QWebSocket* m_websocket;

signals:
	void onMessage(const Message& message);
	void onReady(const User& user);

private slots:
	void connectionClosed();
	void connectionError(QAbstractSocket::SocketError error);
	void messageReceived(const QString& message);
	void sendHeartbeat();
};

#endif // DISCORDCLIENT_H
