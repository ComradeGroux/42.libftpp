#include "server.hpp"

#include "../network.hpp"
#include "../../io_stream/thread_safe_iostream.hpp"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <iostream>
#include <errno.h>
#include <memory>

#ifndef MAX_EVENT
# define MAX_EVENT 100
#endif

Server::Server(void) :	_sockListeningFd{{-1, 0, 0}, {-1, 0, 0}}, _acceptor("ServerAcceptor", [this]() { _acceptorLoop(); }),
						_isRunning(false),
						_receiver("ServerReceiver", [this]() { _receiverLoop(); }), _receiverEpollFd(epoll_create(1))
{
	if (_receiverEpollFd == -1)
		throw std::runtime_error("Failed to create the EPoll FD");
}

Server::~Server(void)
{
	_stop();
}

void	Server::_setupIPv4Sock(const size_t& port)
{
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: Setting up IPv4 socket" << std::endl;
#endif
	_sockListeningFd[0].fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_sockListeningFd[0].fd == -1)
	{
		std::string	error = "Server: socket: ";
		error += strerror(errno);
		throw std::runtime_error(error);
	}
	const int	enable = 1;
	setsockopt(_sockListeningFd[0].fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	setsockopt(_sockListeningFd[0].fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

	sockaddr_in	servAddr{};
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(static_cast<uint16_t>(port));
	if (bind(_sockListeningFd[0].fd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
	{
		std::string	error = "Server: bind: ";
		error += strerror(errno);
		close(_sockListeningFd[0].fd);
		throw std::runtime_error(error);
	}

	if (listen(_sockListeningFd[0].fd, 10) < 0)
	{
		std::string	error = "Server: listen: ";
		error += strerror(errno);
		close(_sockListeningFd[0].fd);
		throw std::runtime_error(error);
	}
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: IPv4 socket created" << std::endl;
#endif
}

void	Server::_setupIPv6Sock(const size_t& port)
{
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: Setting up IPv6 socket" << std::endl;
#endif
	_sockListeningFd[1].fd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_sockListeningFd[1].fd == -1)
	{
		close(_sockListeningFd[0].fd);
		std::string	error = "Server: socket: ";
		error += strerror(errno);
		throw std::runtime_error(error);
	}
	const int	enable = 1;
	setsockopt(_sockListeningFd[1].fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	setsockopt(_sockListeningFd[1].fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));
	setsockopt(_sockListeningFd[1].fd, IPPROTO_IPV6, IPV6_V6ONLY, &enable, sizeof(int));

	sockaddr_in6	servAddr{};
	servAddr.sin6_family = AF_INET6;
	servAddr.sin6_addr = in6addr_any;
	servAddr.sin6_port = htons(static_cast<uint16_t>(port));
	if (bind(_sockListeningFd[1].fd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
	{
		std::string	error = "Server: bind: ";
		error += strerror(errno);
		close(_sockListeningFd[0].fd);
		close(_sockListeningFd[1].fd);
		throw std::runtime_error(error);
	}

	if (listen(_sockListeningFd[1].fd, 10) < 0)
	{
		std::string	error = "Server: listen: ";
		error += strerror(errno);
		close(_sockListeningFd[0].fd);
		close(_sockListeningFd[1].fd);
		throw std::runtime_error(error);
	}
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: IPv6 socket created" << std::endl;
#endif
}

void	Server::start(const size_t& p_port)
{
	if (_isRunning)
		throw AlreadyStartedException();

	_setupIPv4Sock(p_port);
	_setupIPv6Sock(p_port);

	_isRunning = true;
	_acceptor.start();
	_receiver.start();
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: Started !" << std::endl;
#endif
}

void	Server::_stop(void)
{
	if (!_isRunning)
		return;

	_isRunning = false;

	{
		std::lock_guard<std::mutex>	lock(_mutexListeningSocket);
		_acceptor.stop();
		close(_sockListeningFd[0].fd);
		_sockListeningFd[0].fd = -1;
		close(_sockListeningFd[1].fd);
		_sockListeningFd[1].fd = -1;
	}
	
	std::lock_guard<std::mutex>	lock(_mutexClientMap);
	for (std::unordered_map<long long, ClientContext>::iterator it = _clients.begin(); it != _clients.end();)
	{
		shutdown(it->second.sockFd, SHUT_RDWR);
		close(it->second.sockFd);
		it = _clients.erase(it);
	}
	_receiver.stop();
	close(_receiverEpollFd);
}

int	Server::_acceptClient(int listeningSocket)
{
#ifdef DEBUG
	threadSafeCout << "[DEBUG] Server: New client trying to establish a connection" << std::endl;
#endif
	int					sockFd = -1;
	sockaddr_storage	clientAddr{};
	socklen_t			addrLen = sizeof(clientAddr);
	{
		std::lock_guard<std::mutex>	lock(_mutexListeningSocket);
		sockFd = accept4(listeningSocket, (struct sockaddr*)&clientAddr, &addrLen, SOCK_NONBLOCK);
	}
	if (sockFd < 0)
#ifdef DEBUG
	{
		std::string err = "[DEBUG] Server: accept error: ";
		err += strerror(errno);
		threadSafeCout << err << std::endl;
		return errno;
	}
#else
		return errno;
#endif

	ClientContext	context;
	context.sockFd = sockFd;
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		_clients[_clientCounterForID] = context;
	}
	
	struct epoll_event	vnt;
	vnt.events = EPOLLIN;
	vnt.data.fd = sockFd;
	vnt.data.u64 = _clientCounterForID++;
	epoll_ctl(_receiverEpollFd, EPOLL_CTL_ADD, context.sockFd, &vnt);
#ifdef DEBUG
	threadSafeCout << "[DEBUG] Server: New client accepted" << std::endl;
#endif
	return 0;
}

void	Server::_disconnectClient(const long long& clientID)
{
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: Going to disconnect client" << std::endl;
#endif
	std::lock_guard<std::mutex>	lock(_mutexClientMap);
	std::unordered_map<long long, ClientContext>::iterator	it = _clients.find(clientID);
	if (it != _clients.end())
	{
		epoll_ctl(_receiverEpollFd, EPOLL_CTL_DEL, it->second.sockFd, nullptr);
		close(it->second.sockFd);
		_clients.erase(it);
	}
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: Client removed from client map" << std::endl;
#endif
}

void	Server::defineAction(const Message::Type& messageType, const std::function<void(const long long& clientID, const Message& msg)>& action)
{
	_actions[messageType] = action;
}

void	Server::sendTo(const Message& message, long long clientID)
{
	if (!_isRunning)
		throw ServerNotStartedException();
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: Going to send a message to client " << clientID << std::endl;
#endif
	ClientContext	context;
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		std::unordered_map<long long, ClientContext>::iterator it = _clients.find(clientID);
		if (it == _clients.end())
		{
			threadSafeCerr << "Server: Client ID inexistant: " << clientID << std::endl;
			return;
		}
		context = it->second;
	}

	Message::Header	hdr;
	hdr.size = message._rawSize();
	hdr.type = message.type();

	std::vector<uint8_t>	buff;
	buff.resize(sizeof(hdr));
	std::memcpy(buff.data(), &hdr, sizeof(hdr));
	buff.insert(buff.end(), message._rawData(), message._rawData() + message._rawSize());
	if (buff.size() > MAX_MESSAGE_SIZE_MB * 1000000)
	{
		threadSafeCerr << "Server: Message exceed max size (" << buff.size() / 1000000.0 << " MB / " << MAX_MESSAGE_SIZE_MB << "MB)\n";
		return;
	}

	size_t	bytesSended = 0;
	while (bytesSended < buff.size())
	{
		ssize_t	s = ::send(context.sockFd, buff.data() + bytesSended, buff.size() - bytesSended, 0);
		if (s <= 0)
		{
			if (s < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
				continue; // A MODIFIER AVEC EPOLLOUT LORSQUE CE SERA LE TEMPS DE FULL OPTIMISER
			threadSafeCerr << "Server: Error with the client connexion\n";
			_disconnectClient(clientID);
			return;
		}
		bytesSended += s;
	}
#ifdef DEBUG
threadSafeCout << "[DEBUG] Server: Message sended" << std::endl;
#endif
}

void	Server::sendToArray(const Message& message, std::vector<long long> clientIDs)
{
	for (std::vector<long long>::iterator it = clientIDs.begin(); it != clientIDs.end(); it++)
		sendTo(message, *it);
}

void	Server::sendToAll(const Message& message)
{
	std::unordered_map<long long, ClientContext>	copy;
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		copy = _clients;
	}

	for (std::unordered_map<long long, ClientContext>::const_iterator it = copy.begin(); it != copy.end(); it++)
		sendTo(message, it->first);
}

void	Server::update(void)
{
	std::unordered_map<Message::Type, std::function<void(const long long& clientID, const Message&)>>::iterator	action;
	while (true)
	{
		MessageClient	msgClient({-1, 0});
		if (_messageReceived.try_pop_front(msgClient) == false)
			break;

		action = _actions.find(msgClient.message.type());
		if (action != _actions.end())
		{
			try
			{
				action->second(msgClient.clientID, msgClient.message);
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
	}
}

void	Server::_acceptorLoop(void)
{
	_sockListeningFd[0].events = POLLIN;
	_sockListeningFd[0].revents = 0;
	_sockListeningFd[1].events = POLLIN;
	_sockListeningFd[1].revents = 0;

	int	p = 0;
	int res;
	while (_isRunning)
	{
		p = poll(_sockListeningFd, 2, 100);
		if (p > 0)
		{
			res = 0;
			if (_sockListeningFd[0].revents != 0)
			{
				if (_sockListeningFd[0].revents & POLLIN)
				{
					while (res == 0)
						res = _acceptClient(_sockListeningFd[0].fd);

					if (res != EAGAIN)
					{
						std::string err = "accept error IPv4: ";
						err += strerror(errno);
						threadSafeCerr << err << std::endl;
					}
				}
				else
				{
					std::string err = "Poll Error IPv4: ";
					err += strerror(errno);
					threadSafeCerr << err << std::endl;
				}
			}

			res = 0;
			if (_sockListeningFd[1].revents != 0)
			{
				if (_sockListeningFd[1].revents & POLLIN)
				{
					while (res == 0)
						res = _acceptClient(_sockListeningFd[1].fd);

					if (res != EAGAIN)
					{
						std::string err = "accept error IPv6: ";
						err += strerror(errno);
						threadSafeCerr << err << std::endl;
					}
				}
				else
				{
					std::string err = "Poll Error IPv6: ";
					err += strerror(errno);
					threadSafeCerr << err << std::endl;
				}
			}
		}
	}
}

void	Server::_readMessageHeader(const long long& id, ClientContext& context)
{
#ifdef DEBUG
	threadSafeCout << "[DEBUG] Server: Going to retrieve the message's header" << std::endl;
#endif
	if (context.bytesReaded == 0)
		context.buffer.resize(sizeof(context.header));

	ssize_t	toRead = sizeof(context.header) - context.bytesReaded;
	ssize_t	r = recv(context.sockFd, context.buffer.data() + context.bytesReaded, toRead, 0);
	if (r == 0)
	{
		_disconnectClient(id);
		return;
	}
	else if (r < 0)
	{
		if (errno != EAGAIN)
			_disconnectClient(id);
		return;
	}

	context.bytesReaded += r;
	if (r != toRead)
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		_clients[id] = context;
		return;	
	}

	std::memcpy(&context.header, context.buffer.data(), sizeof(context.header));
	std::memcpy(&context.header, context.buffer.data(), sizeof(context.header));

	if (context.header.size > MAX_MESSAGE_SIZE_MB * 1000000.0)
	{
		threadSafeCerr << "Server: Message corrupted (exceed MAX_MESSAGE_SIZE)\n";
		_disconnectClient(id);
		return;
	}
	context.headerReaded = true;
	context.buffer.resize(context.header.size);
	context.bytesReaded = 0;
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		_clients[id] = context;
	}
#ifdef DEBUG
	threadSafeCout << "[DEBUG] Server: Message's header received (" << context.header.size << " B)" << std::endl;
#endif
}

void	Server::_readMessageBody(const long long& id, ClientContext& context)
{
	ssize_t	toRead = context.header.size - context.bytesReaded;
	ssize_t	r = 0;
	if (toRead != 0)
	{
		r = recv(context.sockFd, context.buffer.data() + context.bytesReaded, toRead, 0);
		if (r == 0)
		{
			_disconnectClient(id);
			return;
		}
		else if (r < 0)
		{
			if (errno == EAGAIN)
				return;
#ifdef DEBUG
			std::string err = "[DEBUG] Server: recv error: ";
			err += strerror(errno);
			threadSafeCerr << err << std::endl;
#endif
			_disconnectClient(id);
			return;
		}
	}
	context.bytesReaded += r;
	if (r != toRead)
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		_clients[id] = context;
		return;
	}

	Message	response(context.header.type);
	response._setRawData(context.buffer.data(), context.buffer.size());
	_messageReceived.push_back({id, response});

	context.bytesReaded = 0;
	context.buffer.clear();
	context.headerReaded = false;
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		_clients[id] = context;
	}
}

void	Server::_readMessage(const long long& clientId)
{
#ifdef DEBUG
	threadSafeCout << "[DEBUG] Server: Message received from Client " << clientId << std::endl;
#endif
	ClientContext	context;
	{
		std::lock_guard<std::mutex>	lock(_mutexClientMap);
		if (_clients.find(clientId) == _clients.end())
			return;
		context = _clients[clientId];
	}

	if (context.headerReaded == false)
		_readMessageHeader(clientId, context);
	if (context.headerReaded == true)
		_readMessageBody(clientId, context);
}

void	Server::_receiverLoop(void)
{
	int					numberFdReady;
	struct epoll_event	event[MAX_EVENT];
	while (_isRunning)
	{
		{
			std::lock_guard<std::mutex>	lock(_mutexReceiverEpoll);
			numberFdReady = epoll_wait(_receiverEpollFd, event, MAX_EVENT, 100);
		}
		if (numberFdReady > 0)
		{
			for (int i = 0; i < numberFdReady; i++)
				_readMessage(event[i].data.u64);
		}
		else if (numberFdReady == -1)
		{
			std::string	err = "ServerReceiver: epoll_wait: ";
			err += strerror(errno);
			threadSafeCerr << err << std::endl;
			return;
		}
	}
}
