#include "SimpleLRU.h"
#include <iostream>
namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h

void SimpleLRU::_erase(lru_node &element){
	if(element.prev == nullptr){
		_lru_head.swap(element.next);
		if(_lru_head != nullptr) _lru_head->prev = nullptr;
		element.next.release();
	}else if(element.next == nullptr){
		element.prev->next.release();
	}else{
		element.next->prev = element.prev;
		element.prev->next.swap(element.next);
		element.next.release();
		element.prev = nullptr;
	}
}

size_t SimpleLRU::get_free_size(){
	size_t free_size = _max_size;
	if(_lru_head != nullptr){
		free_size = _lru_head->key.size() + _lru_head->value.size();
		if(_lru_head->next != nullptr){
			for(auto i = _lru_head->next.get(); i != nullptr; i = i->next.get()){
				free_size += (i->key.size() + i->value.size());
			}
		}
		return _max_size -  free_size;
	}
	return free_size;
}
size_t SimpleLRU::_pop_back(){
	lru_node *lru_tail;
	if(_lru_head == nullptr) return 0;
	for(lru_tail = _lru_head->next.get(); 
			lru_tail->next != nullptr; 
			lru_tail = lru_tail->next.get()){}
	if(lru_tail->prev != nullptr){
		size_t size = lru_tail->key.size() + lru_tail->value.size();
		_lru_index.erase(lru_tail->key);
		lru_tail = lru_tail->prev;
		lru_tail->next = nullptr;
		return size;
	}else{
		size_t size = lru_tail->key.size() + lru_tail->value.size();
		_lru_index.erase(lru_tail->key);
		delete lru_tail;
		return size;
	}
}	
bool SimpleLRU::_push_front(lru_node *element){
	size_t size = element->key.size() + element->value.size();
	if(size > _max_size) return false;
	size_t free_size = get_free_size();

	while(free_size < size){
		free_size += _pop_back();
	}

	if(_lru_head != nullptr){
		auto old_head = _lru_head.release();
		if(element->next != nullptr && element->prev != nullptr){
			element->next->prev = element->prev;
			element->prev->next.swap(element->next);
			element->next.release();
		}else if(element->prev != nullptr){
			element->prev->next.release();
		}

		old_head->prev = element;
		element->next.reset(old_head);
		element->prev = nullptr;
	}
	_lru_head.reset(element);
	return true;
	
}
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	auto it = _lru_index.find(key);
/*	auto kek = [this](){
		std::cout << "map: " << std::endl;
		for(auto i : _lru_index){
			std::cout << i.first.get() << " " << i.second.get().value << std::endl;
		}
		std::cout << "list: " << std::endl;
		for(auto i = _lru_head.get(); i != nullptr; i = i->next.get()){
			std::cout << i->value << " ";
		}
		std::cout << std::endl;
		};
		*/
	if(it != _lru_index.end()){
		if((it->second.get().key.size() + value.size()) < _max_size){
			it->second.get().value = value;
			_erase(it->second.get());			
			return _push_front(&(it->second.get()));
		//	kek();
		}
		return false;	
	}else{
		lru_node *element = new lru_node({key, value});
		if(_push_front(element)){
			_lru_index.insert(std::make_pair(std::ref(element->key), std::ref(*element)));
		//	kek();
			return true;
		}
	}
	return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
	if(_lru_index.count(key) == 0){
		auto element = new lru_node({key, value});
		if(_push_front(element)){
			_lru_index.insert(std::make_pair(std::ref(key), std::ref(*element)));
			return true;			
		}
	}
	return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
	auto it = _lru_index.find(key);
	if(it != _lru_index.end()){
		if((it->second.get().key.size() + value.size()) < _max_size){  
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
 	if(it != _lru_index.end()){
		_erase(it->second.get());
		_lru_index.erase(it);
		return true;
	}	
	return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value){
	auto it = _lru_index.find(key);
	if(it != _lru_index.end()){
		std::string buf = it->second.get().value;
		value = buf;
		_erase(it->second.get());
		return _push_front(&(it->second.get()));
	}	
	return false; 
}

} // namespace Backend
} // namespace Afina
