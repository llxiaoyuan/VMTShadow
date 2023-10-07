#pragma once
#include <stdint.h>

class VMTShadow
{
private:
	void* m_object;
	void** m_object_vtable;
	void** m_object_fake_vtable;
	size_t m_object_vtable_size;
	size_t get_function_count();
public:
	void Init(void* object);
	void Quit();
	void Hook(size_t index, void** old_function, void* new_function);
	void UnHook(size_t index);
};
