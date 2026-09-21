/*
* ============================================================================
*  test_network_main.cpp — Suite de tests pour le module Network de libftpp
* ============================================================================
*
*  HYPOTHÈSES SUR DataBuffer :
*  ----------------------------
*  - Le support de `int` via memcpy brut est supposé garanti (c'est le cas
*    le plus simple/standard) : la plupart des tests reposent uniquement
*    sur des `int`, pour ne dépendre d'aucune fonctionnalité optionnelle.
*  - Le test `test_server_to_client_sendTo()` utilise `std::string` en
*    payload : si ton DataBuffer ne supporte pas encore std::string,
*    remplace ce payload par un `int` ou un tableau de char fixe.
*
*  COMPILATION (à adapter selon ton organisation de projet) :
*  ------------------------------------------------------------
*    c++ -std=c++11 -Wall -Wextra -pthread test_network_main.cpp \
*        -L. -lftpp -o test_network_main
*    ./test_network_main
*
*  Adapte les chemins d'include ci-dessous à la structure réelle de ton
*  repo si `libftpp.hpp` n'est pas directement accessible depuis ce fichier.
* ============================================================================
*/

#include "../../network/network.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <functional>
#include <stdexcept>

static int g_passCount = 0;
static int g_failCount = 0;

#define CHECK(cond, label)                                  \
	do {                                                    \
		if (cond) {                                         \
			g_passCount++;                                  \
			std::cout << "  [PASS] " << label << std::endl; \
		} else {                                            \
			g_failCount++;                                  \
			std::cerr << "  [FAIL] " << label << std::endl; \
		}                                                   \
	} while (0)

static size_t g_portCounter = 50000;
static size_t nextPort(void)
{
	return g_portCounter++;
}

template <typename PumpFn, typename PredFn>
static bool waitUntil(PumpFn pump, PredFn pred, int timeoutMs = 3000)
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	while (true)
	{
		pump();
		if (pred())
			return true;
		long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start).count();
		if (elapsed >= timeoutMs)
			return pred();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}

// Types de messages utilisés par les tests.
enum MsgType
{
	MSG_INT = 1,
	MSG_STRING,
	MSG_PING,
	MSG_BROADCAST,
	MSG_ECHO_REPLY,
	MSG_UNHANDLED,   // volontairement jamais enregistré via defineAction
	MSG_BIG,
};

// ============================================================================
//  Tests
// ============================================================================

void test_server_lifecycle(void)
{
	std::cout << "\n=== test_server_lifecycle ===" << std::endl;

	Server server;

	bool threwBeforeStart = false;
	try
	{
		server.sendTo(Message(MSG_PING), 0);
	}
	catch (const Server::ServerNotStartedException&)
	{
		threwBeforeStart = true;
	}
	CHECK(threwBeforeStart, "sendTo() avant start() lance ServerNotStartedException");

	server.start(nextPort());

	bool threwDoubleStart = false;
	try
	{
		server.start(nextPort());
	}
	catch (const Server::AlreadyStartedException&)
	{
		threwDoubleStart = true;
	}
	CHECK(threwDoubleStart, "start() appelé deux fois lance AlreadyStartedException");
}

void test_client_lifecycle(void)
{
	std::cout << "\n=== test_client_lifecycle ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	Client client;
	CHECK(!client.isConnected(), "Client non connecté avant connect()");

	client.connect("127.0.0.1", port);
	CHECK(client.isConnected(), "Client connecté après connect()");

	bool threwDoubleConnect = false;
	try
	{
		client.connect("127.0.0.1", port);
	}
	catch (const Client::AlreadyConnectedException&)
	{
		threwDoubleConnect = true;
	}
	CHECK(threwDoubleConnect, "connect() appelé deux fois lance AlreadyConnectedException");

	client.disconnect();
	CHECK(!client.isConnected(), "Client déconnecté après disconnect()");

	client.disconnect(); // doit être un no-op sûr
	CHECK(true, "un double disconnect() ne plante pas");
}

