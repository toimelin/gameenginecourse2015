#pragma once

//#define TOI_TEMPLATED_POOL_ALLOCATOR_SET_ALLOCATED_MEMORY
#define TOI_TEMPLATED_POOL_ALLOCATOR_SET_ALLOCATED_MEMORY_VALUE 0

//#define TOI_TEMPLATED_POOL_ALLOCATOR_SET_FREED_MEMORY
#define TOI_TEMPLATED_POOL_ALLOCATOR_SET_FREED_MEMORY_VALUE 0

//#define TOI_TEMPLATED_POOL_ALLOCATOR_COUT_INFO

#include "MemoryLibraryDefine.h"
#include <cstddef>
#include <stdint.h>
#include <cassert>
#include <iostream>
#include <iomanip>
#ifdef TOI_TEMPLATED_POOL_ALLOCATOR_MUTEX_LOCK
	#include <mutex>
#endif
#ifdef TOI_TEMPLATED_POOL_ALLOCATOR_SPIN_LOCK
	#include <atomic>
#endif
#include <thread>

#define TOI_TEMPLATED_POOL_ALLOCATOR_NR_OF_BLOCKS 256000
#define TOI_TEMPLATED_POOL_ALLOCATOR_MAX_BLOCK_SIZE 512

//#define TOI_TEMPLATED_POOL_USE_STANDARD_ALLOCATOR

#if defined(TOI_TEMPLATED_POOL_USE_STANDARD_ALLOCATOR)
	#define poolThreadAlloc(Size) malloc(Size)
	#define poolThreadFree(Size, Ptr) free(Ptr)
	#define poolThreadNew(Type, ...) new Type(__VA_ARGS__)
	#define poolThreadDelete(Ptr) delete Ptr;
#else
	#define poolThreadAlloc(Size) GetLockableThreadPoolAllocator<Size>().allocate()
	#define poolThreadFree(Size, Ptr) GetLockableThreadPoolAllocator<Size>().deallocate(Ptr)
	#define poolThreadNew(Type, ...) GetLockableThreadPoolAllocator<sizeof(Type)>().construct<Type>(__VA_ARGS__)
	#define poolThreadDelete(Ptr) GetLockableThreadPoolAllocator<sizeof(*Ptr)>().destroy(Ptr)
#endif

template <size_t BlockSize, size_t NrOfBlocks>
class ToiTemplatedPoolAllocator {
public:
	ToiTemplatedPoolAllocator( ) {
		assert( BlockSize >= sizeof(void*) );
        assert( BlockSize <= TOI_TEMPLATED_POOL_ALLOCATOR_MAX_BLOCK_SIZE );
#ifdef TOI_TEMPLATED_POOL_ALLOCATOR_COUT_INFO
		std::cout << "Creating templated pool allocator for blocksize " << BlockSize << std::endl;
#endif

		m_Memory = static_cast<uint8_t*>( operator new( BlockSize * NrOfBlocks ) );

		for ( size_t i = 0; i < NrOfBlocks * BlockSize; i+= BlockSize ) {
			size_t* nextFree = reinterpret_cast<size_t*>( m_Memory + i );
			*nextFree = reinterpret_cast<size_t>( m_Memory + i + BlockSize );
		}
		
		size_t* last = reinterpret_cast<size_t*>( m_Memory + ( NrOfBlocks - 1) * BlockSize );
		*last = reinterpret_cast<size_t>( nullptr );

		m_FirstFree = reinterpret_cast<size_t*>( m_Memory );
	}

	~ToiTemplatedPoolAllocator() {
#ifdef TOI_TEMPLATED_POOL_ALLOCATOR_COUT_INFO
		std::cout << "Destroying templated pool allocator for blocksize " << BlockSize << std::endl;
#endif
		free( m_Memory );
	}

	void* allocate( ) {
		assert( m_FirstFree != nullptr );
		size_t* toAllocate = m_FirstFree;
		m_FirstFree = reinterpret_cast<size_t*>( *reinterpret_cast<size_t*>( m_FirstFree ) );

#ifdef TOI_TEMPLATED_POOL_ALLOCATOR_SET_ALLOCATED_MEMORY
		memset( toAllocate, TOI_TEMPLATED_POOL_ALLOCATOR_SET_ALLOCATED_MEMORY_VALUE, BlockSize );
#endif

		return toAllocate;
	}

	void deallocate( void* memory ) {
#ifdef TOI_TEMPLATED_POOL_ALLOCATOR_SET_FREED_MEMORY
		memset( memory, TOI_TEMPLATED_POOL_ALLOCATOR_SET_FREED_MEMORY_VALUE, BlockSize );
#endif

		size_t* toSet = reinterpret_cast<size_t*>( memory );
		*toSet = reinterpret_cast<size_t>( m_FirstFree );
		m_FirstFree = reinterpret_cast<size_t*>( memory );
	}


	template<typename Type>
	void PrintMemory() {
		assert( sizeof(Type ) <= BlockSize );

		for ( size_t i = 0; i < NrOfBlocks * BlockSize; i+= BlockSize ) {
			Type* block = reinterpret_cast<Type*>( m_Memory + i );
			std::cout << std::hex << *block << std::endl;
		}
		std::cout << std::resetiosflags( std::ios::hex ) << std::endl;;
	}

	template<typename Type, typename... Args>
	Type* construct( Args&&... args ) {
		assert( sizeof( Type ) <= BlockSize );

		Type* data = static_cast<Type*>( allocate() );
		new(data) Type(std::forward<Args>( args )...);
		return data;
	}

	template<typename Type>
	void destroy( Type* memory ) {
		memory->~Type();
		assert( sizeof( Type ) <= BlockSize );
		deallocate( memory );
	}

private:
	uint8_t* m_Memory = nullptr;
	size_t* m_FirstFree = nullptr;

};

template<size_t BlockSize>
static ToiTemplatedPoolAllocator<BlockSize, TOI_TEMPLATED_POOL_ALLOCATOR_NR_OF_BLOCKS>& GetLockableThreadPoolAllocator() {
	static thread_local ToiTemplatedPoolAllocator<BlockSize, TOI_TEMPLATED_POOL_ALLOCATOR_NR_OF_BLOCKS> poolAllocator;
	return poolAllocator;
}
