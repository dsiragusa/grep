#include "UniqueIdGenerator.h"

UniqueIdGenerator::UniqueIdGenerator(int baseId) {
	id = baseId;
}

UniqueIdGenerator::~UniqueIdGenerator() {
	// TODO Auto-generated destructor stub
}

int UniqueIdGenerator::getUniqueId() {
	return id++;
}