void test_client_to_server_roundtrip(void)
{
	std::cout << "\n=== test_client_to_server_roundtrip ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	bool received = false;
	int receivedValue = 0;
	long long capturedClientId = -1;

	server.defineAction(MSG_INT, [&](const long long& clientID, const Message& msg)
	{
		Message copy = msg;
		int value = 0;
		copy >> value;
		receivedValue = value;
		capturedClientId = clientID;
		received = true;
	});

	Client client;
	client.connect("127.0.0.1", port);

	Message msg(MSG_INT);
	msg << 42;
	client.send(msg);

	bool ok = waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return received; }
	);

	CHECK(ok, "Le serveur reçoit bien le message envoyé par le client");
	CHECK(receivedValue == 42, "La valeur transmise est correcte (42)");
	CHECK(capturedClientId != -1, "Un ID client valide a été assigné par le serveur");

	client.disconnect();
}

void test_server_to_client_sendTo(void)
{
	std::cout << "\n=== test_server_to_client_sendTo ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	long long capturedClientId = -1;
	server.defineAction(MSG_PING, [&](const long long& clientID, const Message&)
	{
		capturedClientId = clientID;
	});

	Client client;
	bool clientReceived = false;
	std::string clientReceivedText;
	client.defineAction(MSG_ECHO_REPLY, [&](const Message& msg)
	{
		Message copy = msg;
		std::string text;
		copy >> text;
		clientReceivedText = text;
		clientReceived = true;
	});

	client.connect("127.0.0.1", port);
	client.send(Message(MSG_PING));

	waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return capturedClientId != -1; }
	);
	CHECK(capturedClientId != -1, "Le serveur a bien capturé l'ID du client");

	Message reply(MSG_ECHO_REPLY);
	reply << std::string("pong");
	server.sendTo(reply, capturedClientId);

	bool ok = waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return clientReceived; }
	);
	CHECK(ok, "Le client reçoit bien le message envoyé via Server::sendTo()");
	CHECK(clientReceivedText == "pong", "Le contenu du message reçu est correct");

	client.disconnect();
}

void test_broadcast(void)
{
	std::cout << "\n=== test_broadcast (sendToAll / sendToArray) ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	const int NB_CLIENTS = 4;

	// clientID (attribué par le serveur, imprévisible) -> index local du client
	std::map<long long, int> idToIndex;
	server.defineAction(MSG_PING, [&](const long long& clientID, const Message& msg)
	{
		Message copy = msg;
		int idx = -1;
		copy >> idx;
		idToIndex[clientID] = idx;
	});

	std::vector<std::unique_ptr<Client> > clients;
	std::vector<int> receivedCount(NB_CLIENTS, 0);

	for (int i = 0; i < NB_CLIENTS; i++)
	{
		clients.emplace_back(new Client());
		clients[i]->connect("127.0.0.1", port);
		clients[i]->defineAction(MSG_BROADCAST, [&receivedCount, i](const Message&)
		{
			receivedCount[i]++;
		});

		Message hello(MSG_PING);
		hello << i; // le client s'annonce avec son index local
		clients[i]->send(hello);
	}

	auto pump = [&]()
	{
		server.update();
		for (size_t i = 0; i < clients.size(); i++)
			clients[i]->update();
	};

	waitUntil(pump, [&]() { return (int)idToIndex.size() == NB_CLIENTS; });
	CHECK((int)idToIndex.size() == NB_CLIENTS, "Tous les clients se sont annoncés auprès du serveur");

	// --- sendToAll : tout le monde doit recevoir exactement 1 message ---
	server.sendToAll(Message(MSG_BROADCAST));
	waitUntil(pump, [&]()
	{
		for (int i = 0; i < NB_CLIENTS; i++)
			if (receivedCount[i] < 1)
				return false;
		return true;
	});

	bool allReceivedOnce = true;
	for (int i = 0; i < NB_CLIENTS; i++)
		if (receivedCount[i] != 1)
			allReceivedOnce = false;
	CHECK(allReceivedOnce, "sendToAll() livre le message à tous les clients connectés, une seule fois");

	// --- sendToArray : uniquement les clients d'index local 0 et 1 ---
	std::vector<long long> subset;
	for (std::map<long long, int>::iterator it = idToIndex.begin(); it != idToIndex.end(); ++it)
		if (it->second == 0 || it->second == 1)
			subset.push_back(it->first);

	server.sendToArray(Message(MSG_BROADCAST), subset);
	waitUntil(pump, [&]() { return receivedCount[0] == 2 && receivedCount[1] == 2; }, 1500);

	CHECK(receivedCount[0] == 2 && receivedCount[1] == 2, "sendToArray() livre bien aux IDs ciblés");
	CHECK(receivedCount[2] == 1 && receivedCount[3] == 1, "sendToArray() ne livre pas aux IDs non ciblés");

	for (size_t i = 0; i < clients.size(); i++)
		clients[i]->disconnect();
}

