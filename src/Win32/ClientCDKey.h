#ifndef _CLIENTCDKEY_H
#define _CLIENTCDKEY_H

// ClientCDKey

// Class that implements WON CD-Key functionality for use by clients.  Allows
// conversion to/from human readable string.  Implements persistance to the registry
// in a secure manner.  Implements a lightweight validation check of the key.

#include "../common/won.h"
#include <ostream>

// Forwards
namespace WONCrypt {
	class BFSymmetricKey;
};


// In WONCDKey namespace
namespace WONCDKey
{

class ClientCDKey
{
public:
	// Standard constructor
	// ProductName is string representing product using the CDKey, (i.e., "Homeworld")
	// *IMPORTANT*  product must be at least 3 chars for good security!!!
	explicit ClientCDKey(const std::string& theProductR);

	// Copy Constructor
	ClientCDKey(const ClientCDKey& theKeyR);

	// Destructor
	virtual ~ClientCDKey();

	// Operators
	ClientCDKey& operator=(const ClientCDKey& theKeyR);
	bool32 operator==(const ClientCDKey& theKeyR) const;
	bool32 operator!=(const ClientCDKey& theKeyR) const;

	// Product access.  Careful setting product.  Product is used to encrypt/decypt
	// the key to/from its binary form and for the Lightweight check.
	const std::string& GetProduct() const;
	void SetProduct(const std::string& theProductR);

	// Check two keys for equality
	virtual bool32 IsEqual(const ClientCDKey& theKeyR) const;

	// Check validity of a key, performs lightweight check if needed.
	virtual bool32 IsValid() const;

	// Is this a beta key?
	bool32 IsBeta() const;

	// Init from raw, unencrypted key.  A return of true DOES NOT imply key is valid!
	// (Call IsValid() to verify validity).
	virtual bool32 Init(const __int64& theKeyR);

	// Initialize from Human readable string (dashes are optional):
	//   CVCN-CVCN-CVCN-CVCN-NNNN
	// Returns true if init is successful, false if not.  A return of true DOES NOT
	// imply key is valid!  (Call IsValid() to verify validity).
	virtual bool32 Init(const std::string& theStrR);

	// Init from encrypted binary image (what is returned by AsBinary).  A return of
	// true DOES NOT imply key is valid!  (Call IsValid() to verify validity).
	virtual bool32 Init(const WONCommon::RawBuffer& theKeyR);

	// Fetch as raw, unencrypted (64bit) key
	__int64 AsRaw() const;

	// Fetch as human readable string.  Validates key if needed if validate is true.
	// If Keys fails to validate, returns "#######INVALID KEY######"
	const std::string& AsString(bool32 validate=true) const;

	// Fetch as encrypted binary image.  Valdates key if needed.  Returns empty
	// buffer if key is not valid.
	const WONCommon::RawBuffer& AsBinary() const;

	// Load/Save key in registry securely (key is encrypted in the registry)
	virtual bool32 Load();
	virtual bool32 Save() const;

	// Clean key from registry
	virtual bool32 CleanReg();

protected:
	// Types
	enum ValidityState { Unknown, Invalid, Valid };

	// Members
	std::string           mProduct;     // Product string
	unsigned char         mLightCheck;  // Lightweight checksum
	WONCommon::RawBuffer  mKey;         // Key value
	mutable ValidityState mValidity;    // Current key validity

	// Buffers for string and binary versions of key
	mutable std::string          mStrKey;  // CD-Key as string
	mutable WONCommon::RawBuffer mBinKey;  // CD-Key as encrypted binary

	// Perform the light validity check.  Returns light checksum
	unsigned char LightValidityCheck() const;

	// Build human readable string rep of CDKey.  Fills in mStrKey
	void BuildStringKey() const;

	// Extract LightCheck, and Key from a buffer
	void FieldsFromBuffer(const __int64& theBuf);

	// Build buffer from LightCheck and Key
	__int64 BufferFromFields() const;

	// Decypt mBin Key into buffer
	bool32 DecryptKey(__int64& theBufR);

	// Encrypt buffer into mBinKey
	bool32 EncryptKey(const __int64& theBufR) const;

private:
	// Methods
	void BuildCChar(const __int64& theBuf, unsigned int& theOffset) const;
	void BuildVChar(const __int64& theBuf, unsigned int& theOffset) const;
	void BuildNChar(const __int64& theBuf, unsigned int& theOffset) const;
	void CreateSymmetricKey(WONCrypt::BFSymmetricKey& theSymKeyR) const;

