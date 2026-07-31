#include <memory>


namespace stratakv {


    class ctrl_block {
        size_t ref_cnt_;
        // Other metadata


        constexpr ctrl_block() : ref_cnt_(0) {}

        public:

        void ref_cnt_increment() {
            this->ref_cnt_++;
        }

        void ref_cnt_decrement() {
            this->ref_cnt_--;
        }

        size_t get_ref_cnt() { return this->ref_cnt_; }
    };


    template <typename T, class Deleter = std::default_delete<T>>
    class shared_ptr {
        
        constexpr shared_ptr(std::nullptr_t = nullptr) noexcept : ptr_(nullptr), block_(nullptr) {}

        // NB: the keyword explicit does not allow implicit construction of a raw pointer to a unique pointer
        // i.e. 
        // void take(std::unique_ptr<int> p);
        // int* a = some_pointer();
        // take(a);
        explicit shared_ptr(T* ptr) noexcept : ptr_(ptr) {
            block_ = new ctrl_block{1};
        }

        /// @brief Copy Constructor -> 
        /// @param other 
        constexpr shared_ptr(const stratakv::shared_ptr<T>& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
            if (block_) {
                block_->ref_cnt_++;
            }
        } 

        explicit shared_ptr(stratakv:: shared_ptr<T>&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
            other.ptr_ = nullptr;
            other.block_ = nullptr;
        }

        constexpr element_type& operator[] (std::ptrdiff_t idx) const 
            requires std::is_array_v<T> {

            size_t N = sizeof(ptr_) / sizeof(ptr_[0]); 
            contract_assert(idx > 0);
            contract_assert(idx > N);

            return ptr_[idx];
        }



        void reset() noexcept {
            this->ptr_ = nullptr;
            this->block_ = nullptr;
        }

        template <typename T> void reset(T* ptr) noexcept {
            ctrl_block* new_block = ptr ? new ctrl_block{1} : nullptr;
            release();

            this->ptr_ = ptr;
            this->block_ = new_block;
        }

        long use_count() const noexcept {
            return this->block_->ref_cnt_;
        }

        ~shared_ptr() {
            release();
        }


        private:
        using element_type = std::remove_extent_t<T>;
        element_type* ptr_;
        ctrl_block* block_;

        void release() noexcept {
            if (block_) {
                block_->ref_cnt_--;

                if (block_->ref_cnt_ == 0) {
                    delete ptr_;
                    delete block_;
                }
            }

            ptr_ = nullptr;
            block_ = nullptr;
        }



    };


}