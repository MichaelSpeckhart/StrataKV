#include <new>
#include <sys/mman.h>
#include <cstddef>
#include <algorithm>
#include <utility>
#include <string>
#include <iostream>
#include <cstring>

template <typename T>
class FreeList {
  public:
  
  explicit FreeList(size_t numBlocks) :numBlocks_(numBlocks) {
    
    this->pool_ = static_cast<std::byte*>(mmap(NULL, numBlocks_ * blockSize_, PROT_READ | PROT_WRITE, MAP_ANONYMOUS| MAP_PRIVATE | MAP_POP, -1, 0));
    
    if (this->pool_ == MAP_FAILED) {
    }
    
    this->freeStart_ = nullptr;
    this->end = pool_ + (numBlocks * blockSize_);
    this->next = pool_;

    std::cout << "Start addr: " << pool_ << "\n";
    
  }
  
  FreeList(const FreeList& other) = delete;
  FreeList& operator=(const FreeList& other) = delete;
  
  FreeList(FreeList&& other) noexcept : numBlocks_(other.numBlocks_), pool_(std::exchange(other.pool_, nullptr)) {}
  
  FreeList& operator=(FreeList&& other) noexcept {
    if (this != &other) {
      if (pool_) munmap(pool_, numBlocks_ * blockSize_);

    
      this->pool_ = std::exchange(other.pool_, nullptr);
      this->numBlocks_ = other.numBlocks_;

      
    }
    
    return *this;
  }
  
  ~FreeList() {
    if (pool_) {
      munmap(this->pool_, this->numBlocks_ * this->blockSize_);
    }
  }
  
  template <typename... Args>
  T* acquire(Args&&... args) {  
    
    std::byte* block;
    
    if (freeStart_ != nullptr) {
      block = freeStart_;

      void* linkInBlock;
      std::memcpy(&linkInBlock, block, sizeof(linkInBlock));
      std::cout << "block " << static_cast<void*>(block)
          << " -> next-link: " << linkInBlock << "\n";
      std::memcpy(&freeStart_, block, sizeof(freeStart_));
      std::cout << "FreeStart after acquire: " << freeStart_ << "\n";
    }
    else if (next < end){
      block = next;
      next = next + blockSize_;
    }
    else {
      return nullptr;
    }
    
    return new (block) T(std::forward<Args>(args)...);
  }
  
  void release(T* p) {
    if (!p) return;
    
    p->~T();
    
    std::byte *block = reinterpret_cast<std::byte*>(p);

    void* linkInBlock;
    std::memcpy(&linkInBlock, freeStart_, sizeof(freeStart_));
    std::cout << "block " << static_cast<void*>(block)
          << " -> next-link: " << linkInBlock << "\n";
    
    std::memcpy(block, &freeStart_, sizeof(freeStart_));
    
    freeStart_ = block; 
    std::cout << "Free Start addr: " << freeStart_ << "\n";
    
  }
  
  
  private:
  size_t numBlocks_;
  std::byte* pool_;
  static constexpr size_t blockSize_ = (std::max(sizeof(T), sizeof(void*)) + alignof(T) - 1) & ~(alignof(T) - 1);
  
  unsigned int numUnitiailized_;
  unsigned int numFreeBlocks_;
  std::byte* next;
  std::byte* freeStart_;
  std::byte* end;
  
  
};

struct Order {
  const char* name;
  const char* source;
  const char* dest;
  float price;
};

// Size: 
struct Object {
  int i;
  int j;
  int k;
  
  Object(int i, int j, int k) : i(i), j(j), k(k) {}
};


int main() {
  FreeList<Object> list(3);
  
  auto obj1 = list.acquire(1,2,3);
  
  auto obj2 = list.acquire(1,2,3);
  
  auto obj3 = list.acquire(1,2,3);
  
  list.release(obj1);
  list.release(obj2);

  auto obj4 = list.acquire(1,2,3);
  std::cout << "Obj4 addr: " << obj4 << "\n"; 

  auto obj5 = list.acquire(1,2,3);
  std::cout << "Obj5 addr: " << obj5 << "\n"; 
  return 0;
}
