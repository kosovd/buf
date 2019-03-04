#include "SimpleLRU.h"
#include <iostream>
namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h

void SimpleLRU::_erase(lru_node &element) {
	_free_size += (element.key.size() + element.value.size());
  if (element.prev == nullptr) {
    _lru_head.swap(element.next);
    if (_lru_head != nullptr)
      _lru_head->prev = nullptr;
    element.next.release();
  } else if (element.next == nullptr) {
    element.prev->next.release();
  } else {
    element.next->prev = element.prev;
    element.prev->next.swap(element.next);
    element.next.release();
    element.prev = nullptr;
  }
//  _free_size += (element.key.size() + element.value.size());
}

size_t SimpleLRU::get_free_size() {
  return _free_size;
}
size_t SimpleLRU::_pop_back() {
  lru_node *lru_tail;
  if (_lru_head == nullptr)
    return 0;
  for (lru_tail = _lru_head.get(); lru_tail->next != nullptr;
       lru_tail = lru_tail->next.get()) {
  }
  if (lru_tail->prev != nullptr) {
    size_t size = lru_tail->key.size() + lru_tail->value.size();
    _lru_index.erase(lru_tail->key);
    lru_tail = lru_tail->prev;
    lru_tail->next = nullptr;
    _free_size += size;
    return size;
  } else {
    size_t size = (lru_tail->key.size() + lru_tail->value.size());
    _lru_index.erase(lru_tail->key);
    delete lru_tail;
    _free_size += size;
    return size;
  }
}
bool SimpleLRU::_push_front(lru_node *element) {
  size_t size = (element->key.size() + element->value.size());
  if (size > _max_size)
    return false;

  while (_free_size < size) {
	  _pop_back();
 }

  if (_lru_head != nullptr) {
    auto old_head = _lru_head.release();
    old_head->prev = element;
    element->next.reset(old_head);
    element->prev = nullptr;
  }
  _free_size -= size;
  _lru_head.reset(element);
  return true;
}

bool SimpleLRU::Put(const std::string &key, const std::string &value) {
  auto it = _lru_index.find(key);
  if (it != _lru_index.end()) {
    if ((it->second.get().key.size() + value.size()) < _max_size) {
      it->second.get().value = value;
      _erase(it->second.get());
      return _push_front(&(it->second.get()));
    }
    return false;
  } else {
    lru_node *element = new lru_node({key, value});
    if (_push_front(element)) {
      _lru_index.insert(
          std::make_pair(std::ref(element->key), std::ref(*element)));
      return true;
    }
  }
  return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
  if (_lru_index.count(key) == 0) {
    auto element = new lru_node({key, value});
    if (_push_front(element)) {
      _lru_index.insert(std::make_pair(std::ref(key), std::ref(*element)));
      return true;
    }
  }
  return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
  auto it = _lru_index.find(key);
  if (it != _lru_index.end()) {
    if ((it->second.get().key.size() + value.size()) < _max_size) {
      it->second.get().value = value;
      _erase(it->second.get());
      return _push_front(&it->second.get());
    }
  }
  return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
  auto it = _lru_index.find(key);
  if (it != _lru_index.end()) {
    _erase(it->second.get());
    _lru_index.erase(it);
    return true;
  }
  return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
  auto it = _lru_index.find(key);
  if (it != _lru_index.end()) {
    std::string buf = it->second.get().value;
    value = buf;
    _erase(it->second.get());
    return _push_front(&(it->second.get()));
  }
  return false;
}

} // namespace Backend
} // namespace Afina
