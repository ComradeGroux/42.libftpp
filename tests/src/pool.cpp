#include <iostream>
#include <string>
#include <cassert>

#include "data_structures/pool/pool.hpp"

class TestObject
{
	public:
		static int	liveCount;
		static int	ctorCalls;
		static int	dtorCalls;

		std::string	name;
		int			value;

		TestObject(const std::string& p_name, int p_value) : name(p_name), value(p_value)
		{
			liveCount++;
			ctorCalls++;
			std::cout << "  [ctor] " << name << " (value=" << value << ") at " << this << "\n";
		}

		~TestObject()
		{
			liveCount--;
			dtorCalls++;
			std::cout << "  [dtor] " << name << " at " << this << "\n";
		}

		void	greet() const
		{
			std::cout << "  Hello from " << name << " (value=" << value << ")\n";
		}
};

int	TestObject::liveCount = 0;
int	TestObject::ctorCalls = 0;
int	TestObject::dtorCalls = 0;

static void	print_section(const std::string& title)
{
	std::cout << "\n=== " << title << " ===\n";
}

int	main()
{
	{
		print_section("1. resize() + acquire() basique");
		Pool<TestObject>	pool;
		pool.resize(3);

		auto	o1 = pool.acquire("Alice", 1);
		auto	o2 = pool.acquire("Bob", 2);
		auto	o3 = pool.acquire("Charlie", 3);

		o1->greet();
		o2->greet();
		o3->greet();

		assert(TestObject::liveCount == 3);
		std::cout << "OK: 3 objets vivants comme attendu\n";

		print_section("2. Pool epuise -> exception attendue");
		try
		{
			auto	o4 = pool.acquire("Dave", 4);
			(void)o4;
			std::cout << "FAIL: aucune exception levee !\n";
		}
		catch (const Pool<TestObject>::NoAvailableObjectsException& e)
		{
			std::cout << "OK: exception attrapee : " << e.what() << "\n";
		}
	}

	print_section("3. Verification post-scope");
	std::cout << "liveCount = " << TestObject::liveCount
			<< " (attendu: 0)\n";
	std::cout << "ctorCalls = " << TestObject::ctorCalls
			<< ", dtorCalls = " << TestObject::dtorCalls
			<< " (doivent etre egaux)\n";
	assert(TestObject::liveCount == 0);
	assert(TestObject::ctorCalls == TestObject::dtorCalls);

	{
		print_section("4. Reutilisation d'un slot relache");
		Pool<TestObject>	pool;
		pool.resize(1);

		int	ctorBefore = TestObject::ctorCalls;
		{
			auto	tmp = pool.acquire("Temp1", 100);
			tmp->greet();
		}

		auto	tmp2 = pool.acquire("Temp2", 200);
		tmp2->greet();

		std::cout << "Constructions realisees : " << (TestObject::ctorCalls - ctorBefore)
				<< " (attendu: 2, une par acquire)\n";
		assert(TestObject::ctorCalls - ctorBefore == 2);
		std::cout << "OK: le slot a bien ete reutilise\n";

		print_section("5. shrinkPool via resize() avec un objet encore acquis");
		try
		{
			pool.resize(0);
			std::cout << "FAIL: aucune exception levee alors qu'un objet est encore acquis\n";
		}
		catch (const Pool<TestObject>::NoAvailableObjectsToRemoveException& e)
		{
			std::cout << "OK: exception attendue, le slot occupe par tmp2 ne peut pas etre libere : "
					<< e.what() << "\n";
		}
		tmp2->greet();
	}

	print_section("6. Verification finale globale");
	std::cout << "liveCount = " << TestObject::liveCount
			<< " (attendu: 0)\n";
	std::cout << "ctorCalls = " << TestObject::ctorCalls
			<< ", dtorCalls = " << TestObject::dtorCalls
			<< " (doivent etre egaux : pas de fuite, pas de double destruction)\n";
	assert(TestObject::liveCount == 0);
	assert(TestObject::ctorCalls == TestObject::dtorCalls);

	std::cout << "\nTous les tests sont passes.\n";
	return 0;
}