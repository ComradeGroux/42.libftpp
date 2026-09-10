#include "design_pattern/memento/memento.hpp"

#include <cassert>
#include <iostream>
#include <string>

static int g_testCount = 0;
static int g_failCount = 0;

#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)

// Classe "saveable" de test : un personnage de jeu simple
class Character : public Memento
{
	public:
		int hp;
		int level;
		std::string name;

		Character(int hp_ = 100, int level_ = 1, std::string name_ = "Hero")
			: hp(hp_), level(level_), name(std::move(name_)) {}

		// Le sujet suggere que Memento a besoin d'un acces "friendly" a ces methodes
		friend class Memento;

	private:
		void	_saveToSnapshot(Memento::Snapshot& snapshot) const override
		{
			snapshot << hp << level << name;
		}

		void	_loadFromSnapshot(Memento::Snapshot& snapshot) override
		{
			snapshot >> hp >> level >> name;
		}
};

static void	testSaveLoadRestoresState(void)
{
	std::cout << "-- testSaveLoadRestoresState --\n";

	Character c(100, 1, "Hero");
	Memento::Snapshot saved = c.save();

	// On modifie l'etat de l'objet
	c.hp = 10;
	c.level = 5;
	c.name = "Damaged Hero";

	// On restaure l'etat initial
	c.load(saved);

	CHECK(c.hp == 100);
	CHECK(c.level == 1);
	CHECK(c.name == "Hero");
}

static void	testMultipleSnapshotsAreIndependent(void)
{
	std::cout << "-- testMultipleSnapshotsAreIndependent --\n";

	Character c(50, 2, "Alice");
	Memento::Snapshot snap1 = c.save();

	c.hp = 30;
	c.level = 3;
	c.name = "Alice the Brave";
	Memento::Snapshot snap2 = c.save();

	c.hp = 0;
	c.level = 99;
	c.name = "Dead";

	c.load(snap2);
	CHECK(c.hp == 30);
	CHECK(c.level == 3);
	CHECK(c.name == "Alice the Brave");

	c.load(snap1);
	CHECK(c.hp == 50);
	CHECK(c.level == 2);
	CHECK(c.name == "Alice");
}

static void	testSnapshotAppliedToDifferentInstance(void)
{
	std::cout << "-- testSnapshotAppliedToDifferentInstance --\n";

	Character original(75, 4, "Bob");
	Memento::Snapshot saved = original.save();

	Character other(1, 1, "Empty");
	other.load(saved);

	CHECK(other.hp == 75);
	CHECK(other.level == 4);
	CHECK(other.name == "Bob");
}

int	main(void)
{
	testSaveLoadRestoresState();
	testMultipleSnapshotsAreIndependent();
	testSnapshotAppliedToDifferentInstance();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
