#include <iostream>
#include <type_traits>
#include <typeinfo>

const size_t CHUNK_SIZE = 32;

template <typename T>
class Deque {
 private:
  T** array_ = nullptr;
  size_t size_ = 0;
  size_t start_ = 0;
  size_t chunk_count_ = 0;
  void append_memory_();
  void appfront_memory_();

 public:
  template <bool is_const, bool is_reversed>
  struct basic_iterator {
    const Deque<T>* deque_ = nullptr;
    long long index_ = 0;
    typedef typename std::conditional<is_const, const T, T>::type value_type;
    typedef typename std::conditional<is_const, const T&, T&>::type reference;
    typedef typename std::conditional<is_const, const T*, T*>::type pointer;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = long long;
    basic_iterator() = default;
    basic_iterator(const Deque<T>* deque, long long index);
    basic_iterator& operator+=(long long step);
    basic_iterator& operator-=(long long step);
    basic_iterator operator+(long long step) const;
    basic_iterator operator-(long long step) const;
    long long operator-(const basic_iterator& other) const;
    basic_iterator& operator++();
    basic_iterator operator++(int);
    basic_iterator& operator--();
    basic_iterator operator--(int);
    bool operator==(const basic_iterator& other) const;
    bool operator!=(const basic_iterator& other) const;
    bool operator<(const basic_iterator& other) const;
    bool operator>=(const basic_iterator& other) const;
    bool operator>(const basic_iterator& other) const;
    bool operator<=(const basic_iterator& other) const;
    reference operator*() const;
    pointer operator->() const;
    operator basic_iterator<true, false>() const;
    operator basic_iterator<true, true>() const;
  };
  using iterator = basic_iterator<false, false>;
  using const_iterator = basic_iterator<true, false>;
  using reverse_iterator = basic_iterator<false, true>;
  using const_reverse_iterator = basic_iterator<true, true>;
  ~Deque();
  Deque() = default;
  Deque(const Deque<T>& other);
  Deque(size_t count);
  Deque(size_t count, const T& element);
  Deque& operator=(const Deque<T>& other);
  size_t size() const;
  T& operator[](size_t index);
  const T& operator[](size_t index) const;
  T& at(size_t index);
  const T& at(size_t index) const;
  void push_back(const T& element);
  void pop_back();
  void push_front(const T& element);
  void pop_front();
  Deque<T>::iterator begin();
  Deque<T>::const_iterator begin() const;
  Deque<T>::const_iterator cbegin() const;
  Deque<T>::iterator end();
  Deque<T>::const_iterator end() const;
  Deque<T>::const_iterator cend() const;
  Deque<T>::reverse_iterator rbegin();
  Deque<T>::const_reverse_iterator rbegin() const;
  Deque<T>::const_reverse_iterator crbegin() const;
  Deque<T>::reverse_iterator rend();
  Deque<T>::const_reverse_iterator rend() const;
  Deque<T>::const_reverse_iterator crend() const;
  void insert(const Deque<T>::iterator& iter, const T& element);
  void erase(const Deque<T>::iterator& iter);
};

template <typename T>
void Deque<T>::append_memory_() {
  T** new_array = nullptr;
  size_t index = chunk_count_;
  size_t new_chunk_count = 2 * chunk_count_ + 1;
  try {
    new_array = reinterpret_cast<T**>(new char*[new_chunk_count]);
    for (; index < new_chunk_count; ++index) {
      new_array[index] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
    }
  } catch (...) {
    for (size_t i = chunk_count_; i < index; ++i) {
      delete[] reinterpret_cast<char*>(new_array[i]);
    }
    delete[] reinterpret_cast<char**>(new_array);
    throw;
  }
  for (size_t i = 0; i < chunk_count_; ++i) {
    new_array[i] = array_[i];
  }
  chunk_count_ = new_chunk_count;
  delete[] array_;
  array_ = new_array;
}

template <typename T>
void Deque<T>::appfront_memory_() {
  T** new_array = nullptr;
  size_t index = 0;
  size_t new_chunk_count = 2 * chunk_count_ + 1;
  try {
    new_array = reinterpret_cast<T**>(new char*[new_chunk_count]);
    for (; index <= chunk_count_; ++index) {
      new_array[index] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
    }
  } catch (...) {
    for (size_t i = 0; i < index; ++i) {
      delete[] reinterpret_cast<char*>(new_array[i]);
    }
    delete[] reinterpret_cast<char**>(new_array);
    throw;
  }
  for (size_t i = 0; i < chunk_count_; ++i) {
    new_array[i + chunk_count_ + 1] = array_[i];
  }
  start_ += (chunk_count_ + 1) * CHUNK_SIZE;
  chunk_count_ = new_chunk_count;
  delete[] array_;
  array_ = new_array;
}

template <typename T>
Deque<T>::~Deque() {
  for (size_t i = 0; i < size_; ++i) {
    (*this)[i].~T();
  }
  for (size_t i = 0; i < chunk_count_; ++i) {
    delete[] reinterpret_cast<char*>(array_[i]);
  }
  delete[] reinterpret_cast<char**>(array_);
}