void test_ordering(void)
{
	std::cout << "\n=== test_ordering ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	std::vector<int> receivedOrder;
	server.defineAction(MSG_INT, [&](const long long&, const Message& msg)
	{
		Message copy = msg;
		int value = 0;
		copy >> value;
		receivedOrder.push_back(value);
	});

	Client client;
	client.connect("127.0.0.1", port);

	const int N = 200;
	for (int i = 0; i < N; i++)
	{
		Message msg(MSG_INT);
		msg << i;
		client.send(msg);
	}

	waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return (int)receivedOrder.size() == N; },
		5000
	);

	bool sizeOk = (int)receivedOrder.size() == N;
	CHECK(sizeOk, "Tous les messages envoyés sont reçus (aucune perte)");

	bool orderOk = sizeOk;
	for (int i = 0; orderOk && i < N; i++)
		if (receivedOrder[i] != i)
			orderOk = false;
	CHECK(orderOk, "Les messages sont reçus dans l'ordre exact d'envoi");

	client.disconnect();
}

void test_boundary_and_oversized_message(void)
{
	std::cout << "\n=== test_boundary_and_oversized_message ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	bool bigMessageReceived = false;
	bool markerReceived = false;

	server.defineAction(MSG_BIG, [&](const long long&, const Message&)
	{
		bigMessageReceived = true;
	});
	server.defineAction(MSG_PING, [&](const long long&, const Message&)
	{
		markerReceived = true;
	});

	Client client;
	client.connect("127.0.0.1", port);

	// --- Message juste sous la limite : doit passer ---
	{
		Message big(MSG_BIG);
		size_t targetBytes = (size_t)MAX_MESSAGE_SIZE_MB * 1000000 - 5000; // marge pour le header
		size_t numInts = targetBytes / sizeof(int);
		for (size_t i = 0; i < numInts; i++)
			big << (int)i;
		client.send(big);
	}
	bool ok = waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return bigMessageReceived; },
		8000
	);
	CHECK(ok, "Un message juste en dessous de MAX_MESSAGE_SIZE_MB est transmis avec succès");

	// --- Message dépassant la limite : rejeté par send(), connexion doit rester utilisable ---
	bigMessageReceived = false;
	{
		Message tooBig(MSG_BIG);
		size_t targetBytes = (size_t)MAX_MESSAGE_SIZE_MB * 1000000 + 500000; // dépasse nettement
		size_t numInts = targetBytes / sizeof(int);
		for (size_t i = 0; i < numInts; i++)
			tooBig << (int)i;
		client.send(tooBig); // doit juste logger une erreur et ne rien envoyer
	}
	client.send(Message(MSG_PING)); // message marqueur envoyé juste après

	bool markerOk = waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return markerReceived; },
		3000
	);
	CHECK(markerOk, "Après un message trop volumineux rejeté, la connexion reste utilisable");
	CHECK(!bigMessageReceived, "Le message trop volumineux n'a jamais été reçu par le serveur");

	client.disconnect();
}

void test_unregistered_message_type(void)
{
	std::cout << "\n=== test_unregistered_message_type ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	bool wrongActionFired = false;
	server.defineAction(MSG_PING, [&](const long long&, const Message&)
	{
		wrongActionFired = true;
	});
	// MSG_UNHANDLED est volontairement non enregistré

	bool markerReceived = false;
	server.defineAction(MSG_INT, [&](const long long&, const Message&)
	{
		markerReceived = true;
	});

	Client client;
	client.connect("127.0.0.1", port);

	client.send(Message(MSG_UNHANDLED));
	client.send(Message(MSG_INT));

	waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return markerReceived; }
	);

	CHECK(markerReceived, "Le message suivant un type non enregistré est bien traité normalement");
	CHECK(!wrongActionFired, "Un type de message non enregistré ne déclenche aucune action erronée");

	client.disconnect();
}

