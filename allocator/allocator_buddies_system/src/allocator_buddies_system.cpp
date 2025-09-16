#include <not_implemented.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include "../include/allocator_buddies_system.h"


allocator_buddies_system::BuddyBlock* allocator_buddies_system::next_block( allocator_buddies_system::BuddyBlock* b ) const {
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
	
	auto addr = (allocator_buddies_system::BuddyBlock*)((uintptr_t)b + (size_t)pow(2, b->size)); // + sizeof(allocator_buddies_system::BuddyBlock));
	return (uintptr_t)(addr) < (uintptr_t)data + data->memSize - 2 ? addr : nullptr; // 2 Because of block metadata
}

allocator_buddies_system::BuddyBlock* allocator_buddies_system::get_buddy( allocator_buddies_system::BuddyBlock* b ) const {
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
    size_t block_size = size_t(1) << b->size;
    uintptr_t base_address = ((uintptr_t)this->_trusted_memory + sizeof(BuddyMetadata));
    uintptr_t block_offset = reinterpret_cast<uintptr_t>(b) - base_address;
    uintptr_t buddy_offset = block_offset ^ block_size;
    
    if (buddy_offset + block_size > data->memSize) {
        return nullptr;
    }
    
    return reinterpret_cast<allocator_buddies_system::BuddyBlock*>(base_address + buddy_offset);
}

