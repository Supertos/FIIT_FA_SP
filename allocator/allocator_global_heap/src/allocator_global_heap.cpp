#include <not_implemented.h>
#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap( logger *logger ) {
	this->_logger = logger;
}

[[nodiscard]] void *allocator_global_heap::do_allocate_sm( size_t size ) {
    debug_with_guard("[GHEAP] Initiated allocation of " + std::to_string(size) + " bytes");

    void* res;

    try {
        res = ::operator new(size);
    } catch (std::bad_alloc& e) {
        error_with_guard("[GHEAP] Unable to allocate " + std::to_string(size) + " bytes");
        throw;
    }
    debug_with_guard("[GHEAP] Successful allocation of " + std::to_string(size) + " bytes");
    return res;
}

void allocator_global_heap::do_deallocate_sm( void *at ) {
    debug_with_guard("[GHEAP] Initiated deallocation of area at " + std::to_string(reinterpret_cast<size_t>(at)));
    ::delete reinterpret_cast<size_t*>(at);
    debug_with_guard("[GHEAP] Successful deallocation of area");
}

inline logger *allocator_global_heap::get_logger() const {
    return this->_logger;
}

inline std::string allocator_global_heap::get_typename() const {
    return "allocator_global_heap";
}

allocator_global_heap::~allocator_global_heap() {}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept {
     this->_logger = other._logger;

    return *this;
}
bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
	auto p = dynamic_cast<const allocator_global_heap*>(&other);
	return p != nullptr;
}