void test_exception_in_callback_does_not_break_update(void)
{
	std::cout << "\n=== test_exception_in_callback_does_not_break_update ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	server.defineAction(MSG_UNHANDLED, [&](const long long&, const Message&)
	{
		throw std::runtime_error("boom (volontaire, pour tester la robustesse de update())");
	});

	bool markerReceived = false;
	server.defineAction(MSG_INT, [&](const long long&, const Message&)
	{
		markerReceived = true;
	});

	Client client;
	client.connect("127.0.0.1", port);

	client.send(Message(MSG_UNHANDLED)); // provoque une exception dans le callback serveur
	client.send(Message(MSG_INT));       // doit quand même être traité normalement après

	bool ok = waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return markerReceived; }
	);
	CHECK(ok, "update() continue de fonctionner normalement après qu'un callback ait levé une exception");

	client.disconnect();
}

void test_reconnect(void)
{
	std::cout << "\n=== test_reconnect ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	std::vector<long long> seenIds;
	server.defineAction(MSG_PING, [&](const long long& clientID, const Message&)
	{
		seenIds.push_back(clientID);
	});

	Client client;

	client.connect("127.0.0.1", port);
	client.send(Message(MSG_PING));
	waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return seenIds.size() == 1; }
	);
	client.disconnect();

	client.connect("127.0.0.1", port); // reconnexion sur le même objet Client
	client.send(Message(MSG_PING));
	waitUntil(
		[&]() { client.update(); server.update(); },
		[&]() { return seenIds.size() == 2; }
	);
	client.disconnect();

	CHECK(seenIds.size() == 2, "Deux connexions successives sont bien vues par le serveur");
	CHECK(seenIds.size() < 2 || seenIds[0] != seenIds[1], "Chaque reconnexion obtient un nouvel ID client distinct");
}

void test_multi_client_stress(void)
{
	std::cout << "\n=== test_multi_client_stress ===" << std::endl;

	Server server;
	size_t port = nextPort();
	server.start(port);

	const int NB_CLIENTS = 10;
	const int MSG_PER_CLIENT = 50;

	int totalReceived = 0;
	server.defineAction(MSG_INT, [&](const long long&, const Message&)
	{
		totalReceived++;
	});

	std::vector<std::unique_ptr<Client> > clients;
	for (int i = 0; i < NB_CLIENTS; i++)
	{
		clients.emplace_back(new Client());
		clients[i]->connect("127.0.0.1", port);
	}

	for (int i = 0; i < NB_CLIENTS; i++)
	{
		for (int j = 0; j < MSG_PER_CLIENT; j++)
		{
			Message msg(MSG_INT);
			msg << j;
			clients[i]->send(msg);
		}
	}

	auto pump = [&]()
	{
		server.update();
		for (size_t i = 0; i < clients.size(); i++)
			clients[i]->update();
	};

	bool ok = waitUntil(pump, [&]()
	{
		return totalReceived == NB_CLIENTS * MSG_PER_CLIENT;
	}, 10000);

	CHECK(ok, "Tous les messages de tous les clients sont reçus (aucune perte sous charge modérée)");
	CHECK(totalReceived == NB_CLIENTS * MSG_PER_CLIENT, "Le compte total de messages reçus est exact");

	for (size_t i = 0; i < clients.size(); i++)
		clients[i]->disconnect();
}

int main(void)
{
	test_server_lifecycle();
	test_client_lifecycle();
	test_client_to_server_roundtrip();
	test_server_to_client_sendTo();
	test_broadcast();
	test_ordering();
	test_boundary_and_oversized_message();
	test_unregistered_message_type();
	test_exception_in_callback_does_not_break_update();
	test_reconnect();
	test_multi_client_stress();

	std::cout << "\n============================================" << std::endl;
	std::cout << g_passCount << " tests passes, " << g_failCount << " tests echoues" << std::endl;
	std::cout << "============================================" << std::endl;

	return g_failCount == 0 ? 0 : 1;
}
