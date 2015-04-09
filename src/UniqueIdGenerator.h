#ifndef SRC_UNIQUEIDGENERATOR_H_
#define SRC_UNIQUEIDGENERATOR_H_

class UniqueIdGenerator {
public:
	UniqueIdGenerator(int baseId);
	virtual ~UniqueIdGenerator();
	int getUniqueId();

private:
	int id;
};

#endif /* SRC_UNIQUEIDGENERATOR_H_ */
