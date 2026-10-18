#if 0  // Decompiled pseudocode reference
/*=== jdiv_round_up (0x5D6F40) ===*/
int __cdecl jdiv_round_up(int a, int b)
{
  return (a + b - 1) / b;
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== jround_up (0x5D6F50) ===*/
int __cdecl jround_up(int a, int b)
{
  return a + b - 1 - (a + b - 1) % b;
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== jcopy_sample_rows (0x5D6F70) ===*/
void __cdecl jcopy_sample_rows(
        void **input_array,
        int source_row,
        void **output_array,
        int dest_row,
        int num_rows,
        unsigned int byte_count)
{
  int v6; // ebx
  const void **v7; // eax
  void **v8; // edx

  v6 = num_rows;
  v7 = (const void **)&input_array[source_row];
  v8 = &output_array[dest_row];
  if ( num_rows > 0 )
  {
    do
    {
      qmemcpy(*v8++, *v7++, byte_count);
      --v6;
    }
    while ( v6 );
  }
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== jzero_far (0x5D6FC0) ===*/
void __cdecl jzero_far(void *target, unsigned int bytestozero)
{
  memset(target, 0, bytestozero);
}
#endif
