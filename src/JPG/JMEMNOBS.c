#if 0  // Decompiled pseudocode reference
/*=== jpeg_mem_available (0x5D6F10) ===*/
int __cdecl jpeg_mem_available(void *cinfo, int min_bytes_needed, int max_bytes_needed)
{
  return max_bytes_needed;
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== jpeg_open_backing_store (0x5D6F20) ===*/
void __cdecl jpeg_open_backing_store(void *cinfo)
{
  *(_DWORD *)(*(_DWORD *)cinfo + 20) = 46;
  (**(void (__cdecl ***)(void *))cinfo)(cinfo);
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== jpeg_free_small (0x5D6F00) ===*/
void __cdecl jpeg_free_small(void *cinfo, void *object)
{
free(object);
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== jpeg_get_small (0x5D6EF0) ===*/
void *__cdecl jpeg_get_small(void *cinfo, size_t sizeofobject)
{
  return malloc_34(sizeofobject);
}
#endif
