#include <memory>
#include <optional>

template <typename T>
class WeakPtr;

struct BaseControlBlock {
  int shared_count_ = 1;
  int weak_count_ = 0;
  virtual void use_deleter() = 0;
  virtual void use_deallocator() = 0;
  virtual ~BaseControlBlock() = default;
};

template <typename T>
class SharedPtr {
  friend class WeakPtr<T>;

  template <typename U>
  friend class SharedPtr;

  template <typename U>
  friend class WeakPtr;

  template <typename... Args>
  friend SharedPtr<T> make_shared(Args&&... args);

  template <typename U, typename Allocator, typename... Args>
  friend SharedPtr<U> allocateShared(const Allocator& alloc, Args&&... args);

 private:
  template <typename Deleter = std::default_delete<T>,
            typename Alloc = std::allocator<T>>
  struct ControlBlockRegular : public BaseControlBlock {
    T* object_;
    Deleter deleter_;
    using RegularAllocator = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockRegular<Deleter, Alloc>>;
    using RegularAllocatorTraits = std::allocator_traits<RegularAllocator>;
    [[no_unique_address]] RegularAllocator allocator_;
    ControlBlockRegular(T* other_object, const Deleter& other_deleter,
                        const Alloc& other_allocator)
        : object_(other_object),
          deleter_(other_deleter),
          allocator_(other_allocator) {
    }
    void use_deleter() override {
      deleter_(object_);
    }
    void use_deallocator() override {
      RegularAllocatorTraits::deallocate(allocator_, this, 1);
    }
  };

  template <typename Alloc = std::allocator<T>>
  struct ControlBlockMakeShared : public BaseControlBlock {
    std::optional<T> object_;
    using MakeSharedAllocator = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockMakeShared<Alloc>>;
    using MakeSharedAllocatorTraits =
        std::allocator_traits<MakeSharedAllocator>;
    [[no_unique_address]] MakeSharedAllocator allocator_;
    template <typename... Args>
    ControlBlockMakeShared(const Alloc& other_allocator, Args&&... args)
        : allocator_(other_allocator) {
      object_.emplace(std::forward<Args>(args)...);
    }
    void use_deleter() override {
      object_.reset();
    }
    void use_deallocator() override {
      MakeSharedAllocatorTraits::destroy(allocator_, this);
      MakeSharedAllocatorTraits::deallocate(allocator_, this, 1);
    }
  };

  T* object_ = nullptr;
  BaseControlBlock* block_ = nullptr;

  SharedPtr(T& object, BaseControlBlock* other_block)
      : object_(&object),
        block_(other_block) {
  }
  SharedPtr(const WeakPtr<T>& other)
      : object_(nullptr),
        block_(other.block_) {
    ++block_->shared_count_;
  }

 public:
  SharedPtr() {
  }
  SharedPtr(const SharedPtr& other)
      : object_(other.object_),
        block_(other.block_) {
    if (block_ != nullptr) {
      ++block_->shared_count_;
    }
  }
  template <typename U>
  SharedPtr(const SharedPtr<U>& other)
      : object_(other.object_),
        block_(other.block_) {
    if (block_ != nullptr) {
      ++block_->shared_count_;
    }
  }
  SharedPtr(SharedPtr&& other)
      : object_(std::move(other.object_)),
        block_(std::move(other.block_)) {
    other.object_ = nullptr;
    other.block_ = nullptr;
  }
  template <typename U>
  SharedPtr(SharedPtr<U>&& other)
      : object_(std::move(other.object_)),
        block_(std::move(other.block_)) {
    other.object_ = nullptr;
    other.block_ = nullptr;
  }
  template <typename Y, typename Deleter = std::default_delete<Y>,
            typename Alloc = std::allocator<Y>>
  SharedPtr(Y* other_object, const Deleter& other_deleter,
            const Alloc& other_allocator)
      : object_(other_object) {
    using RegularAllocator =
        typename std::allocator_traits<Alloc>::template rebind_alloc<
            typename SharedPtr<Y>::template ControlBlockRegular<Deleter,
                                                                Alloc>>;
    using RegularAllocatorTraits = std::allocator_traits<RegularAllocator>;
    RegularAllocator block_alloc(other_allocator);
    auto* new_block = RegularAllocatorTraits::allocate(block_alloc, 1);
    new (new_block)
        typename SharedPtr<Y>::template ControlBlockRegular<Deleter, Alloc>(
            other_object, other_deleter, other_allocator);
    block_ = new_block;
  }
  template <typename Y>
  SharedPtr(Y* other_object)
      : SharedPtr(other_object, std::default_delete<Y>(), std::allocator<Y>()) {
  }
  template <typename Y, typename Deleter = std::default_delete<Y>>
  SharedPtr(Y* other_object, const Deleter& other_deleter)
      : SharedPtr(other_object, other_deleter, std::allocator<Y>()) {
  }
  ~SharedPtr() {
    if (block_ == nullptr) {
      return;
    }
    --block_->shared_count_;
    if (block_->shared_count_ == 0) {
      block_->use_deleter();
      if (block_->weak_count_ == 0) {
        block_->use_deallocator();
      }
    }
  }
  SharedPtr& operator=(const SharedPtr& other) {
    if (this == &other) {
      return *this;
    }
    SharedPtr<T> copy(other);
    copy.swap(*this);
    return *this;
  }
  template <typename U>
  SharedPtr<T>& operator=(const SharedPtr<U>& other) {
    SharedPtr<T> copy(other);
    copy.swap(*this);
    return *this;
  }
  SharedPtr& operator=(SharedPtr&& other) {
    SharedPtr copy(std::move(other));
    copy.swap(*this);
    return *this;
  }
  template <typename U>
  SharedPtr<T>& operator=(SharedPtr<U>&& other) {
    SharedPtr<T> copy(std::move(other));
    copy.swap(*this);
    return *this;
  }
  size_t use_count() const {
    return block_->shared_count_;
  }
  T& operator*() {
    return *object_;
  }
  const T& operator*() const {
    return *object_;
  }
  T* operator->() {
    return object_;
  }
  const T* operator->() const {
    return object_;
  }
  T* get() {
    return object_;
  }
  const T* get() const {
    return object_;
  }
  void swap(SharedPtr& other) {
    std::swap(object_, other.object_);
    std::swap(block_, other.block_);
  }
  void reset() {
    *this = SharedPtr();
  }
  template <typename U>
  void reset(U* other_object) {
    *this = other_object;
  }
};

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocate_shared(const Alloc& alloc, Args&&... args) {
  using MakeSharedAllocator =
      typename std::allocator_traits<Alloc>::template rebind_alloc<
          typename SharedPtr<T>::template ControlBlockMakeShared<Alloc>>;
  using MakeSharedAllocatorTraits = std::allocator_traits<MakeSharedAllocator>;
  MakeSharedAllocator shared_alloc(alloc);
  auto* new_block = MakeSharedAllocatorTraits::allocate(shared_alloc, 1);
  try {
    MakeSharedAllocatorTraits::construct(shared_alloc, new_block, alloc,
                                         std::forward<Args>(args)...);
  } catch (...) {
    MakeSharedAllocatorTraits::deallocate(shared_alloc, new_block, 1);
    throw;
  }
  return SharedPtr<T>(*new_block->object_, new_block);
}

