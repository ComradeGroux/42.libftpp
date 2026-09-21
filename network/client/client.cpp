#include "client.hpp"

#include "../network.hpp"
#include "../../io_stream/thread_safe_iostream.hpp"

#include <string>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <cstring>

Client::Client(void) : _sockFd(-1), _connected(false), _receiver("ClientReceiver", [this]() { _receiverLoop(); })
{}

Client::~Client(void)
{
	disconnect();
}

void	Client::connect(const std::string& address, const size_t& port)
{
	if (_connected)
		throw AlreadyConnectedException();

	addrinfo	hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	addrinfo	*res;
	int	gai_errcode = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &res);
	if (gai_errcode != 0)
	{
		std::string	error = "Client: ";
		error += gai_strerror(gai_errcode);
		throw std::runtime_error(error);
	}

	addrinfo* curr = res;
	while (curr)
	{
		_sockFd = socket(curr->ai_family, SOCK_STREAM, curr->ai_protocol);
		if (_sockFd == -1)
		{
			freeaddrinfo(res);
			std::string	error = "Client: ";
			error += strerror(errno);
			throw std::runtime_error(error);
		}

		if (::connect(_sockFd, reinterpret_cast<sockaddr*>(curr->ai_addr), curr->ai_addrlen) == 0)
			break;
		
		close(_sockFd);
		_sockFd = -1;
		curr = curr->ai_next;
	}
	freeaddrinfo(res);

	if (_sockFd == -1)
		throw std::runtime_error("Client: Unable to open a connection with the server");

#ifdef DEBUG
threadSafeCout << "[DEBUG] Client: Connection established" << std::endl;
#endif
	_connected = true;
	_receiver.start();
}

void	Client::disconnect(void)
{
	if (_sockFd == -1)
		return;
#ifdef DEBUG
threadSafeCout << "[DEBUG]: Client: Going to disconnect" << std::endl;
#endif
	shutdown(_sockFd, SHUT_RDWR);
	_receiver.stop();
	close(_sockFd);
	_sockFd = -1;
	_connected = false;
#ifdef DEBUG
threadSafeCout << "[DEBUG]: Client: Disconnected" << std::endl;
#endif
}

bool	Client::isConnected(void) const noexcept
{
	return _connected;
}

void	Client::_receiverLoop(void)
{
	std::vector<uint8_t>	buff;
	size_t					bytesReaded = 0;
	ssize_t					r = 0;
	while (_connected)
	{
		bytesReaded = 0;
		buff.resize(sizeof(Message::Header));
		while (bytesReaded < sizeof(Message::Header))
		{
			r = recv(_sockFd, buff.data() + bytesReaded, buff.size() - bytesReaded, 0);
			if (r <= 0)
				break;
			bytesReaded += r;
		}
		if (r <= 0)
			break;

		Message::Header hdr;
		std::memcpy(&hdr, buff.data(), sizeof(hdr));

		if (hdr.size > MAX_MESSAGE_SIZE_MB * 1000000)
		{
			threadSafeCerr << "Client: Message corrupted (exceed MAX_MESSAGE_SIZE)\n";
			break;
		}

		buff.resize(hdr.size);
		bytesReaded = 0;
		while (bytesReaded < hdr.size)
		{
			r = recv(_sockFd, buff.data() + bytesReaded, buff.size() - bytesReaded, 0);
			if (r <= 0)
				break;
			bytesReaded += r;
		}
		if (r <= 0)
			break;

		Message	response(hdr.type);
		response._setRawData(buff.data(), buff.size());
		_messageReceived.push_back(response);
	}
	close(_sockFd);
	_connected = false;
}

void	Client::defineAction(const Message::Type& messageType, const std::function<void(const Message&)>& action)
{
	_actions[messageType] = action;
}

void	Client::send(const Message& message)
{
	if (!_connected)
	{
#ifdef DEBUG
		threadSafeCerr << "Client not connected" << std::endl;
#endif
		return;
	}

	Message::Header	hdr;
	hdr.size = message._rawSize();
	hdr.type = message.type();

	if (hdr.size > MAX_MESSAGE_SIZE_MB * 1000000)
	{
		threadSafeCerr << "Client: Message exceed max size (" << hdr.size / 1000000.0 << " MB / " << MAX_MESSAGE_SIZE_MB << " MB)\n";
		return;
	}

	std::vector<uint8_t>	buff;
	buff.resize(sizeof(hdr));
	std::memcpy(buff.data(), &hdr, sizeof(hdr));
	buff.insert(buff.end(), message._rawData(), message._rawData() + message._rawSize());

	size_t	bytesSended = 0;
	while (bytesSended < buff.size())
	{
		ssize_t	s = ::send(_sockFd, buff.data() + bytesSended, buff.size() - bytesSended, 0);
		if (s <= 0)
		{
			threadSafeCerr << "Client: Error with the server connexion\n";
			disconnect();
			return;
		}
		bytesSended += s;
	}
}

void	Client::update(void)
{
	std::unordered_map<Message::Type, std::function<void(const Message&)>>::iterator	action;
	Message	msg(-1);
	while (true)
	{
		if (_messageReceived.try_pop_front(msg) == false)
			break;

		action = _actions.find(msg.type());
		if (action != _actions.end())
		{
			try
			{
				action->second(msg);
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
	}
}
