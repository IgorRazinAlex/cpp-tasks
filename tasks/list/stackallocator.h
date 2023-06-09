#include <cstdlib>
#include <iostream>
#include <memory>

template <size_t N>
struct StackStorage {
  char data[N];
  size_t shift = 0;
  StackStorage() = default;
  ~StackStorage() = default;
  StackStorage(const StackStorage& other) = delete;
  StackStorage& operator=(const StackStorage& other) = delete;
};

template <typename T, size_t N>
struct StackAllocator {
  StackStorage<N>& stack;
  using pointer = T*;
  using const_pointer = const T*;
  using void_pointer = void*;
  using const_void_pointer = const void*;
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  ~StackAllocator() = default;
  StackAllocator() = delete;
  StackAllocator(const StackAllocator& other);
  StackAllocator(StackStorage<N>& stack);
  template <typename U>
  StackAllocator(const StackAllocator<U, N>& other);
  StackAllocator<T, N>& operator=(const StackAllocator& other);
  bool operator==(const StackAllocator& other) const;
  T* allocate(size_t n);
  void deallocate(const T*, size_t){};
  template <typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };
};

template <typename T, size_t N>
StackAllocator<T, N>::StackAllocator(const StackAllocator& other)
    : stack(other.stack) {
}

template <typename T, size_t N>
StackAllocator<T, N>::StackAllocator(StackStorage<N>& stack)
    : stack(stack) {
}

template <typename T, size_t N>
template <typename U>
StackAllocator<T, N>::StackAllocator(const StackAllocator<U, N>& other)
    : stack(other.stack) {
}

template <typename T, size_t N>
StackAllocator<T, N>& StackAllocator<T, N>::operator=(
    const StackAllocator& other) {
  stack = other.stack;
  return *this;
}

template <typename T, size_t N>
bool StackAllocator<T, N>::operator==(const StackAllocator& other) const {
  return stack == other.stack;
}

template <typename T, size_t N>
T* StackAllocator<T, N>::allocate(size_t n) {
  size_t offset =
      (sizeof(T) - (size_t(stack.data) + stack.shift) % sizeof(T)) % sizeof(T);
  if (stack.shift + n * sizeof(T) + offset > N) {
    throw std::bad_alloc();
  }
  stack.shift += n * sizeof(T) + offset;
  return reinterpret_cast<T*>(stack.data + (stack.shift - n * sizeof(T)));
}

template <typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  struct Node {
    Node* previous = nullptr;
    Node* next = nullptr;
    T value;
    Node(Node* other)
        : previous(other->previous),
          next(other->next),
          value(other->value) {
    }
    Node(Node* previous, Node* next, const T& value)
        : previous(previous),
          next(next),
          value(value) {
    }
    Node(Node* previous, Node* next)
        : previous(previous),
          next(next) {
    }
  };
  using NodeAllocator =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using AllocatorTraits = std::allocator_traits<NodeAllocator>;
  [[no_unique_address]] NodeAllocator allocator_;
  size_t size_ = 0;
  Node* begin_ = nullptr;
  Node* end_ = nullptr;
  Node* construct_node_(const T& value);
  Node* construct_default_node_();
  void fill_(size_t n, const T& element);
  void fill_default_(size_t n);
  void clear_();
  void swap_(List& other);

 public:
  template <bool is_const>
  struct basic_iterator {
    Node* node;
    const List* list;
    typedef typename std::conditional<is_const, const T, T>::type value_type;
    typedef typename std::conditional<is_const, const T&, T&>::type reference;
    typedef typename std::conditional<is_const, const T*, T*>::type pointer;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    basic_iterator() = default;
    basic_iterator(Node* node, const List* list);
    basic_iterator& operator++();
    basic_iterator operator++(int);
    basic_iterator& operator--();
    basic_iterator operator--(int);
    bool operator==(const basic_iterator& other) const;
    bool operator!=(const basic_iterator& other) const;
    reference operator*() const;
    pointer operator->() const;
    operator basic_iterator<true>() const;
    basic_iterator<is_const> base() const;
  };
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  List() = default;
  List(size_t n);
  List(size_t n, const T& element);
  List(const Allocator& allocator);
  List(size_t n, const Allocator& allocator);
  List(size_t n, const T& element, const Allocator& allocator);
  List(const List& other);
  ~List();
  List& operator=(const List& other);
  Allocator get_allocator() const;
  size_t size() const;
  void push_back(const T& element);
  void pop_back();
  void push_front(const T& element);
  void pop_front();
  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;
  iterator end();
  const_iterator end() const;
  const_iterator cend() const;
  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  const_reverse_iterator crbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
  const_reverse_iterator crend() const;
  void insert(const const_iterator& iter, const T& element);
  void erase(const const_iterator& iter);
};