	// Class methods
	static void RemoveSkipChars(string& theStrR);
	static char ValFromBits(const __int64& aBuf, unsigned int theOffset, unsigned int theBits);
	static bool32 ProcessCChar(__int64& theBuf, unsigned int& theOffset, char theChar);
	static bool32 ProcessVChar(__int64& theBuf, unsigned int& theOffset, char theChar);
	static bool32 ProcessNChar(__int64& theBuf, unsigned int& theOffset, char theChar);
};


// Inlines
inline bool32
ClientCDKey::operator==(const ClientCDKey& theKeyR) const
{ return IsEqual(theKeyR); }

inline bool32
ClientCDKey::operator!=(const ClientCDKey& theKeyR) const
{ return (! IsEqual(theKeyR)); }

inline const std::string&
ClientCDKey::GetProduct() const
{ return mProduct; }

inline void
ClientCDKey::SetProduct(const std::string& theProductR)
{ mProduct = theProductR; }

inline __int64
ClientCDKey::AsRaw() const
{ return BufferFromFields(); }

};  //namespace WONCDKey


// Output operators
inline std::ostream&
operator<<(std::ostream& os, const WONCDKey::ClientCDKey& theKeyR)
{ os << theKeyR.AsString();  return os; }

#if 0  // Cat2: cdKeyRegistryCdKeysPathCleanup live code differs from binary or raw IDA offsets
/*=== cdKeyRegistryCdKeysPathCleanup (0x551F70) ===*/
void cdKeyRegistryCdKeysPathCleanup(void)
{
  char v0;

  if ( dword_A70F94 )
  {
    v0 = *(_BYTE *)(dword_A70F94 - 1);
    if ( v0 )
      *(_BYTE *)(dword_A70F94 - 1) = v0 - 1;
    else
      operator_delete((_DWORD *)(dword_A70F94 - 1));
  }
  dword_A70F94 = 0;
  dword_A70F98 = 0;
  dword_A70F9C = 0;
}
#endif

#if 0  // Cat2: stdStringDestruct live code differs from binary or raw IDA offsets
/*=== stdStringDestruct (0x553420) ===*/
void __thiscall stdStringDestruct(_DWORD *this)
{
  int v2;
  _DWORD *v3;
  char v4;

  v2 = this[1];
  if ( v2 )
  {
    v3 = (_DWORD *)(v2 - 1);
    v4 = *(_BYTE *)(v2 - 1);
    if ( v4 )
      *(_BYTE *)v3 = v4 - 1;
    else
      operator_delete(v3);
  }
  this[1] = 0;
  this[2] = 0;
  this[3] = 0;
}
#endif

#if 0  // Cat2: cdKeyRegistryPathPrefixInit live code differs from binary or raw IDA offsets
/*=== cdKeyRegistryPathPrefixInit (0x553C30) ===*/
void cdKeyRegistryPathPrefixInit(void)
{
  _DWORD *v0;
  _DWORD *v1;
  char v2;
  int v3;
  int v4;
  int v5;

  v0 = stdStringAppendString(&v3, &byte_A70F70, (int)&byte_A70F50);
  v5 = 0;
  stdStringAppendString(&dword_A70F30, v0, (int)&byte_A70F20);
  if ( v4 )
  {
    v1 = (_DWORD *)(v4 - 1);
    v2 = *(_BYTE *)(v4 - 1);
    if ( v2 )
      *(_BYTE *)v1 = v2 - 1;
    else
      operator_delete(v1);
  }
}
#endif

#if 0  // Cat2: cdKeyRegistryPathPrefixCleanup live code differs from binary or raw IDA offsets
/*=== cdKeyRegistryPathPrefixCleanup (0x553CC0) ===*/
void cdKeyRegistryPathPrefixCleanup(void)
{
  char v0;

  if ( dword_A70F34 )
  {
    v0 = *(_BYTE *)(dword_A70F34 - 1);
    if ( v0 )
      *(_BYTE *)(dword_A70F34 - 1) = v0 - 1;
    else
      operator_delete((_DWORD *)(dword_A70F34 - 1));
  }
  dword_A70F34 = 0;
  dword_A70F38 = 0;
  dword_A70F3C = 0;
}
#endif

#if 0  // Cat2: clientCDKeyInvalidKeyStringCleanup live code differs from binary or raw IDA offsets
/*=== clientCDKeyInvalidKeyStringCleanup (0x551E70) ===*/
void clientCDKeyInvalidKeyStringCleanup(void)
{
  char v0;

  if ( dword_A70FA4 )
  {
    v0 = *(_BYTE *)(dword_A70FA4 - 1);
    if ( v0 )
      *(_BYTE *)(dword_A70FA4 - 1) = v0 - 1;
    else
      operator_delete((_DWORD *)(dword_A70FA4 - 1));
  }
  dword_A70FA4 = 0;
  dword_A70FA8 = 0;
  dword_A70FAC = 0;
}
#endif
  
#endif