std::mutex& allocator_buddies_system::mutex() const {
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
    return data->globalLock;
}
allocator_buddies_system::~allocator_buddies_system() {
	std::lock_guard lock(this->mutex());
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
	
	if( data->allocatorObj ) {
		data->allocatorObj->deallocate( this->_trusted_memory, data->memSize );
	}else{
		::operator delete( this->_trusted_memory );
	}
}
allocator_buddies_system::allocator_buddies_system( size_t size, std::pmr::memory_resource* parentAllocator, logger* logger, allocator_with_fit_mode::fit_mode fitMode ) {
	if( size < 5 ) throw std::logic_error("[BUDDY] Insufficient space");
	
	size_t allocSize = (1 << (size_t)(log2( size ))) + sizeof(BuddyMetadata);
	
	this->_trusted_memory = (parentAllocator == nullptr) ? ::operator new(allocSize) : parentAllocator->allocate(allocSize, 1);
	
	BuddyMetadata* data = (BuddyMetadata*)this->_trusted_memory;
	
	memset(&data->blocks, 0, sizeof(BuddyMetadata));
	data->loggerObj = logger;
	data->allocatorObj = parentAllocator;
	data->fitMode = fitMode;
	data->memSize = allocSize;
	
	data->blocks[(size_t)(log2(size))] = (allocator_buddies_system::BuddyBlock*)((uintptr_t)this->_trusted_memory + sizeof(BuddyMetadata));
	
	data->blocks[(size_t)(log2(size))]->occupied = false;
	data->blocks[(size_t)(log2(size))]->size = log2(size);
	data->blocks[(size_t)(log2(size))]->next = 0;
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm( size_t size ) {
    std::lock_guard lock(this->mutex());
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
	debug_with_guard("[BUDDY] Allocating " + std::to_string(size) + " bytes");
	
	allocator_buddies_system::BuddyBlock* freeBlock = this->get_block(size);
	
	if( freeBlock == nullptr ) {
		debug_with_guard("[BUDDY] Unable to allocate " + std::to_string(size) + " bytes");
        return nullptr;
	}
	
	debug_with_guard("[BUDDY] Block found." + std::to_string((size_t)freeBlock) );
	
	if( freeBlock->size != size ) {
		warning_with_guard( "[BUDDY] allocated space of " + std::to_string(pow(2,freeBlock->size)) + "bytes allocated instead of requested space!" );
	}

	
	return reinterpret_cast<void*>((uintptr_t)(freeBlock) + 1);
}


void allocator_buddies_system::do_deallocate_sm( void *at ) {
	if( at == nullptr ) return;
    std::lock_guard lock(this->mutex());
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
	allocator_buddies_system::BuddyBlock* block = (allocator_buddies_system::BuddyBlock*)((uintptr_t)(at) - 1);
	
	debug_with_guard("[BUDDY] Freeing obj");
	if( (uintptr_t)(at) < (uintptr_t)(this->_trusted_memory) || (uintptr_t)(at) > (uintptr_t)(this->_trusted_memory) + data->memSize + sizeof(allocator_buddies_system::BuddyBlock) ) {
		error_with_guard("[BUDDY] Invalid deallocation");
		throw std::logic_error("[BUDDY] Invalid deallocation!");
	}
	
	while( this->get_buddy(block) && !this->get_buddy(block)->occupied && this->get_buddy(block)->size == block->size ) {
		if( this->get_buddy(block) < block ) block = this->get_buddy(block);
		
		allocator_buddies_system::BuddyBlock* buddy = this->get_buddy(block);
		
		if( buddy->prev == 0 ) {
			if( buddy->next ) reinterpret_cast<allocator_buddies_system::BuddyBlock*>(buddy->next)->prev = 0;
			data->blocks[buddy->size] = reinterpret_cast<allocator_buddies_system::BuddyBlock*>(buddy->next);
		}else{
			reinterpret_cast<allocator_buddies_system::BuddyBlock*>(buddy->prev)->next = buddy->next;
			if( buddy->next ) reinterpret_cast<allocator_buddies_system::BuddyBlock*>(buddy->next)->prev = buddy->prev;
		}
		
		block->size++;
		debug_with_guard("[BUDDY] Merged Block!");
	}
	
	if( data->blocks[block->size] != nullptr ) {
		block->next = (uintptr_t)data->blocks[block->size];
		data->blocks[block->size]->prev = (uintptr_t)block;
	}
	data->blocks[block->size] = block;
	block->prev = 0;
	
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
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
    return data->loggerObj;
}

inline void allocator_buddies_system::set_fit_mode( allocator_with_fit_mode::fit_mode mode ) {
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
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
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
    std::vector<allocator_test_utils::block_info> out;

    std::back_insert_iterator<std::vector<allocator_test_utils::block_info>> inserter(out);

	allocator_buddies_system::BuddyBlock* curBlock = reinterpret_cast<allocator_buddies_system::BuddyBlock*>((uintptr_t)data + sizeof(BuddyMetadata));
	
	allocator_buddies_system::BuddyBlock* worstBlock = nullptr;
	do {
		inserter = {static_cast<size_t>(pow(2, curBlock->size)), curBlock->occupied};
	} while( (curBlock=this->next_block(curBlock)) != nullptr && curBlock->size != 0 );

    return out;
}

void allocator_buddies_system::split_first_block( allocator_buddies_system::BuddyBlock* block ) {
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
	size_t oldSize = block->size;
	data->blocks[oldSize] = reinterpret_cast<allocator_buddies_system::BuddyBlock*>(block->next);
	
	
	block->size--;
	
	allocator_buddies_system::BuddyBlock* buddy = this->get_buddy( block );

	buddy->next = (uintptr_t)data->blocks[block->size];
	if( buddy->next != 0 ) (reinterpret_cast<allocator_buddies_system::BuddyBlock*>(buddy->next))->prev = (uintptr_t)buddy;
	block->next = (uintptr_t)buddy;
	buddy->prev = (uintptr_t)block;
	block->prev = 0;
	
	buddy->size = block->size;
	buddy->occupied = false;
	
	data->blocks[block->size] = block;
}

allocator_buddies_system::BuddyBlock* allocator_buddies_system::get_block(size_t size) noexcept {
	BuddyMetadata* data = (BuddyMetadata*)(this->_trusted_memory);
	
	allocator_buddies_system::BuddyBlock** blocks = data->blocks;
	
	size_t initialOffset = size == 0 ? 4 : ceil(log2(size));
	if( size == (size_t)pow(2, initialOffset) ) initialOffset++;
	for( size_t offset = initialOffset; offset < CPU_VM_BITS; ++offset ) {
		if( blocks[offset] && blocks[offset]->size != offset ) return nullptr;
		if( blocks[offset] ) {
			
			while( initialOffset < offset ) {
				this->split_first_block( blocks[offset--] );
			}
			
			blocks[offset]->occupied = true;
			blocks[offset]->size = offset;
			allocator_buddies_system::BuddyBlock* block = blocks[offset];
			blocks[offset] = reinterpret_cast<allocator_buddies_system::BuddyBlock*>(blocks[offset]->next);
			if( blocks[offset] ) blocks[offset]->prev = 0;
			
			return block;
		}
	}
	
	return nullptr;
}