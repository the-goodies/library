#ifndef _hashmap_h
#define _hashmap_h

#include "utility.h"

template <typename type>
class HashMap
{
	struct key_entry
	{
		char* key;
		u16 length;

		key_entry(): key(nullptr), length(0) { /* empty */ }

		key_entry(const char* key, u16 length)
		{
			this->key = (char*)malloc(sizeof(char) * (length + 1)); // +1 for \0
			strncpy_s(this->key, (size_t)(length+1), key, (size_t)length);
			this->length = length;
		}
	};

	key_entry* Keys;
	type* Values;

	s64 capacity; // allocated size
	s64 count; // effective size

	// will double every time reaches limit
	static const u16 INITIAL_CAPACITY = 64;


	u64 hashCode(const char* key, u16 length)
	{
		u64 hash = 5381;
		const u64 MULTIPLIER = 33;
		for (s64 i = 0; i < length; ++i)
			hash = hash * MULTIPLIER + key[i];
		return hash;
	}

	inline u16 key_length(const char* key)
	{
		u16 length = 0;
		while (key[length] != '\0') ++length;
		return length;
	}

	// returns key position in Keys array, -1 if key is not present
	s64 key_position(const char* key)
	{
		u16 length = key_length(key);
		u64 pos = hashCode(key, length) % (u64)capacity;

		while (true)
		{
			if (Keys[pos].key == nullptr) return -1;
			if (strncmp(Keys[pos].key, key, length) == 0) return (s64)pos;
			pos = (pos + 1) % (u64)capacity;
		}
	}

	// inserts key and associated value and returns insertion position
	s64 key_insert(const char* key, type value)
	{
		if (count * 2 >= capacity) resize(capacity * 2);

		u16 length = key_length(key);
		u64 pos = hashCode(key, length) % (u64)capacity;

		while (true)
		{
			if (Keys[pos].key == nullptr)
			{
				Keys[pos]   = key_entry(key, length);
				new (Values + pos) type(std::move(value));
				++count; break;
			}
			if (strncmp(Keys[pos].key, key, length) == 0)
			{
				Values[pos] = std::move(value); break;
			}
			pos = (pos + 1) % (u64)capacity;
		}
		return pos;
	}

	void resize(s64 new_size)
	{
		s64 entry_size = sizeof(key_entry) > sizeof(type) ? sizeof(key_entry) : sizeof(type);
		if ((new_size * entry_size) / new_size != entry_size)
			ERROR("HashMap: capacity expansion failed, given new_size(%d) * entry_size(%d) overflows", new_size, entry_size);

		key_entry* new_Keys = (key_entry*)malloc(sizeof(key_entry) * new_size);
		type* new_Values = (type*)malloc(sizeof(type) * new_size);
		if (new_Keys == nullptr || new_Values == nullptr)
			ERROR("HashMap: failed to allocated memory to resize object");
		for (s64 new_pos = 0; new_pos < new_size; ++new_pos) new(new_Keys + new_pos) key_entry();

		for (s64 pos = 0; pos < capacity; ++pos)
		{
			if (Keys[pos].key == nullptr) continue;
			u64 new_pos = hashCode(Keys[pos].key, Keys[pos].length) % (u64)new_size;
			while (new_Keys[new_pos].key != nullptr) new_pos = (new_pos + 1) % (u64)new_size;
			new_Keys[new_pos] = Keys[pos];
			new (new_Values + new_pos) type(std::move(Values[pos])); Values[pos].~type();
		}
		free(Keys);
		free(Values);

		Keys = new_Keys;
		Values = new_Values;
		this->capacity = new_size;
	}

public:

	class iterator //: public std::iterator<std::input_iterator_tag, char*>
	{
		const HashMap* map;
		s64 pos;

	public:

		// default non usable constructor, which returns invalid iterator
		iterator() : map(nullptr), pos(nullptr) { /* empty */ }

		// end indicates to initialize end iterator
		iterator(const HashMap* map, bool end = false)
		{
			this->map = map;
			if (!end)
			{
				s64 pos = 0;
				while (pos < map->capacity)
				{
					if (map->Keys[pos].key != nullptr)
					{
						this->pos = pos; break;
					}
					++pos;
				}
				if (pos == map->capacity) this->pos = map->capacity;
			}
			else this->pos = map->capacity;
		}

		// copy constructor
		iterator(const iterator & it)
		{
			this->map = it.map;
			this->pos = it.pos;
		}

		iterator & operator++() 
		{
			s64 pos = this->pos + 1;
			while (pos < map->capacity)
			{
				if (map->Keys[pos].key != nullptr)
				{
					this->pos = pos; break;
				}
				++pos;
			}
			if (pos == map->capacity) this->pos = map->capacity;
			
			return *this;
		}

		iterator operator++(int)
		{
			iterator result(*this);
			++(*this);
			return result;
		}

		const char* operator*() { return map->Keys[pos].key; }
		const char** operator->() { return &map->Keys[pos].key; }

		bool operator==(const iterator & it) const { return this->map == it.map && this->pos == it.pos; }
		bool operator!=(const iterator & it) const { return !(*this == it); }
	};

	iterator begin() const { return iterator(this); }
	iterator end() const { return iterator(this, /*end*/ true); }


