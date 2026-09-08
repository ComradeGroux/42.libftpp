#include "design_pattern/singleton/singleton.hpp"

#include <iostream>

class MyClass : public Singleton<MyClass>
{
	private:
		friend class Singleton<MyClass>;

		MyClass(int value)
		{
			std::cout << "MyClass constructor, with value [" << value << "]" << std::endl;
		}

	public:
		void printMessage()
		{
			std::cout << "Hello from MyClass" << std::endl;
		}
};

int main()
{
	try
	{
		MyClass::instance();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	MyClass::instantiate(42);

	MyClass::instance()->printMessage();

	try
	{
		MyClass::instantiate(100);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}