template <typename T>
Deque<T>::Deque(const Deque<T>& other) {
  *this = other;
}

template <typename T>
Deque<T>::Deque(size_t count)
    : Deque(count, T()) {
}

template <typename T>
Deque<T>::Deque(size_t count, const T& element)
    : size_(count),
      start_(0),
      chunk_count_((count + CHUNK_SIZE - 1) / CHUNK_SIZE) {
  size_t chunk = 0;
  size_t index = 0;
  try {
    array_ = reinterpret_cast<T**>(new char*[chunk_count_]);
    for (; chunk < chunk_count_; ++chunk) {
      array_[chunk] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
    }
    for (; index < size_; ++index) {
      new (array_[index / CHUNK_SIZE] + (index % CHUNK_SIZE)) T(element);
    }
  } catch (...) {
    for (size_t i = 0; i < index; ++i) {
      (array_[i / CHUNK_SIZE] + (i % CHUNK_SIZE))->~T();
    }
    for (size_t i = 0; i < chunk; ++i) {
      delete[] reinterpret_cast<char*>(array_[i]);
    }
    delete[] reinterpret_cast<char**>(array_);
    array_ = nullptr;
    size_ = 0;
    start_ = 0;
    chunk_count_ = 0;
    throw;
  }
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque<T>& other) {
  size_t chunk = 0;
  size_t index = 0;
  try {
    size_ = other.size_;
    start_ = other.start_;
    chunk_count_ = other.chunk_count_;
    array_ = reinterpret_cast<T**>(new char*[chunk_count_]);
    for (; chunk < chunk_count_; ++chunk) {
      array_[chunk] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
    }
    for (; index < size_; ++index) {
      new (array_[index / CHUNK_SIZE] + (index % CHUNK_SIZE)) T(other[index]);
    }
  } catch (...) {
    for (size_t i = 0; i < index; ++i) {
      (array_[i / CHUNK_SIZE] + (i % CHUNK_SIZE))->~T();
    }
    for (size_t i = 0; i < chunk; ++i) {
      delete[] reinterpret_cast<char*>(array_[i]);
    }
    delete[] reinterpret_cast<char**>(array_);
    array_ = nullptr;
    size_ = 0;
    start_ = 0;
    chunk_count_ = 0;
    throw;
  }
  return *this;
}

template <typename T>
size_t Deque<T>::size() const {
  return size_;
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
  return array_[(start_ + index) / CHUNK_SIZE][(start_ + index) % CHUNK_SIZE];
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
  return array_[(start_ + index) / CHUNK_SIZE][(start_ + index) % CHUNK_SIZE];
}

template <typename T>
T& Deque<T>::at(size_t index) {
  if (index >= size_) {
    throw std::out_of_range("Deque error: index out of range");
  }
  return (*this)[index];
}

template <typename T>
const T& Deque<T>::at(size_t index) const {
  if (index >= size_) {
    throw std::out_of_range("Deque error: index out of range");
  }
  return (*this)[index];
}

template <typename T>
void Deque<T>::push_back(const T& element) {
  if (start_ + size_ == chunk_count_ * CHUNK_SIZE) {
    append_memory_();
  }
  new (&((*this)[size_])) T(element);
  ++size_;
}

template <typename T>
void Deque<T>::pop_back() {
  (*this)[size_].~T();
  --size_;
}

template <typename T>
void Deque<T>::push_front(const T& element) {
  if (start_ == 0) {
    appfront_memory_();
  }
  new (&(array_[(start_ - 1) / CHUNK_SIZE][(start_ - 1) % CHUNK_SIZE]))
      T(element);
  --start_;
  ++size_;
}

template <typename T>
void Deque<T>::pop_front() {
  (*this)[0].~T();
  ++start_;
  --size_;
}