template <typename T, typename Allocator>
typename List<T, Allocator>::Node* List<T, Allocator>::construct_node_(
    const T& element) {
  Node* result = AllocatorTraits::allocate(allocator_, 1);
  try {
    AllocatorTraits::construct(allocator_, &(result->value), element);
  } catch (...) {
    AllocatorTraits::deallocate(allocator_, result, 1);
    throw;
  }
  result->previous = nullptr;
  result->next = nullptr;
  return result;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::Node*
List<T, Allocator>::construct_default_node_() {
  Node* result = AllocatorTraits::allocate(allocator_, 1);
  try {
    AllocatorTraits::construct(allocator_, &(result->value));
  } catch (...) {
    AllocatorTraits::deallocate(allocator_, result, 1);
    throw;
  }
  result->previous = nullptr;
  result->next = nullptr;
  return result;
}

template <typename T, typename Allocator>
void List<T, Allocator>::fill_(size_t n, const T& element) {
  try {
    for (size_t i = 0; i < n; ++i) {
      Node* node = construct_node_(element);
      if (begin_ == nullptr) {
        begin_ = node;
        end_ = node;
      } else if (begin_->next == nullptr) {
        begin_->next = node;
        node->previous = begin_;
        end_ = node;
      } else {
        node->previous = end_;
        end_->next = node;
        end_ = node;
      }
      ++size_;
    }
  } catch (...) {
    clear_();
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::fill_default_(size_t n) {
  try {
    for (size_t i = 0; i < n; ++i) {
      Node* node = construct_default_node_();
      if (begin_ == nullptr) {
        begin_ = node;
        end_ = node;
      } else if (begin_->next == nullptr) {
        begin_->next = node;
        node->previous = begin_;
        end_ = node;
      } else {
        node->previous = end_;
        end_->next = node;
        end_ = node;
      }
      ++size_;
    }
  } catch (...) {
    clear_();
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::clear_() {
  Node* temporary = begin_;
  while (temporary != nullptr) {
    std::swap(begin_, temporary->next);
    AllocatorTraits::destroy(allocator_, temporary);
    AllocatorTraits::deallocate(allocator_, temporary, 1);
    temporary = begin_;
  }
  end_ = nullptr;
  size_ = 0;
}

template <typename T, typename Allocator>
void List<T, Allocator>::swap_(List& other) {
  std::swap(size_, other.size_);
  std::swap(begin_, other.begin_);
  std::swap(end_, other.end_);
}

template <typename T, typename Allocator>
template <bool is_const>
List<T, Allocator>::basic_iterator<is_const>::basic_iterator(Node* node,
                                                             const List* list)
    : node(node),
      list(list) {
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template basic_iterator<is_const>&
List<T, Allocator>::basic_iterator<is_const>::operator++() {
  if (node != nullptr) {
    node = node->next;
  } else {
    node = list->begin_;
  }
  return *this;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template basic_iterator<is_const>
List<T, Allocator>::basic_iterator<is_const>::operator++(int) {
  basic_iterator copy = basic_iterator(*this);
  if (node != nullptr) {
    node = node->next;
  } else {
    node = list->begin_;
  }
  return copy;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template basic_iterator<is_const>&
List<T, Allocator>::basic_iterator<is_const>::operator--() {
  if (node != nullptr) {
    node = node->previous;
  } else {
    node = list->end_;
  }
  return *this;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template basic_iterator<is_const>
List<T, Allocator>::basic_iterator<is_const>::operator--(int) {
  basic_iterator copy = basic_iterator(*this);
  if (node != nullptr) {
    node = node->previous;
  } else {
    node = list->end_;
  }
  return copy;
}

template <typename T, typename Allocator>
template <bool is_const>
bool List<T, Allocator>::basic_iterator<is_const>::operator==(
    const basic_iterator& other) const {
  return node == other.node;
}

template <typename T, typename Allocator>
template <bool is_const>
bool List<T, Allocator>::basic_iterator<is_const>::operator!=(
    const basic_iterator& other) const {
  return node != other.node;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template basic_iterator<is_const>::reference
List<T, Allocator>::basic_iterator<is_const>::operator*() const {
  return node->value;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template basic_iterator<is_const>::pointer
List<T, Allocator>::basic_iterator<is_const>::operator->() const {
  return &(node->value);
}

template <typename T, typename Allocator>
template <bool is_const>
List<T, Allocator>::basic_iterator<is_const>::operator List<
    T, Allocator>::basic_iterator<true>() const {
  return basic_iterator<true>(node, list);
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template basic_iterator<is_const>
List<T, Allocator>::basic_iterator<is_const>::base() const {
  return basic_iterator<is_const>(node->next, list);
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t n) {
  fill_default_(n);
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t n, const T& element) {
  fill_(n, element);
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const Allocator& allocator)
    : allocator_(allocator) {
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t n, const Allocator& allocator)
    : allocator_(allocator) {
  fill_default_(n);
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t n, const T& element, const Allocator& allocator)
    : allocator_(allocator) {
  fill_(n, element);
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : allocator_(AllocatorTraits::select_on_container_copy_construction(
          other.allocator_)) {
  if (other.size_ == 0) {
    return;
  }
  try {
    Node* cur_other = other.begin_;
    for (size_t i = 0; i < other.size_; ++i) {
      Node* node = construct_node_(cur_other->value);
      if (begin_ == nullptr) {
        begin_ = node;
        end_ = node;
      } else if (begin_->next == nullptr) {
        begin_->next = node;
        node->previous = begin_;
        end_ = node;
      } else {
        node->previous = end_;
        end_->next = node;
        end_ = node;
      }
      ++size_;
      cur_other = cur_other->next;
    }
  } catch (...) {
    clear_();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::~List() {
  clear_();
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
  if (this == &other) {
    return *this;
  }
  if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::
                    value) {
    allocator_ = other.allocator_;
  }
  List temporary(other);
  swap_(temporary);
  return *this;
}

template <typename T, typename Allocator>
Allocator List<T, Allocator>::get_allocator() const {
  return allocator_;
}

template <typename T, typename Allocator>
size_t List<T, Allocator>::size() const {
  return size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& element) {
  insert({nullptr, this}, element);
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  erase({end_, this});
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& element) {
  insert({begin_, this}, element);
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  erase({begin_, this});
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin() {
  return iterator(begin_, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin() const {
  return const_iterator(begin_, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const {
  return const_iterator(begin_, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end() {
  return iterator(end_->next, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
  return const_iterator(end_->next, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
  return const_iterator(end_->next, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
  return reverse_iterator(iterator(end_->next, this));
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin()
    const {
  return crbegin();
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::crbegin() const {
  return const_reverse_iterator(const_iterator(end_->next, this));
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rend() {
  return reverse_iterator(iterator(begin_, this));
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rend()
    const {
  return crend();
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend()
    const {
  return const_reverse_iterator(const_iterator(begin_, this));
}

template <typename T, typename Allocator>
void List<T, Allocator>::insert(const const_iterator& iter, const T& element) {
  Node* node = construct_node_(element);
  Node* place = iter.node;
  if (place == nullptr) {
    if (end_ == nullptr) {
      begin_ = node;
      end_ = node;
    } else {
      end_->next = node;
      node->previous = end_;
      end_ = node;
    }
  } else {
    node->next = place;
    node->previous = place->previous;
    if (place->previous != nullptr) {
      place->previous->next = node;
    } else {
      begin_ = node;
    }
    place->previous = node;
  }
  ++size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::erase(const const_iterator& iter) {
  if (iter.node->previous != nullptr) {
    iter.node->previous->next = iter.node->next;
  } else {
    begin_ = iter.node->next;
  }
  if (iter.node->next != nullptr) {
    iter.node->next->previous = iter.node->previous;
  } else {
    end_ = iter.node->previous;
  }
  AllocatorTraits::destroy(allocator_, iter.node);
  AllocatorTraits::deallocate(allocator_, iter.node, 1);
  --size_;
}