template <typename T, typename... Args>
SharedPtr<T> make_shared(Args&&... args) {
  return allocate_shared<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

template <typename T>
class WeakPtr {
  template <typename U>
  friend class SharedPtr;

  template <typename U>
  friend class WeakPtr;

 private:
  BaseControlBlock* block_ = nullptr;

  void swap(WeakPtr& other) {
    std::swap(block_, other.block_);
  }

 public:
  WeakPtr() = default;
  template <typename U>
  WeakPtr(const SharedPtr<U>& other)
      : block_(static_cast<BaseControlBlock*>(other.block_)) {
    if (block_ != nullptr) {
      ++block_->weak_count_;
    }
  }
  WeakPtr(const WeakPtr& other)
      : block_(other.block_) {
    if (block_ != nullptr) {
      ++block_->weak_count_;
    }
  }
  template <typename U>
  WeakPtr(const WeakPtr<U>& other)
      : block_(other.block_) {
    if (block_ != nullptr) {
      ++block_->weak_count_;
    }
  }
  WeakPtr(WeakPtr&& other)
      : block_(std::move(other.block_)) {
    other.block_ = nullptr;
  }
  template <typename U>
  WeakPtr(WeakPtr<U>&& other)
      : block_(std::move(other.block_)) {
    other.block_ = nullptr;
  }
  ~WeakPtr() {
    if (block_ == nullptr) {
      return;
    }
    --block_->weak_count_;
    if ((block_->weak_count_ == 0) && (block_->shared_count_ == 0)) {
      block_->use_deallocator();
    }
  }
  WeakPtr& operator=(const SharedPtr<T>& other) {
    WeakPtr copy(other);
    copy.swap(*this);
    return *this;
  }
  WeakPtr& operator=(const WeakPtr& other) {
    if (this == &other) {
      return *this;
    }
    WeakPtr copy(other);
    copy.swap(*this);
    return *this;
  }
  template <typename U>
  WeakPtr<T>& operator=(const WeakPtr<U> other) {
    WeakPtr<T> copy(other);
    copy.swap(*this);
    return *this;
  }
  WeakPtr& operator=(WeakPtr&& other) {
    WeakPtr copy(std::move(other));
    copy.swap(*this);
    return *this;
  }
  template <typename U>
  WeakPtr<T>& operator=(WeakPtr<U>&& other) {
    WeakPtr<T> copy(std::move(other));
    copy.swap(*this);
    return *this;
  }
  bool expired() const {
    if (block_ != nullptr) {
      return block_->shared_count_ == 0;
    } else {
      throw std::runtime_error("No block");
    }
  }
  size_t use_count() const {
    if (block_ != nullptr) {
      return block_->shared_count_;
    } else {
      throw std::runtime_error("No block");
    }
  }
  SharedPtr<T> lock() const {
    return expired() ? SharedPtr<T>() : SharedPtr(*this);
  }
};

template <typename T>
class EnableSharedFromThis {
 private:
  mutable WeakPtr<T> weak_pointer_;

 protected:
  EnableSharedFromThis() = default;
  EnableSharedFromThis(const EnableSharedFromThis&) {
  }
  EnableSharedFromThis& operator=(const EnableSharedFromThis&) {
    return *this;
  }

 public:
  SharedPtr<T> shared_from_this() {
    return SharedPtr<T>(weak_pointer_.lock());
  }
  SharedPtr<const T> shared_from_this() const {
    return SharedPtr<const T>(weak_pointer_.lock());
  }
  virtual ~EnableSharedFromThis() = default;
};