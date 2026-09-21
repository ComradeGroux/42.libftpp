#pragma once

#include "../message/message.hpp"
#include "../../threading/thread/thread.hpp"
#include "../../threading/thread_safe_queue/thread_safe_queue.hpp"

#include <vector>
#include <functional>
#include <exception>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <poll.h>

class Server
{
	private:
		struct ClientContext {
			int							sockFd = -1;
			size_t						bytesReaded = 0;
			Message::Header				header;
			bool						headerReaded = false;
			std::vector<unsigned char>	buffer;
		};
		struct MessageClient {
			long long	clientID;
			Message		message;
		};

		std::mutex			_mutexListeningSocket;
		struct pollfd		_sockListeningFd[2];
		Thread				_acceptor;
		void				_acceptorLoop(void);

		std::atomic_bool	_isRunning;

		std::mutex										_mutexClientMap;
		std::unordered_map<long long, ClientContext>	_clients;
		long long										_clientCounterForID = 0;

		Thread		_receiver;
		std::mutex	_mutexReceiverEpoll;
		int			_receiverEpollFd;
		void		_receiverLoop(void);
		void		_readMessage(const long long& clientId);
		void		_readMessageHeader(const long long& id, ClientContext& context);
		void		_readMessageBody(const long long& id, ClientContext& context);

		ThreadSafeQueue<MessageClient>	_messageReceived;

		std::unordered_map<Message::Type, std::function<void(const long long& clientID, const Message&)>>	_actions;

		void	_setupIPv4Sock(const size_t& port);
		void	_setupIPv6Sock(const size_t& port);
		void	_stop(void);
		int		_acceptClient(int listeningSocket);
		void	_disconnectClient(const long long& clientID);

	public:
		Server(void);
		~Server(void);

		void	start(const size_t& p_port);

		void	sendTo(const Message& message, long long clientID);
		void	sendToArray(const Message& message, std::vector<long long> clientIDs);
		void	sendToAll(const Message& message);

		void	defineAction(const Message::Type& messageType, const std::function<void(const long long& clientID, const Message& msg)>& action);
		void	update(void);

		class AlreadyStartedException : public std::exception {
			public:
				const char*	what(void) const noexcept { return "Server: Already started !"; }
		};
		class ServerNotStartedException : public std::exception {
			public:
				const char* what(void) const noexcept { return "Server: Is not started !"; }
		};
};
