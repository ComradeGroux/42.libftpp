#pragma once

#include "../message/message.hpp"
#include "../../threading/thread/thread.hpp"
#include "../../threading/thread_safe_queue/thread_safe_queue.hpp"

#include <functional>
#include <unordered_map>
#include <atomic>
#include <stdexcept>

class Client
{
	private:
		int					_sockFd;
		std::atomic_bool	_connected;

		std::unordered_map<Message::Type, std::function<void(const Message&)>>	_actions;

		Thread						_receiver;
		ThreadSafeQueue<Message>	_messageReceived;
		void						_receiverLoop(void);

	public:
		Client(void);
		~Client(void);

		void	connect(const std::string& address, const size_t& port);
		void	disconnect(void);
		bool	isConnected(void) const noexcept;

		void	defineAction(const Message::Type& messageType, const std::function<void(const Message&)>& action);

		void	send(const Message& message);
		void	update(void);

		class AlreadyConnectedException : public std::exception {
			public:
				const char*	what(void) const noexcept { return "Client: Already connected !"; }
		};
};
