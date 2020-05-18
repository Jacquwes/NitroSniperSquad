#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
namespace DiscordAPI {
	inline QString baseURL() { return "https://discord.com/api"; }
	inline QString gatewayURI() { return baseURL() + "/gateway"; }
	enum class redeemResponseErrorCode
	{
		INVALID = 10038,
		ALREADY_REDEEMED = 50050,
	};
}
namespace Gateway {
	enum class OpCode
	{
		DISPATCH,
		HEARTBEAT,
		IDENTIFY,
		PRESENCE_UPDATE,
		VOICE_STATE_UPDATE,
		VOICE_SERVER_PING,
		RESUME,
		RECONNECT,
		REQUEST_GUILD_MEMBERS,
		INVALID_SESSION,
		HELLO,
		HEARTBEAT_ACK
	};

	enum class Intents
	{
		GUILDS						= 1 << 0,
		GUILD_MEMBERS				= 1 << 1,
		GUILD_BANS					= 1 << 2,
		GUILD_EMOJIS				= 1 << 3,
		GUILD_INTEGRATIONS			= 1 << 4,
		GUILD_WEBHOOKS				= 1 << 5,
		GUILD_INVITES				= 1 << 6,
		GUILD_VOICE_STATES			= 1 << 7,
		GUILD_PRESENCES				= 1 << 8,
		GUILD_MESSAGES				= 1 << 9,
		GUILD_MESSAGE_REACTIONS		= 1 << 10,
		GUILD_MESSAGE_TYPING		= 1 << 11,
		DIRECT_MESSAGES				= 1 << 12,
		DIRECT_MESSAGE_REACTIONS	= 1 << 13,
		DIRECT_MESSAGE_TYPING		= 1 << 14
	};
}
}

#endif // CONSTANTS_H
