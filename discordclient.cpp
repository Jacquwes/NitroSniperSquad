#include "constants.h"
#include "discordclient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QWebSocket>

void DiscordClient::login(const QString& token)
{
	m_token = token;

	connect(m_networkManager, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply)
	{
		disconnect(m_networkManager, &QNetworkAccessManager::finished, this, nullptr);
		QJsonDocument jsonReply = QJsonDocument::fromJson(reply->readAll());

		m_websocket->open(jsonReply.object().find("url").value().toString());
	});
	QNetworkRequest gatewayRequest(Constants::DiscordAPI::gatewayURI());
	gatewayRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	m_networkManager->get(gatewayRequest);
}

void DiscordClient::messageReceived(const QString& message)
{
	QJsonObject payload = QJsonDocument::fromJson(message.toUtf8()).object();

	// Switch on OP
	switch (static_cast<Constants::Gateway::OpCode>(payload.find("op").value().toInt()))
	{
	case Constants::Gateway::OpCode::DISPATCH:
	{
		if (!payload.find("s").value().isNull())
			m_sequenceNumber = payload.find("s").value().toInt();

		QString eventName = payload.find("t").value().toString();
		if (eventName == "READY")
		{
			// Resume if needed
			m_loggedIn = true;
			m_sessionId = payload.find("d").value().toObject().find("session_id").value().toString();
			m_user.id = payload.find("d").value().toObject().find("user").value().toObject().find("id").value().toString();
			m_user.username = payload.find("d").value().toObject().find("user").value().toObject().find("username").value().toString();
			m_user.discriminator = payload.find("d").value().toObject().find("user").value().toObject().find("discriminator").value().toString();
			emit onReady(m_user);
		}
		else if (eventName == "MESSAGE_CREATE")
		{
			User author;
			author.id = payload.find("d").value().toObject().find("author").value().toObject().find("id").value().toString();
			author.username = payload.find("d").value().toObject().find("author").value().toObject().find("username").value().toString();
			author.discriminator = payload.find("d").value().toObject().find("author").value().toObject().find("discriminator").value().toString();

			Message message;
			message.author = author;
			message.content = payload.find("d").value().toObject().find("content").value().toString();
			message.channelId = payload.find("d").value().toObject().find("channel_id").value().toString();

			emit onMessage(message);
		}
		break;
	}
	case Constants::Gateway::OpCode::INVALID_SESSION:
	{
		// Identify
		QJsonDocument identifyPayload;
		identifyPayload.object().insert("op", (static_cast<int>(Constants::Gateway::OpCode::IDENTIFY)));
		QJsonObject data;
		data.insert("intents",
					static_cast<int>(Constants::Gateway::Intents::GUILD_MESSAGES)
					| static_cast<int>(Constants::Gateway::Intents::DIRECT_MESSAGES));
		data.insert("token", m_token);
		QJsonObject properties;
		properties.insert("$os", "Windows");
		properties.insert("$device", "");
		properties.insert("$browser", "Chrome");
		data.insert("properties", properties);
		identifyPayload.object().insert("d", data);

		QTimer* timer = new QTimer;
		connect(timer, &QTimer::timeout, this, [&]
		{
			m_websocket->sendTextMessage(identifyPayload.toJson(QJsonDocument::JsonFormat::Compact));
			delete timer;
		});
		timer->start((rand() % 5 + 1) * 1000);
	}
	case Constants::Gateway::OpCode::HELLO:
	{
		// Send heartbeats
		m_heartbeat->start(payload.find("d").value().toObject().find("heartbeat_interval").value().toInt());
		if (m_loggedIn)
		{
			// Resume
			QJsonDocument resumePayload;
			resumePayload.object().insert("op", static_cast<int>(Constants::Gateway::OpCode::RESUME));
			QJsonObject data;
			data.insert("token", m_token);
			data.insert("session_id", m_sessionId);
			data.insert("seq", m_sequenceNumber);
			resumePayload.object().insert("d", data);
			m_websocket->sendTextMessage(resumePayload.toJson(QJsonDocument::JsonFormat::Compact));
		}
		else
		{
			// Identify
			QJsonDocument identifyPayload;
			identifyPayload.object().insert("op", (static_cast<int>(Constants::Gateway::OpCode::IDENTIFY)));
			QJsonObject data;
			data.insert("intents",
						static_cast<int>(Constants::Gateway::Intents::GUILD_MESSAGES)
						| static_cast<int>(Constants::Gateway::Intents::DIRECT_MESSAGES));
			data.insert("token", m_token);
			QJsonObject properties;
			properties.insert("$os", "Windows");
			properties.insert("$device", "");
			properties.insert("$browser", "Chrome");
			data.insert("properties", properties);
			identifyPayload.object().insert("d", data);
			m_websocket->sendTextMessage(identifyPayload.toJson(QJsonDocument::JsonFormat::Compact));
		}
		break;
	}
	default:
		break;
	}
}

void DiscordClient::connectionClosed()
{
	qDebug() << "Connection closed (" << (m_loggedIn ? m_user.username + "#" + m_user.discriminator : "not logged in") << ")";
	qDebug() << "Trying to reconnect...";
	m_heartbeat->stop();
	login(m_token);
}

void DiscordClient::connectionError(QAbstractSocket::SocketError error)
{
	qDebug() << error;
}

void DiscordClient::sendHeartbeat()
{
	QJsonDocument payload;
	payload.object().insert("op", static_cast<int>(Constants::Gateway::OpCode::HEARTBEAT));
	payload.object().insert("d", m_sequenceNumber);
	m_websocket->sendTextMessage(payload.toJson(QJsonDocument::JsonFormat::Compact));
}
