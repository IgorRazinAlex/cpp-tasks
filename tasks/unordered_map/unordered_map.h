#include <iostream>
#include <vector>

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
  template <typename... Args>
  Node* construct_move_node_(Args&&... args);
  Node* construct_node_(const T& value);
  Node* construct_default_node_();
  void fill_(size_t n, const T& element);
  void fill_default_(size_t n);
  void clear_();
  void swap_(List& other);
  void swap_(List&& other);

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
  List(List&& other);
  ~List();
  List& operator=(const List& other);
  List& operator=(List&& other);
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
  template <typename... Args>
  void emplace_back(Args&&... args);
};

template <typename T, typename Allocator>
template <typename... Args>
typename List<T, Allocator>::Node* List<T, Allocator>::construct_move_node_(
    Args&&... args) {
  Node* result = AllocatorTraits::allocate(allocator_, 1);
  try {
    AllocatorTraits::construct(allocator_, &result->value,
                               std::forward<Args>(args)...);
  } catch (...) {
    AllocatorTraits::deallocate(allocator_, result, 1);
    throw;
  }
  result->previous = nullptr;
  result->next = nullptr;
  return result;
}

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
void List<T, Allocator>::swap_(List&& other) {
  auto copy = std::move(other);
  swap_(copy);
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
List<T, Allocator>::List(List&& other)
    : allocator_(other.allocator_),
      size_(other.size_),
      begin_(other.begin_),
      end_(other.end_) {
  other.size_ = 0;
  other.begin_ = nullptr;
  other.end_ = nullptr;
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
List<T, Allocator>& List<T, Allocator>::operator=(List&& other) {
  if (this == &other) {
    return *this;
  }
  swap_(std::move(other));
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
  return iterator(nullptr, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
  return const_iterator(nullptr, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
  return const_iterator(nullptr, this);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
  return reverse_iterator(iterator(nullptr, this));
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin()
    const {
  return crbegin();
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::crbegin() const {
  return const_reverse_iterator(const_iterator(nullptr, this));
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

template <typename T, typename Allocator>
template <typename... Args>
void List<T, Allocator>::emplace_back(Args&&... args) {
  Node* node = construct_move_node_(std::forward<Args>(args)...);
  if (end_ == nullptr) {
    end_ = node;
    begin_ = node;
  } else {
    end_->next = node;

    node->previous = end_;
    end_ = node;
  }
  ++size_;
}

template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
 public:
  using NodeType = std::pair<const Key, Value>;
  using iterator = typename List<NodeType, Alloc>::iterator;
  using const_iterator = typename List<NodeType, Alloc>::const_iterator;
  using reverse_iterator = typename List<NodeType, Alloc>::reverse_iterator;
  using const_reverse_iterator =
      typename List<NodeType, Alloc>::const_reverse_iterator;

 private:
  using AllocIterator =
      typename std::allocator_traits<Alloc>::template rebind_alloc<iterator>;
  using AllocList = typename std::allocator_traits<
      Alloc>::template rebind_alloc<List<iterator, AllocIterator>>;
  Equal equal_ = Equal();
  Hash hash_ = Hash();
  List<NodeType, Alloc> data_;
  [[no_unique_address]] Alloc regular_allocator_;
  std::vector<List<iterator, AllocIterator>, AllocList> hash_table_;
  float max_load_factor_ = 1.0;

  size_t get_bucket_(const Key& key) const;
  void rehash_(bool force = false);
  void swap_(UnorderedMap& other);

 public:
  UnorderedMap();
  UnorderedMap(const UnorderedMap& other);
  UnorderedMap(UnorderedMap&& other);
  ~UnorderedMap();
  UnorderedMap& operator=(const UnorderedMap& other);
  UnorderedMap& operator=(UnorderedMap&& other);
  size_t size() const;
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
  iterator find(const Key& key);
  const_iterator find(const Key& key) const;
  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args);
  std::pair<iterator, bool> insert(const NodeType& element);
  std::pair<iterator, bool> insert(NodeType&& element);
  template <typename Pair>
  std::pair<iterator, bool> insert(Pair&& value);
  template <typename InputIterator>
  void insert(InputIterator start, InputIterator finish);
  Value& at(const Key& key);
  const Value& at(const Key& key) const;
  Value& operator[](const Key& key);
  Value& operator[](Key&& key);
  void erase(const_iterator iter);
  void erase(const_iterator start, const_iterator finish);
  void reserve(size_t new_size);
  float load_factor();
  float max_load_factor() const;
  void max_load_factor(float new_load_factor);
};

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::get_bucket_(
    const Key& key) const {
  return hash_(key) % hash_table_.size();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::rehash_(bool force) {
  if ((hash_table_.size() >= max_load_factor_ * data_.size()) && (!force)) {
    return;
  }
  try {
    std::vector<List<iterator, AllocIterator>, AllocList> new_hash_table(
        static_cast<int>(2 * data_.size() * max_load_factor_) + 1);
    for (auto it = data_.begin(); it != data_.end(); ++it) {
      new_hash_table[hash_(it->first) % new_hash_table.size()].push_back(it);
    }
    hash_table_ = new_hash_table;
  } catch (...) {
    throw;
  }
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::swap_(UnorderedMap& other) {
  std::swap(data_, other.data_);
  std::swap(hash_table_, other.hash_table_);
  std::swap(max_load_factor_, other.max_load_factor_);
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap() = default;

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(
    const UnorderedMap& other)
    : data_(other.data_),
      hash_table_(other.hash_table_),
      max_load_factor_(other.max_load_factor_) {
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(UnorderedMap&& other)
    : data_(std::move(other.data_)),
      hash_table_(std::move(other.hash_table_)),
      max_load_factor_(other.max_load_factor_) {
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::~UnorderedMap() = default;

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(
    const UnorderedMap& other) {
  if (this == &other) {
    return *this;
  }
  UnorderedMap copy(other);
  swap_(copy);
  return *this;
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(UnorderedMap&& other) {
  if (this == &other) {
    return *this;
  }
  data_ = std::move(other.data_);
  hash_table_ = std::move(other.hash_table_);
  max_load_factor_ = other.max_load_factor_;
  return *this;
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::size() const {
  return data_.size();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin() {
  return data_.begin();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin() const {
  return data_.cbegin();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::cbegin() const {
  return data_.cbegin();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::end() {
  return data_.end();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::end() const {
  return data_.cend();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::cend() const {
  return data_.cend();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::reverse_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::rbegin() {
  return data_.rbegin();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::rbegin() const {
  return data_.crbegin();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::crbegin() const {
  return data_.crbegin();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::reverse_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::rend() {
  return data_.rend();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::rend() const {
  return data_.crend();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::crend() const {
  return data_.crend();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) {
  if (hash_table_.empty()) {
    return end();
  }
  for (auto iter : hash_table_[get_bucket_(key)]) {
    if (equal_(iter->first, key)) {
      return iter;
    }
  }
  return end();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) const {
  if (hash_table_.empty()) {
    return cend();
  }
  for (auto iter : hash_table_[get_bucket_(key)]) {
    if (Equal(iter->first, key)) {
      return iter;
    }
  }
  return cend();
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
template <typename... Args>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::emplace(Args&&... args) {
  data_.emplace_back(std::forward<Args>(args)...);
  iterator new_element = --end();
  iterator in_table = find(new_element->first);
  if (in_table != end()) {
    data_.pop_back();
    return {in_table, false};
  }
  if (hash_table_.empty()) {
    hash_table_.resize(1);
    rehash_(true);
  }
  hash_table_[get_bucket_(new_element->first)].push_back(new_element);
  rehash_();
  return {new_element, true};
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(const NodeType& element) {
  return emplace(element);
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(NodeType&& element) {
  return emplace(std::forward<NodeType>(element));
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
template <typename Pair>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(Pair&& value) {
  return emplace(std::forward<Pair>(value));
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
template <typename InputIterator>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(
    InputIterator start, InputIterator finish) {
  for (auto it = start; it != finish; ++it) {
    insert(*it);
  }
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key& key) {
  iterator it = find(key);
  if (it == end()) {
    throw("ERROR: Key not found");
  }
  return it->second;
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
const Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(
    const Key& key) const {
  const_iterator it = find(key);
  if (it == cend()) {
    throw("ERROR: Key not found");
  }
  return it->second;
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](
    const Key& key) {
  iterator it = find(key);
  if (it == end()) {
    it = emplace(NodeType(key, Value())).first;
  }
  return it->second;
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](Key&& key) {
  iterator it = find(key);
  if (it == end()) {
    it = emplace(NodeType(std::move(key), Value())).first;
  }
  return it->second;
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(const_iterator iter) {
  size_t hash = get_bucket_(iter->first);
  for (auto x = hash_table_[hash].begin(); x != hash_table_[hash].end(); ++x) {
    if (equal_(iter->first, (*x)->first)) {
      hash_table_[hash].erase(x);
      break;
    }
  }
  data_.erase(iter);
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(
    const_iterator start, const_iterator finish) {
  for (auto iter = start; iter != finish; ++iter) {
    size_t hash = get_bucket_(iter->first);
    for (auto x = hash_table_[hash].begin(); x != hash_table_[hash].end();
         ++x) {
      if (equal_(iter->first, (*x)->first)) {
        hash_table_[hash].erase(x);
        break;
      }
    }
    auto next = ++iter;
    --iter;
    data_.erase(iter);
    iter = --next;
  }
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::reserve(size_t new_size) {
  if (hash_table_.size() < new_size) {
    hash_table_.resize(new_size);
    rehash_(true);
  }
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::load_factor() {
  if (hash_table_.empty()) {
    hash_table_.resize(1);
    rehash_();
  }
  return static_cast<float>(data_.size()) /
         static_cast<float>(hash_table_.size());
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_load_factor() const {
  return max_load_factor_;
}

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_load_factor(
    float new_load_factor) {
  max_load_factor_ = new_load_factor;
  rehash_();
}