#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"


struct BuddyAllocatorBlockMetadata* allocator_buddies_system::next_block( struct BuddyAllocatorBlockMetadata* b ) const {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	
	auto addr = reinterpret_cast<struct BuddyAllocatorBlockMetadata*>(reinterpret_cast<std::byte*>(b) + (size_t)pow(2, b->size)); // + sizeof(struct BuddyAllocatorBlockMetadata));
	return reinterpret_cast<std::byte*>(addr) < reinterpret_cast<std::byte*>(data) + data->memSize - 2 ? addr : nullptr; // 2 Because of block metadata
}
std::mutex& allocator_buddies_system::mutex() const {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
    return data->globalLock;
}
allocator_buddies_system::~allocator_buddies_system() {
	std::lock_guard lock(this->mutex());
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	
	if( data->allocatorObj ) {
		data->allocatorObj->deallocate( this->_trusted_memory, data->memSize );
	}else{
		::operator delete( this->_trusted_memory );
	}
}
allocator_buddies_system::allocator_buddies_system(
        size_t size, std::pmr::memory_resource *parentAllocator,
        logger *logger, allocator_with_fit_mode::fit_mode fitMode) {
	
	if( size < 5 ) {
		throw std::logic_error("[BUDDY] Insufficient space");
	}
	
	size_t allocSize = (1 << (int)(log2( size ))) + sizeof(struct BuddyAllocatorMetadata) + sizeof(struct BuddyAllocatorBlockMetadata);
	
	
	
	this->_trusted_memory = (parentAllocator == nullptr) ? ::operator new(allocSize) : parentAllocator->allocate(allocSize, 1);
	
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	
	data->loggerObj = logger ? logger : nullptr;
	data->allocatorObj = parentAllocator ? parentAllocator : nullptr;
	data->fitMode = fitMode;
	data->memSize = allocSize;
	data->begin = reinterpret_cast<struct BuddyAllocatorBlockMetadata*>(reinterpret_cast<char*>(this->_trusted_memory) + sizeof( struct BuddyAllocatorMetadata ));
	
	data->begin->occupied = false;
	data->begin->size = log2(size);
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm( size_t size ) {
    std::lock_guard lock(this->mutex());
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	size_t realSize = size; // + pow(2, 5); // 5 - log2(sizeof(struct BuddyAllocatorBlockMetadata))
	debug_with_guard("[BUDDY] Allocating " + std::to_string(realSize) + " bytes");
	
	struct BuddyAllocatorBlockMetadata* freeBlock = nullptr;
	
	switch( data->fitMode ) {
        case allocator_with_fit_mode::fit_mode::first_fit:
            freeBlock = get_first(realSize);
            break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            freeBlock = get_best(realSize);
            break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            freeBlock = get_worst(realSize);
            break;
		default:
			throw std::bad_alloc();
	}
	
	if( freeBlock == nullptr ) {
		debug_with_guard("[BUDDY] Unable to allocate " + std::to_string(realSize) + " bytes");
        return nullptr;
	}
	
	debug_with_guard("[BUDDY] Block found." + std::to_string((size_t)freeBlock) );
	
	while( pow(2, freeBlock->size) > realSize * 2 && freeBlock->size > 4 ) { // 4 is required by tests
		freeBlock->size--;
		struct BuddyAllocatorBlockMetadata* next = this->next_block(freeBlock);
		next->size = freeBlock->size;
		next->occupied = false;
		// if( next ) next->prev = realNext;
	}
	debug_with_guard("[BUDDY] Splitting process finished." );
	
	if( freeBlock->size != realSize ) {
		warning_with_guard( "[BUDDY] allocated space of " + std::to_string(pow(2,freeBlock->size)) + "bytes allocated instead of requested space!" );
	}

	freeBlock->occupied = true;
	
	return reinterpret_cast<void*>(reinterpret_cast<std::byte*>(freeBlock) + sizeof(struct BuddyAllocatorBlockMetadata));
}


void allocator_buddies_system::do_deallocate_sm( void *at ) {
	if( at == nullptr ) return;
    std::lock_guard lock(this->mutex());
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	struct BuddyAllocatorBlockMetadata* block = reinterpret_cast<struct BuddyAllocatorBlockMetadata*>(reinterpret_cast<std::byte*>(at) - sizeof(struct BuddyAllocatorBlockMetadata));
	
	if( reinterpret_cast<std::byte*>(at) < reinterpret_cast<std::byte*>(this->_trusted_memory) || reinterpret_cast<std::byte*>(at) > reinterpret_cast<std::byte*>(this->_trusted_memory) + data->memSize + sizeof(struct BuddyAllocatorBlockMetadata) ) {
		error_with_guard("[BUDDY] Invalid deallocation");
		throw std::logic_error("[BUDDY] Invalid deallocation!");
	}
	
	// while( block->prev && block->prev->occupied && block->prev->size == block->size )
		// block = block->prev;
	
	
	while( this->next_block(block) && !this->next_block(block)->occupied && this->next_block(block)->size == block->size ) {
		block->size++;
		debug_with_guard("[BUDDY] Merged Block!");
		// this->next_block(block) = this->next_block(this->next_block(block));
		// if( this->next_block(block) ) this->next_block(block)->prev = block;	
	}
	
	block->occupied = false;
	
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other) {
    this->_trusted_memory = other._trusted_memory;
}

allocator_buddies_system &allocator_buddies_system::operator=(const allocator_buddies_system &other) {
    if (this != &other)
        this->_trusted_memory = other._trusted_memory;
    return *this;
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
    auto p = dynamic_cast<const allocator_buddies_system*>(&other);
	return p != nullptr;
}

logger* allocator_buddies_system::get_logger() const {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
    return data->loggerObj;
}

inline void allocator_buddies_system::set_fit_mode( allocator_with_fit_mode::fit_mode mode ) {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
    std::lock_guard lock(this->mutex());
    data->fitMode = mode;
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept {
	std::lock_guard lock(this->mutex());
	
    return get_blocks_info_inner();
}


inline std::string allocator_buddies_system::get_typename() const {
    return "allocator_buddies_system";
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
    std::vector<allocator_test_utils::block_info> out;

    std::back_insert_iterator<std::vector<allocator_test_utils::block_info>> inserter(out);

	struct BuddyAllocatorBlockMetadata* curBlock = data->begin;
	
	struct BuddyAllocatorBlockMetadata* worstBlock = nullptr;
	do {
		inserter = {static_cast<size_t>(pow(2, curBlock->size)), curBlock->occupied};
	} while( (curBlock=this->next_block(curBlock)) != nullptr );

    return out;
}

struct BuddyAllocatorBlockMetadata* allocator_buddies_system::get_best(size_t size) noexcept {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	struct BuddyAllocatorBlockMetadata* best = nullptr;
	
	struct BuddyAllocatorBlockMetadata* cur = data->begin;
	uint8_t bestSize = ~uint8_t(0);
	
	do {
		if( !cur->occupied && pow(2, cur->size) >= size && cur->size < bestSize ) {
			bestSize = cur->size;
			best = cur;
		}
	} while( (cur=this->next_block(cur)) != nullptr );

    return best;
}

struct BuddyAllocatorBlockMetadata* allocator_buddies_system::get_worst(size_t size) noexcept {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	struct BuddyAllocatorBlockMetadata* worst = nullptr;
	
	struct BuddyAllocatorBlockMetadata* cur = data->begin;
	uint8_t worstSize = 0;
	
	do {
		if( !cur->occupied && pow(2, cur->size) >= size && cur->size > worstSize ) {
			worstSize = cur->size;
			worst = cur;
		}
	} while( (cur=this->next_block(cur)) != nullptr );

    return worst;
}


struct BuddyAllocatorBlockMetadata* allocator_buddies_system::get_first(size_t size) noexcept {
	struct BuddyAllocatorMetadata* data = reinterpret_cast<struct BuddyAllocatorMetadata*>(this->_trusted_memory);
	
	struct BuddyAllocatorBlockMetadata* cur = data->begin;
	
	do {
		if( !cur->occupied && pow(2, cur->size) > size ) return cur;
	} while( (cur=this->next_block(cur)) != nullptr );

    return nullptr;
}