	HashMap()
	{
		count = 0;
		capacity = INITIAL_CAPACITY;
		Keys = (key_entry*)malloc(sizeof(key_entry) * capacity);
		Values = (type*)malloc(sizeof(type) * capacity);
		if (Keys == nullptr || Values == nullptr)
			ERROR("HashMap: failed to allocated memory to construct object");
		for (s64 pos = 0; pos < capacity; ++pos) new(Keys + pos) key_entry();
	}

	HashMap(s64 size)
	{
		if (size <= 0) ERROR("HashMap: size provided to constructor has to be positive, given %d", size);
		count = 0;
		capacity = size > INITIAL_CAPACITY ? size : INITIAL_CAPACITY;

		s64 entry_size = sizeof(key_entry) > sizeof(type) ? sizeof(key_entry) : sizeof(type);
		if ((size * entry_size) / size != entry_size)
			ERROR("HashMap: size provided to constructor overflows: size(%d) * entry_size(%d)", size, entry_size);

		Keys = (key_entry*)malloc(sizeof(key_entry) * capacity);
		Values = (type*)malloc(sizeof(type) * capacity);
		if (Keys == nullptr || Values == nullptr)
			ERROR("HashMap: failed to allocated memory to construct object");
		for (s64 pos = 0; pos < capacity; ++pos) new(Keys + pos) key_entry();
	}

	// copy constructor
	HashMap(const HashMap<type> & map)
	{
		count = map.count;
		capacity = map.capacity;
		Keys = (key_entry*)malloc(sizeof(char) * capacity);
		Values = (type*)malloc(sizeof(type) * capacity);
		if (Keys == nullptr || Values == nullptr)
			ERROR("HashMap: failed to allocate memory to copy construct");
		for (s64 pos = 0; pos < capacity; ++pos) Keys[pos] = key_entry(map.Keys[pos].key, map.Keys[pos].length);
		for (s64 pos = 0; pos < capacity; ++pos) new (Values + pos) type(Values[pos]);
	}

	// move constructor
	HashMap(HashMap<type> && map)
	{
		count = map.count;
		capacity = map.capacity;
		Keys = map.Keys;
		Values = map.Values;

		map.count = 0;
		map.capacity = 0;
		map.Keys = nullptr;
		map.Values = nullptr;
	}

	// copy/move assignment utilizing copy/move constructor by taking argument as value
	HashMap & operator=(HashMap<type> map)
	{
		// swap
		s64 capacity_temp = capacity;
		s64 count_temp = count;
		key_entry* Keys_temp = Keys;
		type* Values_temp = Values;

		capacity = map.capacity;
		count = map.count;
		Keys = map.Keys;
		Values = map.Values;

		map.capacity = capacity_temp;
		map.count = count_temp;
		map.Keys = Keys_temp;
		map.Values = Values_temp;
		// map going out of scope will destruct old HashMap's object's data
		return *this;
	}

	~HashMap()
	{
		// call destructor manually on all objects, since allocation and construction is done separately
		for (s64 pos = 0; pos < capacity; ++pos)
		{
			if (Keys[pos].key == nullptr) continue;
			free(Keys[pos].key);
			Values[pos].~type();
		}
		free(Keys);
		free(Values);
		capacity = count = 0;
	}

	inline s64 size() { return count; }
	inline bool isEmpty() { return count == 0; }


	// key length can't exceed 2^16 - 1
	void insert(const char* key, const type & value) { key_insert(key, value); }

	// usage: if (map.contains(key)) ...
	bool contains(const char* key) { return key_position(key) != -1; }

	// safe usage: if (map.contains(key)) value = map.get(key);
	// if we are sure that key is present, we can ignore contains check
	type get(const char* key)
	{
		const s64 pos = key_position(key);
		if (pos == -1)
			ERROR("HashMap: '%s' key is not in a map, before getting a value associated with a key, use map.contains(key) to verify that key:value pair is in a map", key);
		return Values[pos];
	}

	// usage: map[key]
	// get and set/insert element through [] operator
	// if key is present, reference of value associated with key is returned
	// if not present, a new entry is created for that key with default type value
	type & operator[](const char* key)
	{		
		s64 pos = key_position(key);
		if (pos == -1) pos = key_insert(key, type());
		return Values[pos];
	}

	// usage: map.remove(key);
	void remove(const char* key)
	{
		s64 pos = key_position(key);
		if (pos == -1) return; // no element to remove

		free(Keys[pos].key); Keys[pos].key = nullptr; Key[pos].length = 0;
		Values[pos].~type();
		--count;

		if (count > INITIAL_CAPACITY && count * 8 == capacity)
		{
			// after resizing, there's no gap to fill
			resize(capacity / 2); return;
		}

		// reallocate next elements to fill a gap
		pos = (pos + 1) % (u64)capacity;
		while(Keys[pos].key != nullptr)
		{
			key_entry key_reallocate = Keys[pos];
			Keys[pos].key = nullptr; Keys[pos].length = 0;
			type value_reallocate(std::move(Values[pos])); Values[pos].~type();
			u64 pos_reallocate = hashCode(key_reallocate.key, key_reallocate.length) % (u64)capacity;

			while (Keys[pos_reallocate].key != nullptr) pos_reallocate = (pos_reallocate + 1) % (u64)capacity;
			Keys[pos_reallocate]   = key_reallocate;
			Values[pos_reallocate] = std::move(value_reallocate);

			pos = (pos + 1) % (u64)capacity;
		}
	}
};


#endif