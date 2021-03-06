#pragma once
#include <thread>
#include <memory/ToiStackAllocator.h>
#include "../Subsystem.h"

#define THREAD1_PATTERN 0x0F0F
#define THREAD2_PATTERN 0xFFFF
#define THREAD3_PATTERN 0x0000
#define THREAD4_PATTERN 0xF0F0
#define MEBI 1024 * 1024 
void WriteTest(int pattern, int* memory, int numthreads);
static std::atomic<int> counter;
class SSThreadTest : public Subsystem {
public:
	SSThreadTest(int ID) : Subsystem(Name, ID) {
		SSThreadTest::ID = ID;
	}
	~SSThreadTest() = default;

	void 		Startup(SubsystemCollection* const subsystemCollection) override;
	void 		Shutdown(SubsystemCollection* const subsystemCollection) override;
	void 		UpdateUserLayer(const float deltaTime) override;
	void 		UpdateSimulationLayer(const float timeStep) override;

	Subsystem* 	CreateNew() const override;

	static int GetStaticID();

	const static pString Name;
private:
	static int ID;
    ToiStackAllocator* m_Allocator = nullptr;

	std::thread m_Threads[8];
	unsigned int m_Patterns[4];
};
