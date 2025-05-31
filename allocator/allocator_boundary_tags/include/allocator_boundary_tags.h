#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <pp_allocator.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <iterator>
#include <mutex>

struct alignas(size_t) block_metadata {
	size_t size : 63;
	bool allocated : 1;
	block_metadata* next;
	block_metadata* prev;
	struct allocator_metadata* parent;
} __attribute__( (__packed__) );

struct alignas(size_t) allocator_metadata {
	logger* loggerObj;
	std::pmr::memory_resource* allocatorObj;
	allocator_with_fit_mode::fit_mode fitMode;
	size_t memSize;
	std::mutex globalLock;
	struct block_metadata* firstBlock;
};




class allocator_boundary_tags final :
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    static constexpr const size_t metadataSize = sizeof(struct allocator_metadata);

    static constexpr const size_t allocatedMetadataSize = sizeof(struct block_metadata);

    static constexpr const size_t freeMetadataSize = 0;

    void* _allocatorMemory;

public:

	struct BuddyAllocatorBlockMetadata* get_first(size_t size) noexcept;
	struct BuddyAllocatorBlockMetadata* get_worst(size_t size) noexcept;
	struct BuddyAllocatorBlockMetadata* get_best(size_t size) noexcept;
	~allocator_boundary_tags() override;
	
    allocator_boundary_tags(
        size_t memSize,
        std::pmr::memory_resource *allocatorObj,
        logger *loggerObj,
        allocator_with_fit_mode::fit_mode fitMode );

inline bool canMergePrev( struct block_metadata* block);

inline bool canMergeNext( struct block_metadata* block);
allocator_boundary_tags( allocator_boundary_tags &&other ) noexcept;

void initBlockMetadata( struct block_metadata* block, struct block_metadata* prev, size_t size );

void splitBlockAndInit( struct block_metadata* block, size_t size );

void mergeBlocks( struct block_metadata* a, struct block_metadata* b );

[[nodiscard]] void* allocate( size_t size );
[[nodiscard]] void* do_allocate_sm( size_t size );
void do_deallocate_sm( void* ptr );
bool do_is_equal(const std::pmr::memory_resource& other) const noexcept;
	
void deallocate( void* ptr );

struct block_metadata* firstfit( size_t size );

struct block_metadata* bestfit( size_t size );

struct block_metadata* worstfit( size_t size );

void* trustedMemoryBegin() const;

size_t realBlockSize( size_t size );

inline void set_fit_mode( allocator_with_fit_mode::fit_mode mode );
inline std::string get_typename() const noexcept;
std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const;
std::vector<allocator_test_utils::block_info> get_blocks_info() const;

allocator_with_fit_mode::fit_mode fitMode() const;

logger* get_logger() const;
std::mutex& mutex() const;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H