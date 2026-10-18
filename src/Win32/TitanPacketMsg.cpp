#if 0  // Decompiled pseudocode reference
/*=== TitanPacketMsgConstruct (0x5a4600) ===*/
 _DWORD *__thiscall TitanPacketMsgConstruct(_DWORD *this, char a2, char a3)
 {
   WONMsgMiniMessageConstruct(this);
   *((_WORD *)this + 6) = 0;
   this[4] = 0;
   *((_BYTE *)this + 20) = a2 != 96;
   *((_BYTE *)this + 24) = a2;
   this[7] = 0;
   this[8] = 0;
   this[9] = 0;
   *this = &off_72BF20;
   *(_BYTE *)(WONMsgBaseMessageGetDataPtr(this) + 1) = (a3 != 0) - 30;
   *(_BYTE *)(WONMsgBaseMessageGetDataPtr(this) + 2) = a2;
   return this;
 }
#endif

#if 0  // Decompiled pseudocode reference
/*=== TitanPacketMsgDelete (0x5a46a0) ===*/
 _DWORD *__thiscall TitanPacketMsgDelete(_DWORD *this, char a2)
 {
   TitanPacketMsgDestruct(this);
   if ( (a2 & 1) != 0 )
     operator delete(this);
   return this;
 }
#endif

#if 0  // Decompiled pseudocode reference
/*=== TitanPacketMsgConstructFromMiniMessage (0x5a46c0) ===*/
 _WORD *__thiscall TitanPacketMsgConstructFromMiniMessage(_DWORD *this, int a2)
 {
   WONMsgMiniMessageCopyConstruct(this, a2);
   *((_WORD *)this + 6) = 0;
   this[4] = 0;
   *((_BYTE *)this + 20) = 1;
   *((_BYTE *)this + 24) = a2;
   this[7] = 0;
   this[8] = 0;
   this[9] = 0;
   *this = &off_72BF20;
   TitanPacketMsgUnpack(this);
   return this;
 }
#endif

#if 0  // Decompiled pseudocode reference
/*=== TitanPacketMsgDestruct (0x5a4730) ===*/
 void __thiscall TitanPacketMsgDestruct(_DWORD *this)
 {
   int v2; // ecx
   char v3; // al

   *this = &off_72BF20;
   v2 = this[7];
   if ( v2 )
   {
     v3 = *(_BYTE *)(v2 - 1);
     if ( v3 )
       *(_BYTE *)(v2 - 1) = v3 - 1;
     else
       operator delete((void *)(v2 - 2));
   }
   this[7] = 0;
   this[8] = 0;
   this[9] = 0;
   WONMsgMiniMessageDestruct(this);
 }
#endif

#if 0  // Decompiled pseudocode reference
/*=== TitanPacketMsgPack (0x5a4780) ===*/
 int __thiscall TitanPacketMsgPack(_DWORD *this)
 {
   const void *v2; // ebx
   unsigned int v3; // edi

   sub_695020(this);
   if ( *((_BYTE *)this + 20) )
   {
     WONMsgBaseMessageEnsureUniqueData(this);
     WONMsgDataAppendWString((_DWORD *)this[1], (int)(this + 6));
   }
   v2 = (const void *)this[4];
   v3 = *((unsigned __int16 *)this + 6);
   WONMsgBaseMessageEnsureUniqueData(this);
   WONMsgDataAppendBytes((_DWORD *)this[1], v3, v2, 1, 1);
   return WONMsgBaseMessageGetDataPtr(this);
 }
#endif

#if 0  // Decompiled pseudocode reference
/*=== TitanPacketMsgUnpack (0x5a47d0) ===*/
 int __thiscall TitanPacketMsgUnpack(void *this)
 {
   bool v2; // al
   __int16 v3; // ax
   _DWORD *v4; // ecx
   int v5; // eax
   int result; // eax
   char v7[204]; // [esp+4h] [ebp-CCh] BYREF

   sub_694E90(this);
   if ( (*(int (__thiscall **)(void *))(*(_DWORD *)this + 52))(this) != 227
     && (*(int (__thiscall **)(void *))(*(_DWORD *)this + 52))(this) != 226 )
   {
     sub_6B3590(
       (Concurrency::details::UMSFreeVirtualProcessorRoot *)v7,
       (int)this,
       100,
       aCCataclysmSrcW_3,
       (int)aNotATitanpacke);
     _CxxThrowException(v7, &_TI3_AVBadMsgException_WONMsg__);
   }
   v2 = (*(int (__thiscall **)(void *))(*(_DWORD *)this + 56))(this) != 96;
   *((_BYTE *)this + 20) = v2;
   if ( v2 )
     WONMsgDataReadWString(*((_DWORD **)this + 1), (_DWORD *)this + 6);
   v3 = WONMsgDataBytesLeft(*((_DWORD **)this + 1));
   v4 = (_DWORD *)*((_DWORD *)this + 1);
   *((_WORD *)this + 6) = v3;
   v5 = WONMsgDataBytesLeft(v4);
   result = WONMsgDataReadBytes(*((_DWORD **)this + 1), v5);
   *((_DWORD *)this + 4) = result;
   return result;
 }
#endif

#if 0  // Decompiled pseudocode reference
/*=== TitanPacketMsgDuplicate (0x5a4910) ===*/
 _DWORD *__thiscall TitanPacketMsgDuplicate(_WORD *this)
 {
   _DWORD *v2; // eax
   _DWORD *v3; // esi
   int v4; // ecx

   v2 = operator new(0x28u);
   v3 = v2;
   if ( !v2 )
     return 0;
   WONMsgMiniMessageCopyConstruct(v2, (int)this);
   *((_WORD *)v3 + 6) = this[6];
   v3[4] = *((_DWORD *)this + 4);
   *((_BYTE *)v3 + 20) = *((_BYTE *)this + 20);
   *((_BYTE *)v3 + 24) = *((_BYTE *)this + 24);
   v3[7] = 0;
   v3[8] = 0;
   v3[9] = 0;
   sub_59E960(v3 + 6, (_DWORD *)this + 6, 0, dword_72BF1C);
   *v3 = &off_72BF20;
   v4 = *((_DWORD *)this + 4);
   *((_WORD *)v3 + 6) = this[6];
   v3[4] = v4;
   return v3;
 }
#endif
