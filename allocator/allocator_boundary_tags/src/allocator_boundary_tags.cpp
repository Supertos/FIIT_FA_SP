//#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags() {
	std::lock_guard lock(mutex());
	size_t totalSize = reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->memSize;
	if( reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->allocatorObj ) {
		reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->allocatorObj->deallocate( reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory), totalSize );
	}else{
		::operator delete( reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory) );
	}
}

allocator_boundary_tags::allocator_boundary_tags( allocator_boundary_tags &&other ) noexcept {
    this->_allocatorMemory = other._allocatorMemory;
}

inline void* recomputeWithOffset( void* a, size_t offset ) {
	return reinterpret_cast<void*>(reinterpret_cast<char*>(a) + offset);
}
inline void* recomputeWithNegOffset( void* a, size_t offset ) {
	return reinterpret_cast<void*>(reinterpret_cast<char*>(a) - offset);
}

/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t memSize,
        std::pmr::memory_resource *allocator,
        logger *loggerObj,
        allocator_with_fit_mode::fit_mode fitMode ){
    
	size_t totalSize = memSize + allocator_boundary_tags::metadataSize;
	if( memSize < allocator_boundary_tags::metadataSize ) throw std::logic_error( "Not enough space" );
	
	void* allocatedMemory = (!allocator) ? ::operator new(totalSize) : allocator->allocate( totalSize );
	if( !allocatedMemory ) {
		error_with_guard("[BOUNDARY_TAGS] Unable to init allocator!");
		return;
	}
	
	this->_allocatorMemory = new(allocatedMemory) allocator_metadata();
	
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->loggerObj = loggerObj;
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->allocatorObj = allocator;
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->memSize = memSize;
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->fitMode = fitMode;
	
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->firstBlock = reinterpret_cast<struct block_metadata*>(recomputeWithOffset(allocatedMemory, allocator_boundary_tags::metadataSize));
	
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->firstBlock->size = memSize - sizeof( struct block_metadata );
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->firstBlock->next = nullptr;
	reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->firstBlock->prev = nullptr;
}

inline bool allocator_boundary_tags::canMergePrev( struct block_metadata* block) {
	if( !block || !block->prev || block->prev->allocated ) return false;
	return recomputeWithOffset(reinterpret_cast<void*>(block->prev), block->prev->size) == reinterpret_cast<void*>(block);
}

inline bool allocator_boundary_tags::canMergeNext( struct block_metadata* block) {
	if( !block || !block->next || block->next->allocated ) return false;
	return recomputeWithOffset(reinterpret_cast<void*>(block), block->size) == reinterpret_cast<void*>(block->next);
}

void allocator_boundary_tags::initBlockMetadata( struct block_metadata* block, struct block_metadata* prev, size_t size ) {
	if( !block ) return;
	block->parent = reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory);
	block->size = size;
	
	if( !prev ) return; // Init run, no block here yet.
	block->prev = prev;
	
	if( prev->next ) block->next = prev->next;
	prev->next = block;
}

void allocator_boundary_tags::splitBlockAndInit( struct block_metadata* block, size_t size ) {
	if( !block || block->size <= size ) return;
	
	size_t nextSize = block->size - size;
	block->size = size;
	
	void* voidMem = recomputeWithOffset(reinterpret_cast<void*>(block), size);
	
	new (voidMem) block_metadata();
	this->initBlockMetadata(reinterpret_cast<struct block_metadata*>(voidMem), block, nextSize );
	warning_with_guard("[BOUNDARY_TAGS] Split block at addr." + std::to_string(reinterpret_cast<size_t>(block)));
}

void allocator_boundary_tags::mergeBlocks( struct block_metadata* a, struct block_metadata* b ) {
	if( !a || !b ) return;
	struct block_metadata* acceptor = a;
	struct block_metadata* donnor = b;
	if( a > b ) std::swap(acceptor, donnor);
	
	acceptor->next = donnor->next;
	if( acceptor->next ) acceptor->next->prev = acceptor;
	
	debug_with_guard("[BOUNDARY_TAGS] Merged block" + std::to_string(reinterpret_cast<size_t>(acceptor)) + "(acceptor) with " + std::to_string(reinterpret_cast<size_t>(donnor)) + "(donnor)");
}

[[nodiscard]] void* allocator_boundary_tags::allocate( size_t size ) {
	// if( size == 0 ) return nullptr;
	std::lock_guard<std::mutex> lock(this->mutex());
	debug_with_guard("[BOUNDARY_TAGS] Initiated allocation of " + std::to_string(size) + " bytes boundary tags");
	
	size_t minBlockSize = this->realBlockSize( size );
	
	struct block_metadata* freeBlock;
    switch( this->fitMode() ) {
        case allocator_with_fit_mode::fit_mode::first_fit:
            freeBlock = this->firstfit( minBlockSize );
        break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            freeBlock = this->bestfit( minBlockSize );
        break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            freeBlock = this->worstfit( minBlockSize );
        break;
    }
	
	debug_with_guard("[BOUNDARY_TAGS] Allocated" + std::to_string(reinterpret_cast<size_t>(freeBlock)));
	if( !freeBlock ) {
		error_with_guard("[BOUNDARY_TAGS] Unable to allocate" + std::to_string(minBlockSize) + " bytes");
		throw std::bad_alloc();
	}
	
	debug_with_guard("[BOUNDARY_TAGS] Found block of size" + std::to_string(freeBlock->size) + " bytes");
	
	if( freeBlock->size > minBlockSize + allocator_boundary_tags::allocatedMetadataSize ) {
		this->splitBlockAndInit( freeBlock, minBlockSize );
	}
	
	freeBlock->allocated = true;
	freeBlock->parent = reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory);
	return recomputeWithOffset( freeBlock, allocator_boundary_tags::allocatedMetadataSize );
}

