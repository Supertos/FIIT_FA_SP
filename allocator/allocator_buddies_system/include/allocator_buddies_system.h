#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H

#include <pp_allocator.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <mutex>
#include <cmath>

struct BuddyAllocatorMetadata {
	logger* loggerObj;
	std::pmr::memory_resource* allocatorObj;
	allocator_with_fit_mode::fit_mode fitMode;
	size_t memSize;
	std::mutex globalLock;
	struct BuddyAllocatorBlockMetadata* begin;
};
struct BuddyAllocatorBlockMetadata {
	bool occupied : 1;
	uint8_t size : 7;
} __attribute__( (__packed__) );

namespace __detail
{
    constexpr size_t nearest_greater_k_of_2(size_t size) noexcept {
        int ones_counter = 0, index = -1;

        constexpr const size_t o = 1;

        for (int i = sizeof(size_t) * 8 - 1; i >= 0; --i) {
            if (size & (o << i)) {
                if (ones_counter == 0)
                    index = i;
                ++ones_counter;
            }
        }

        return ones_counter <= 1 ? index : index + 1;
    }
}

class allocator_buddies_system final:
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    void *_trusted_memory;
	static constexpr const size_t min_k = __detail::nearest_greater_k_of_2(sizeof(struct BuddyAllocatorBlockMetadata));

public:

    explicit allocator_buddies_system(
            size_t space_size_power_of_two,
            std::pmr::memory_resource *parent_allocator = nullptr,
            logger *logger = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

    allocator_buddies_system(
        allocator_buddies_system const &other);
    
    allocator_buddies_system &operator=(
        allocator_buddies_system const &other);
    
    allocator_buddies_system(
        allocator_buddies_system &&other) noexcept;
    
    allocator_buddies_system &operator=(
        allocator_buddies_system &&other) noexcept;
	struct BuddyAllocatorBlockMetadata* next_block( struct BuddyAllocatorBlockMetadata* b ) const;
    ~allocator_buddies_system() override;
	std::mutex& mutex() const;
public:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t size) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;


    std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

struct BuddyAllocatorBlockMetadata* get_best(size_t size) noexcept;
struct BuddyAllocatorBlockMetadata* get_worst(size_t size) noexcept;
struct BuddyAllocatorBlockMetadata* get_first(size_t size) noexcept;

private:

    
    inline logger *get_logger() const override;
    
    inline std::string get_typename() const override;

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;

    
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H

