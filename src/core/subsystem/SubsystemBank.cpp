#include "SubsystemBank.h"
#include <cassert>
#include "graphics/SSWindow.h"
#include "graphics/SSGraphicsSwap.h"
#include "graphics/SSGraphics.h"
#include "graphics/SSParticles.h"
#include "graphics/SSVisualizer.h"
#include "profile/SSProfilerInOut.h"
#include "resourcing/SSResourceManager.h"
#include "testing/SSDeranesPoolTest.h"
#include "testing/SSStackAllocatorBasicTest.h"
#include "testing/SSSimplePoolTest.h"
#include "testing/SSThreadTest.h"
#include "testing/SSPoolThreadingTest.h"
#include "testing/SSStackTest.h"
#include "testing/SSResourcingTest.h"

SubsystemBank& SubsystemBank::GetInstance() {
	static SubsystemBank subsystemBank;
	return subsystemBank;
}

void SubsystemBank::Initialize() {
	CreateSubsystemTemplate<SSDeranesPoolTest>();
	CreateSubsystemTemplate<SSGraphicsSwap>();
	CreateSubsystemTemplate<SSGraphics>();
	//CreateSubsystemTemplate<SSParticles>();
	CreateSubsystemTemplate<SSResourceManager>();
	CreateSubsystemTemplate<SSPoolThreadingTest>();
	CreateSubsystemTemplate<SSProfilerInOut>();
	//CreateSubsystemTemplate<SSStackTest>();
	CreateSubsystemTemplate<SSResourcingTest>();
	CreateSubsystemTemplate<SSStackAllocatorBasicTest>();
	CreateSubsystemTemplate<SSSimplePoolTest>();
	CreateSubsystemTemplate<SSWindow>();
	//CreateSubsystemTemplate<SSVisualizer>();
	//CreateSubsystemTemplate<SSThreadTest>();
	// Startup priorities
	auto setStartPrio = [this] ( int id, int prio ) {
		m_SubsystemTemplates.at( id )->SetStartOrderPriority( prio );
	};
	setStartPrio( SSWindow::GetStaticID(),          	-500 ); // Before Input, graphics etc.
	setStartPrio(SSGraphics::GetStaticID(), 	-400 ); //                                    | After window

	// Update priorities
	auto setUpdatePrio = [this] ( int id, int prio ) {
		m_SubsystemTemplates.at( id )->SetUpdateOrderPriority( prio );
	};
	// Defaulted
	setUpdatePrio( SSDeranesPoolTest::GetStaticID(), 0 );
	setUpdatePrio(SSWindow::GetStaticID(), 0);
	setUpdatePrio(SSGraphics::GetStaticID(), 0 ); //after window
	//setUpdatePrio( SSParticles::GetStaticID(), 0);
	//setUpdatePrio( SSStackTest::GetStaticID(), 0);
	setUpdatePrio( SSStackAllocatorBasicTest::GetStaticID(), 0 );
	setUpdatePrio( SSSimplePoolTest::GetStaticID(), 0 );
	setUpdatePrio( SSPoolThreadingTest::GetStaticID(), 0 );
	//setUpdatePrio( SSThreadTest::GetStaticID(), 0);
	setUpdatePrio( SSResourcingTest::GetStaticID(), 0);
	//setUpdatePrio( SSVisualizer::GetStaticID(), 10);		  // After Particles					|
	setUpdatePrio( SSProfilerInOut::GetStaticID(), 250 ); // After profiling 					| before graphics swap
	setUpdatePrio( SSGraphicsSwap::GetStaticID(), 500 );  // Before frame reset stuff 			| After all rendering

	// Shutdown priorities
	auto setShutdownPrio = [this] ( int id, int prio ) {
		m_SubsystemTemplates.at( id )->SetShutdownOrderPriority( prio );
	};
	setShutdownPrio( SSResourceManager::GetStaticID(), 800 ); 	// Before graphics context destruction 	|
	setShutdownPrio( SSWindow::GetStaticID(), 900 ); 			//									 	| After context destruction
}

void SubsystemBank::Deinitialize() {
	for ( auto& subsystem : m_SubsystemTemplates ) {
		pDelete( subsystem );
	}
}

Subsystem* SubsystemBank::CreateSubsystem( int subsystemID ) {
	assert( subsystemID >= 0 && subsystemID < m_SubsystemTemplates.size() );
	return m_SubsystemTemplates.at( subsystemID )->Clone();
}

size_t SubsystemBank::GetNrOfSubsystems() const {
	return m_SubsystemTemplates.size();
}
