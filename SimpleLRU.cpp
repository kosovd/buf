#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if(_lru_index.count(key) == 0){
        
    }else{
        _lru_index[key] -> value = value;
        _lru_head -> prev = _lru_index[key];
        _lru_index[key] -> next = _lru_head.release();
        _lru_index[key] -> prev = nullptr;
        _lru_head = _lru_index[key];
    }
    return false;
    
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { return false; }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) { return false; }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { return false; }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) const { return false; }

} // namespace Backend
} // namespace Afina
