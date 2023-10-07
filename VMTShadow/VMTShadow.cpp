#include "VMTShadow.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "../code/AllocFree.h"

void VMTShadow::Init(void* object)
{
	m_object = object;
	m_object_vtable = *(void***)(m_object);
	m_object_vtable_size = get_function_count();
	m_object_fake_vtable = (void**)Alloc(m_object_vtable_size * sizeof(void*));
	for (size_t i = 0; i < m_object_vtable_size; i++) {
		m_object_fake_vtable[i] = m_object_vtable[i];
	}
	*(void***)(m_object) = m_object_fake_vtable;
}

void VMTShadow::Quit()
{
	*(void***)(m_object) = m_object_vtable;
	Free(m_object_fake_vtable);
}

size_t VMTShadow::get_function_count()
{
#define PAGE_READABLE (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)
#define PAGE_EXECUTABLE ( PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
	MEMORY_BASIC_INFORMATION mbi;
	size_t i = 0;
	while (true) {
		if (!VirtualQuery(m_object_vtable + i, &mbi, sizeof(mbi))) {
			break;
		}
		if ((mbi.State != MEM_COMMIT) || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) || !(mbi.Protect & PAGE_READABLE)) {
			break;
		}
		if (!VirtualQuery(m_object_vtable[i], &mbi, sizeof(mbi))){
			break;
		}
		if ((mbi.State != MEM_COMMIT) || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) || !(mbi.Protect & PAGE_EXECUTABLE)) {
			break;
		}
		++i;
	}
	return i;
#undef PAGE_READABLE
#undef PAGE_EXECUTABLE
}

void VMTShadow::Hook(size_t index, void** old_function, void* new_function)
{
	*old_function = m_object_fake_vtable[index];
	m_object_fake_vtable[index] = new_function;
}

void VMTShadow::UnHook(size_t index)
{
	m_object_fake_vtable[index] = m_object_vtable[index];
}