void allocator_boundary_tags::deallocate( void* ptr ) {
	if( !ptr ) return;
	std::lock_guard<std::mutex> lock(this->mutex());
	debug_with_guard("[BOUNDARY_TAGS] Initiated free of block at addr." + std::to_string(reinterpret_cast<size_t>(ptr)));
	
	auto block = reinterpret_cast<struct block_metadata*>(recomputeWithNegOffset(ptr, allocator_boundary_tags::allocatedMetadataSize));
	
	if( !block->allocated || block->parent != reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory) ) {
		error_with_guard( "[BOUNDARY_TAGS] Double free attempt!" );
		return;
	}
	debug_with_guard("[BOUNDARY_TAGS] Freed!");
	
	
	block->allocated = false;
	if( this->canMergeNext(block) ) mergeBlocks( block, block->next );
	
	if( this->canMergePrev(block) ) mergeBlocks( block->prev, block );
}

struct block_metadata* allocator_boundary_tags::firstfit( size_t size ) {
	struct block_metadata* curBlock = reinterpret_cast<struct block_metadata*>(this->trustedMemoryBegin());
	debug_with_guard("[BOUNDARY_TAGS] Begin at" + std::to_string(reinterpret_cast<size_t>(curBlock)));
	do {
		debug_with_guard("[BOUNDARY_TAGS] Block" + std::to_string(reinterpret_cast<size_t>(curBlock)) + "/" +
		std::to_string(curBlock->allocated) + "/" +
		std::to_string(reinterpret_cast<size_t>(curBlock->size))
		);
		if( !curBlock->allocated && curBlock->size >= size ) return curBlock;
	} while( (curBlock=curBlock->next) != nullptr );
	
	return nullptr;
}

struct block_metadata* allocator_boundary_tags::bestfit( size_t size ) {
	struct block_metadata* curBlock = reinterpret_cast<struct block_metadata*>(this->trustedMemoryBegin());
	
	struct block_metadata* bestBlock = nullptr;
	size_t bestSize = ~size_t(0); // Fuck limits.h
	do {
		size_t blockSize = curBlock->size;
		if( !curBlock->allocated && blockSize >= size && blockSize < bestSize ) {
			bestSize = blockSize;
			bestBlock = curBlock;
		}
	} while( (curBlock=curBlock->next) != nullptr );
	
	return bestBlock;
}

[[nodiscard]] void* allocator_boundary_tags::do_allocate_sm( size_t size ) { return this->allocate( size ); }
void allocator_boundary_tags::do_deallocate_sm( void* ptr ) { this->deallocate( ptr ); }

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
	auto p = dynamic_cast<const allocator_boundary_tags*>(&other);
	return p != nullptr;
}


struct block_metadata* allocator_boundary_tags::worstfit( size_t size ) {
	struct block_metadata* curBlock = reinterpret_cast<struct block_metadata*>(this->trustedMemoryBegin());
	
	struct block_metadata* worstBlock = nullptr;
	size_t worstSize = 0; // Fuck limits.h
	do {
		size_t blockSize = curBlock->size;
		if( !curBlock->allocated && blockSize >= size && blockSize > worstSize ) {
			worstSize = blockSize;
			worstBlock = curBlock;
		}
	} while( (curBlock=curBlock->next) != nullptr );
	
	return worstBlock;
}

void* allocator_boundary_tags::trustedMemoryBegin() const {
	return reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->firstBlock;
}

size_t allocator_boundary_tags::realBlockSize( size_t size ) {
	return size + allocator_boundary_tags::allocatedMetadataSize;
}

allocator_with_fit_mode::fit_mode allocator_boundary_tags::fitMode() const {
    return reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->fitMode;
}

logger* allocator_boundary_tags::get_logger() const {
    return reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->loggerObj;
}

std::mutex& allocator_boundary_tags::mutex() const {
    return reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->globalLock;
}

inline std::string allocator_boundary_tags::get_typename() const noexcept {
    return "allocator_boundary_tags";
}

inline void allocator_boundary_tags::set_fit_mode( allocator_with_fit_mode::fit_mode mode ) {
    std::lock_guard lock(this->mutex());
    reinterpret_cast<struct allocator_metadata*>(this->_allocatorMemory)->fitMode = mode;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const {
    std::vector<allocator_test_utils::block_info> out;

    std::back_insert_iterator<std::vector<allocator_test_utils::block_info>> inserter(out);

	struct block_metadata* curBlock = reinterpret_cast<struct block_metadata*>(this->trustedMemoryBegin());
	
	struct block_metadata* worstBlock = nullptr;
	size_t worstSize = ~size_t(0); // Fuck limits.h
	do {
		inserter = {curBlock->size + (curBlock->allocated ? 0 : sizeof( struct block_metadata ) ) , curBlock->allocated};
	} while( (curBlock=curBlock->next) != nullptr );

    return out;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const {
    std::lock_guard lock(this->mutex());

    return get_blocks_info_inner();
}