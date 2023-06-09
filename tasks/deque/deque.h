#include <iostream>
#include <type_traits>
#include <typeinfo>

template <typename T>
class Deque {
 private:
  static const size_t chunk_size = 32;
  T** array_ = nullptr;
  size_t size_ = 0;
  size_t start_ = 0;
  size_t chunk_count_ = 0;
  void append_memory();
  void appfront_memory();
  void swap(Deque<T> other);

 public:
  template <bool is_const>
  struct basic_iterator {
   private:
    const T* const* array_;
    size_t start_;
    basic_iterator(const T* const* array, size_t start, int64_t index);

   public:
    int64_t index_ = 0;
    using value_type = typename std::conditional<is_const, const T, T>::type;
    using reference = typename std::conditional<is_const, const T&, T&>::type;
    using pointer = typename std::conditional<is_const, const T*, T*>::type;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = int64_t;
    basic_iterator(const Deque<T>* deque, int64_t index);
    basic_iterator& operator+=(int64_t step);
    basic_iterator& operator-=(int64_t step);
    basic_iterator operator+(int64_t step) const;
    basic_iterator operator-(int64_t step) const;
    int64_t operator-(const basic_iterator& other) const;
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
    operator basic_iterator<true>() const;
  };
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
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
void Deque<T>::append_memory() {
  T** new_array = nullptr;
  size_t index = chunk_count_;
  size_t new_chunk_count = 2 * chunk_count_ + 1;
  try {
    new_array = reinterpret_cast<T**>(new char*[new_chunk_count]);
    for (; index < new_chunk_count; ++index) {
      new_array[index] = reinterpret_cast<T*>(new char[chunk_size * sizeof(T)]);
    }
  } catch (...) {
    for (size_t i = chunk_count_; i < index; ++i) {
      delete[] reinterpret_cast<char*>(new_array[i]);
    }
    delete[] reinterpret_cast<char**>(new_array);
    throw;
  }
  std::copy(array_, array_ + chunk_count_, new_array);
  chunk_count_ = new_chunk_count;
  delete[] array_;
  array_ = new_array;
}

template <typename T>
void Deque<T>::appfront_memory() {
  T** new_array = nullptr;
  size_t index = 0;
  size_t new_chunk_count = 2 * chunk_count_ + 1;
  try {
    new_array = reinterpret_cast<T**>(new char*[new_chunk_count]);
    for (; index <= chunk_count_; ++index) {
      new_array[index] = reinterpret_cast<T*>(new char[chunk_size * sizeof(T)]);
    }
  } catch (...) {
    for (size_t i = 0; i < index; ++i) {
      delete[] reinterpret_cast<char*>(new_array[i]);
    }
    delete[] reinterpret_cast<char**>(new_array);
    throw;
  }
  std::copy(array_, array_ + chunk_count_, new_array + chunk_count_ + 1);
  start_ += (chunk_count_ + 1) * chunk_size;
  chunk_count_ = new_chunk_count;
  delete[] array_;
  array_ = new_array;
}

template <typename T>
void Deque<T>::swap(Deque<T> other) {
  std::swap(array_, other.array_);
  std::swap(size_, other.size_);
  std::swap(start_, other.start_);
  std::swap(chunk_count_, other.chunk_count_);
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
  size_t chunk = 0;
  size_t index = 0;
  try {
    size_ = other.size_;
    start_ = other.start_;
    chunk_count_ = other.chunk_count_;
    array_ = reinterpret_cast<T**>(new char*[chunk_count_]);
    for (; chunk < chunk_count_; ++chunk) {
      array_[chunk] = reinterpret_cast<T*>(new char[chunk_size * sizeof(T)]);
    }
    for (; index < size_; ++index) {
      new (array_[index / chunk_size] + (index % chunk_size)) T(other[index]);
    }
  } catch (...) {
    for (size_t i = 0; i < index; ++i) {
      (array_[i / chunk_size] + (i % chunk_size))->~T();
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
Deque<T>::Deque(size_t count)
    : size_(count),
      start_(0),
      chunk_count_((count + chunk_size - 1) / chunk_size) {
  size_t chunk = 0;
  size_t index = 0;
  try {
    array_ = reinterpret_cast<T**>(new char*[chunk_count_]);
    for (; chunk < chunk_count_; ++chunk) {
      array_[chunk] = reinterpret_cast<T*>(new char[chunk_size * sizeof(T)]);
    }
    for (; index < size_; ++index) {
      new (array_[index / chunk_size] + (index % chunk_size)) T();
    }
  } catch (...) {
    for (size_t i = 0; i < index; ++i) {
      (array_[i / chunk_size] + (i % chunk_size))->~T();
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
Deque<T>::Deque(size_t count, const T& element)
    : size_(count),
      start_(0),
      chunk_count_((count + chunk_size - 1) / chunk_size) {
  size_t chunk = 0;
  size_t index = 0;
  try {
    array_ = reinterpret_cast<T**>(new char*[chunk_count_]);
    for (; chunk < chunk_count_; ++chunk) {
      array_[chunk] = reinterpret_cast<T*>(new char[chunk_size * sizeof(T)]);
    }
    for (; index < size_; ++index) {
      new (array_[index / chunk_size] + (index % chunk_size)) T(element);
    }
  } catch (std::bad_alloc) {
    delete[] reinterpret_cast<char**>(array_);
    array_ = nullptr;
    size_ = 0;
    start_ = 0;
    chunk_count_ = 0;
    throw;
  } catch (std::runtime_error) {
    for (size_t i = 0; i < index; ++i) {
      (array_[i / chunk_size] + (i % chunk_size))->~T();
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
  if (this == &other) {
    return *this;
  }
  Deque<T> copy(other);
  swap(copy);
  return *this;
}

template <typename T>
size_t Deque<T>::size() const {
  return size_;
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
  return array_[(start_ + index) / chunk_size][(start_ + index) % chunk_size];
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
  return array_[(start_ + index) / chunk_size][(start_ + index) % chunk_size];
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
  if (start_ + size_ == chunk_count_ * chunk_size) {
    append_memory();
  }
  new (array_[(start_ + size_) / chunk_size] + ((start_ + size_) % chunk_size))
      T(element);
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
    appfront_memory();
  }
  new (array_[(start_ - 1) / chunk_size] + ((start_ - 1) % chunk_size))
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
template <bool is_const>
Deque<T>::basic_iterator<is_const>::basic_iterator(const T* const* array,
                                                   size_t start, int64_t index)
    : array_(array),
      start_(start),
      index_(index) {
}

template <typename T>
template <bool is_const>
Deque<T>::basic_iterator<is_const>::basic_iterator(const Deque* deque,
                                                   int64_t index)
    : array_(deque->array_),
      start_(deque->start_),
      index_(index) {
}

// TODO
template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>&
Deque<T>::basic_iterator<is_const>::operator+=(int64_t step) {
  index_ += step;
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>&
Deque<T>::basic_iterator<is_const>::operator-=(int64_t step) {
  index_ -= step;
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>
Deque<T>::basic_iterator<is_const>::operator+(int64_t step) const {
  return basic_iterator<is_const>(array_, start_, index_ + step);
};

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>
Deque<T>::basic_iterator<is_const>::operator-(int64_t step) const {
  return basic_iterator<is_const>(array_, start_, index_ - step);
}

template <typename T>
template <bool is_const>
int64_t Deque<T>::basic_iterator<is_const>::operator-(
    const basic_iterator& other) const {
  return index_ - other.index_;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>&
Deque<T>::basic_iterator<is_const>::operator++() {
  ++index_;
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>
Deque<T>::basic_iterator<is_const>::operator++(int) {
  ++index_;
  return *this - 1;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>&
Deque<T>::basic_iterator<is_const>::operator--() {
  --index_;
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>
Deque<T>::basic_iterator<is_const>::operator--(int) {
  --index_;
  return *this + 1;
}

template <typename T>
template <bool is_const>
bool Deque<T>::basic_iterator<is_const>::operator==(
    const basic_iterator<is_const>& other) const {
  return index_ == other.index_;
}

template <typename T>
template <bool is_const>
bool Deque<T>::basic_iterator<is_const>::operator!=(
    const basic_iterator<is_const>& other) const {
  return index_ != other.index_;
}

template <typename T>
template <bool is_const>
bool Deque<T>::basic_iterator<is_const>::operator<(
    const basic_iterator<is_const>& other) const {
  return index_ < other.index_;
}

template <typename T>
template <bool is_const>
bool Deque<T>::basic_iterator<is_const>::operator>=(
    const basic_iterator<is_const>& other) const {
  return !(*this < other);
}

template <typename T>
template <bool is_const>
bool Deque<T>::basic_iterator<is_const>::operator>(
    const basic_iterator<is_const>& other) const {
  return other < *this;
}

template <typename T>
template <bool is_const>
bool Deque<T>::basic_iterator<is_const>::operator<=(
    const basic_iterator<is_const>& other) const {
  return !(*this > other);
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>::reference
Deque<T>::basic_iterator<is_const>::operator*() const {
  return const_cast<reference>(
      array_[index_ / Deque<T>::chunk_size][index_ % Deque<T>::chunk_size]);
}

template <typename T>
template <bool is_const>
typename Deque<T>::template basic_iterator<is_const>::pointer
Deque<T>::basic_iterator<is_const>::operator->() const {
  return const_cast<pointer>(
      &(array_[index_ / Deque<T>::chunk_size][index_ % Deque<T>::chunk_size]));
}

template <typename T>
template <bool is_const>
Deque<T>::basic_iterator<is_const>::operator Deque<T>::basic_iterator<true>()
    const {
  return basic_iterator<true>(array_, start_, index_);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
  return {this, static_cast<int64_t>(start_)};
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
  return cbegin();
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
  return {this, static_cast<int64_t>(start_)};
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
  return {this, static_cast<int64_t>(start_ + size_)};
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
  return cend();
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
  return {this, static_cast<int64_t>(start_ + size_)};
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
  return reverse_iterator(iterator(this, static_cast<int64_t>(start_ + size_)));
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const {
  return crbegin();
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
  return const_reverse_iterator(
      const_iterator(this, static_cast<int64_t>(start_ + size_)));
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
  return reverse_iterator(iterator(this, static_cast<int64_t>(start_)));
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const {
  return crend();
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
  return const_reverse_iterator(
      const_iterator(this, static_cast<int64_t>(start_)));
}

template <typename T>
void Deque<T>::insert(const Deque<T>::iterator& iter, const T& element) {
  push_back(element);
  for (size_t i = iter.index_ - start_; i < size_; ++i) {
    std::swap((*this)[i], (*this)[size_ - 1]);
  }
}

template <typename T>
void Deque<T>::erase(const Deque<T>::iterator& iter) {
  for (size_t i = iter.index_ - start_; i < size_ - 1; ++i) {
    std::swap((*this)[i], (*this)[i + 1]);
  }
  pop_back();
}