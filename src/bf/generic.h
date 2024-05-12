#pragma once
#include <cstdint>
#include <string>

enum ClassID {
    CID_BFSoldierTemplate = 0x9493
};

class IBase {
public:
    virtual void addRef() = 0;
    virtual void release() = 0;
    virtual IBase* queryInterface(uint32_t) = 0;
};

class IConsoleSaveable : public IBase {
public:
    virtual bool makeScript(void* stream) = 0; // IStream*
};

class IObject;
class BitStream;

struct Vec2 {
    float x, y;
};

class Vec3;
typedef Vec3 Pos3;
class Mat4; // BaseMatrix4<float>

struct PlayerInput {
    float controls[55];
    uint64_t mask;
    int unk1;
    int unk2;
};

static_assert(sizeof(PlayerInput) == 0xF0);

class Game : public IBase {
public:
    // missing virtual methods here

    void addPlayerInput_orig(int playerid, PlayerInput* input) noexcept;
    void addPlayerInput_hook(int playerid, PlayerInput* input);
};


class BFPlayer : public IBase {
public:
    virtual int GetClassID();
    virtual ~BFPlayer();
    virtual void setName(bfs::string const& name);
    virtual bfs::string const& getName() const;
    virtual void setFlags(void*);
    virtual bool testFlags(void*) const;
    virtual void handleUpdate(float tDelta, unsigned int);
    virtual int getUpdateFrequencyType() const;
    virtual void setUpdateFrequencyType(int);
    virtual void handleInput(PlayerInput* input, float tDelta);
    virtual void* getCamera() const;
    virtual IObject* getVehicle() const;
    virtual bool setVehicle(IObject* vehicle, int inputid);
    virtual int getInputId() const;
    virtual int getId() const;
    virtual void setId(int id);
    virtual void setIsRemote();
    virtual bool getIsRemote() const;
    virtual void setIsAIPlayer();
    virtual bool getIsAIPlayer() const;
    virtual void setCamera(IObject* camera);
    // these belong in PlayerManager
    static BFPlayer* __stdcall getFromID(int id);
    static BFPlayer* getLocal();
    static bfs::list<BFPlayer*>* getPlayers();
};

template<int B>
struct Placeholder {
    uint8_t b[B];
};

// For casting non-virtual method pointers to void*, for hook_function
// example: hook_function(..., method_to_voidptr(&class::method);
// This is needed because C++ doesn't let you cast method pointers, but probably
// there is a better solution
// See also:
//  https://stackoverflow.com/questions/1307278/casting-between-void-and-a-pointer-to-member-function
//  https://stackoverflow.com/questions/1207106/calling-base-class-definition-of-virtual-member-function-with-function-pointer
__declspec(naked) inline void* method_to_voidptr(...) {
    _asm mov eax, [esp + 4]
    _asm ret
}


template<class Tk, class Tv/*, class Tcompare*/>
class Hash {
public:
    class Node {
        Node* next;
        Tk key;
        Tv value;
    };
    Node* buckets;
    size_t numBuckets;
    uint32_t unknown;
};

uint32_t __fastcall calcStringHashValueNoCase(const bfs::string& str);

// MD5 hashes data, then fills outputHexString with 32 hexadecimal characters. A null
// terminator is also appended, outputHexString must be atleast 33 bytes.
void __fastcall MD5Digest(const void* data, unsigned int length, char* outputHexString);


void generic_hook_init();