template <typename T>
template <bool is_const, bool is_reversed>
Deque<T>::basic_iterator<is_const, is_reversed>::basic_iterator(
    const Deque* deque, long long index)
    : deque_(deque),
      index_(index) {
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>&
Deque<T>::basic_iterator<is_const, is_reversed>::operator+=(long long step) {
  if (!is_reversed) {
    index_ += step;
  } else {
    index_ -= step;
  }
  return *this;
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>&
Deque<T>::basic_iterator<is_const, is_reversed>::operator-=(long long step) {
  if (!is_reversed) {
    index_ -= step;
  } else {
    index_ += step;
  }
  return *this;
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>
Deque<T>::basic_iterator<is_const, is_reversed>::operator+(
    long long step) const {
  if (!is_reversed) {
    return basic_iterator<is_const, is_reversed>(deque_, index_ + step);
  } else {
    return basic_iterator<is_const, is_reversed>(deque_, index_ - step);
  }
};

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>
Deque<T>::basic_iterator<is_const, is_reversed>::operator-(
    long long step) const {
  if (!is_reversed) {
    return basic_iterator<is_const, is_reversed>(deque_, index_ - step);
  } else {
    return basic_iterator<is_const, is_reversed>(deque_, index_ + step);
  }
}

template <typename T>
template <bool is_const, bool is_reversed>
long long Deque<T>::basic_iterator<is_const, is_reversed>::operator-(
    const basic_iterator& other) const {
  if (!is_reversed) {
    return index_ - other.index_;
  } else {
    return other.index_ - index_;
  }
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>&
Deque<T>::basic_iterator<is_const, is_reversed>::operator++() {
  if (!is_reversed) {
    ++index_;
  } else {
    --index_;
  }
  return *this;
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>
Deque<T>::basic_iterator<is_const, is_reversed>::operator++(int) {
  if (!is_reversed) {
    ++index_;
  } else {
    --index_;
  }
  return *this - 1;
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>&
Deque<T>::basic_iterator<is_const, is_reversed>::operator--() {
  if (!is_reversed) {
    --index_;
  } else {
    ++index_;
  }
  return *this;
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>
Deque<T>::basic_iterator<is_const, is_reversed>::operator--(int) {
  if (!is_reversed) {
    --index_;
  } else {
    ++index_;
  }
  return *this + 1;
}

template <typename T>
template <bool is_const, bool is_reversed>
bool Deque<T>::basic_iterator<is_const, is_reversed>::operator==(
    const basic_iterator<is_const, is_reversed>& other) const {
  return index_ == other.index_;
}

template <typename T>
template <bool is_const, bool is_reversed>
bool Deque<T>::basic_iterator<is_const, is_reversed>::operator!=(
    const basic_iterator<is_const, is_reversed>& other) const {
  return index_ != other.index_;
}

template <typename T>
template <bool is_const, bool is_reversed>
bool Deque<T>::basic_iterator<is_const, is_reversed>::operator<(
    const basic_iterator<is_const, is_reversed>& other) const {
  if (!is_reversed) {
    return index_ < other.index_;
  }
  return index_ > other.index_;
}

template <typename T>
template <bool is_const, bool is_reversed>
bool Deque<T>::basic_iterator<is_const, is_reversed>::operator>=(
    const basic_iterator<is_const, is_reversed>& other) const {
  return !(*this < other);
}

template <typename T>
template <bool is_const, bool is_reversed>
bool Deque<T>::basic_iterator<is_const, is_reversed>::operator>(
    const basic_iterator<is_const, is_reversed>& other) const {
  return other < *this;
}

template <typename T>
template <bool is_const, bool is_reversed>
bool Deque<T>::basic_iterator<is_const, is_reversed>::operator<=(
    const basic_iterator<is_const, is_reversed>& other) const {
  return !(*this > other);
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>::reference
Deque<T>::basic_iterator<is_const, is_reversed>::operator*() const {
  return deque_->array_[index_ / CHUNK_SIZE][index_ % CHUNK_SIZE];
}

template <typename T>
template <bool is_const, bool is_reversed>
typename Deque<T>::template basic_iterator<is_const, is_reversed>::pointer
Deque<T>::basic_iterator<is_const, is_reversed>::operator->() const {
  return &(deque_->array_[index_ / CHUNK_SIZE][index_ % CHUNK_SIZE]);
}

template <typename T>
template <bool is_const, bool is_reversed>
Deque<T>::basic_iterator<is_const, is_reversed>::operator Deque<
    T>::basic_iterator<true, false>() const {
  return basic_iterator<true, false>(deque_, index_);
}

template <typename T>
template <bool is_const, bool is_reversed>
Deque<T>::basic_iterator<is_const, is_reversed>::operator Deque<
    T>::basic_iterator<true, true>() const {
  return basic_iterator<true, true>(deque_, index_);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
  return {this, static_cast<long long>(start_)};
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
  return cbegin();
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
  return {this, static_cast<long long>(start_)};
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
  return {this, static_cast<long long>(start_ + size_)};
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
  return cend();
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
  return {this, static_cast<long long>(start_ + size_)};
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
  return {this, static_cast<long long>(start_ + size_) - 1};
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const {
  return crbegin();
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
  return {this, static_cast<long long>(start_ + size_) - 1};
};

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
  return {this, static_cast<long long>(start_) - 1};
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const {
  return crend();
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
  return {this, static_cast<long long>(start_) - 1};
  ;
}

template <typename T>
void Deque<T>::insert(const Deque<T>::iterator& iter, const T& element) {
  if (size_ == 0) {
    push_back(element);
    return;
  }
  push_back((*this)[size_ - 1]);
  for (size_t i = size_ - 2; i > iter.index_ - start_; --i) {
    (*this)[i].~T();
    new (&((*this)[i])) T((*this)[i - 1]);
  }
  (*this)[iter.index_ - start_].~T();
  new (&((*this)[iter.index_ - start_])) T(element);
}

template <typename T>
void Deque<T>::erase(const Deque<T>::iterator& iter) {
  for (size_t i = iter.index_ - start_; i < size_ - 1; ++i) {
    (*this)[i].~T();
    new (&((*this)[i])) T((*this)[i + 1]);
  }
  pop_back();
}