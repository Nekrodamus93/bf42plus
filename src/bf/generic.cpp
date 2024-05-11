#include "../pch.h"

__declspec(naked) BFPlayer* __stdcall BFPlayer::getFromID(int id)
{
    __asm {
        mov ecx, 0x0097D76C // pPlayerManager
        mov ecx, [ecx]
        test ecx,ecx
        jnz cont
        xor eax,eax // player manager is null, return 0
        ret 4
    cont:
        mov eax, [ecx]
        jmp dword ptr [eax+0x18] // tailcall to pPlayerManager->getPlayerFromId()
    }
}

__declspec(naked )BFPlayer* BFPlayer::getLocal()
{
    __asm {
        mov ecx, 0x0097D76C // pPlayerManager
        mov ecx, [ecx]
        test ecx, ecx
        jnz cont
        xor eax, eax // player manager is null, return 0
        ret
    cont:
        mov eax, [ecx]
        jmp dword ptr[eax+0x20] // tailcall to pPlayerManager->getLocalHumanPlayer()
    }
}

__declspec(naked) bfs::list<BFPlayer*>* BFPlayer::getPlayers()
{
    __asm {
        mov ecx, 0x0097D76C // pPlayerManager
        mov eax, [ecx]
        test eax, eax
        jz return
        add eax,0xC // PlayerManager->playerList
    return:
        ret
    }
}

__declspec(naked) uint32_t __fastcall calcStringHashValueNoCase(const bfs::string& str)
{
    _asm mov eax, 0x00502E60
    _asm jmp eax
}

static uintptr_t addPlayerInput_addr = 0x0040ECB0;
__declspec(naked) void Game::addPlayerInput_orig(int playerid, PlayerInput* input) noexcept
{
    _asm mov eax, addPlayerInput_addr
    _asm jmp eax
}

static uintptr_t getStringFromRegistry_addr = 0x00443F10;
__declspec(naked) bool __stdcall getStringFromRegistry(const char* key, const char* valueName, char* output, size_t* outlength)
{
    _asm mov eax, getStringFromRegistry_addr
    _asm jmp eax
}

void Game::addPlayerInput_hook(int playerid, PlayerInput* input)
{
    // input can be modified here, the object will not be used after this function returns

    // The following is a fix for the issue that when stainding on-foot, the mouse
    // sensitivity is half compared to when you are moving. This is caused by some bug
    // in BFSoldier::handlePlayerInput, but fixing it there would cause desync when playing
    // on servers (because the server code has the same bug). This workaround halves the
    // LookX sensitivity when the player is moving (forward or strafe inputs arent 0).
    // This is to match the vertical sensitivity, which used to be half of the horizontal
    // when moving.
    if (g_settings.correctedLookSensitivity) {
        auto localPlayer = BFPlayer::getLocal();
        if (localPlayer && localPlayer->getId() == playerid) {
            auto vehicle = localPlayer->getVehicle();
            // is player in BFSoldier?
            if (vehicle && vehicle->getTemplate()->getClassID() == CID_BFSoldierTemplate) {
                // moving? (forward or strafe arent 0)
                if (input->controls[3] != 0.0 || input->controls[0] != 0.0) {
                    // halve look left/right sensitivity
                    input->controls[4] /= 2;
                }
            }
        }
    }

    addPlayerInput_orig(playerid, input);
}

// This function reads a value from HKLM, it is used by the game to read the CD key out of registry
// this is a Setup method but the "this" object pointer is not used so it can be called as __stdcall
bool __stdcall getStringFromRegistry_hook(const char* key, const char* valueName, char* output, size_t* outlength)
{
    bool ok = getStringFromRegistry(key, valueName, output, outlength);

    // Make sure a CD key is being retrieved
    size_t keylength = strlen(key);
    if (keylength < 6 || strcmp(key + (keylength - 5), "\\ergc") != 0 || *valueName != 0) {
        // Not a CD key is being retrieved, this never actually happens
        return ok;
    }

    bool useMachineGuid = false;
    if (!ok) {
        // Reading CD key from registry failed, use alternate methods for obtaining an unique ID
        useMachineGuid = true;
        ok = true;
    }

    if (useMachineGuid) {
        // Use windows installation ID as a base for key generation
        char guid[40];
        DWORD guid_size = sizeof(guid);
        if (GetMachineGUID((unsigned char*)guid, &guid_size)) {
            // The hash of the installation id is converted into a string containing a 22 digit decimal number
            
            // Hash the installation ID, use the 96 bits of the result as 3 32 bit integers
            sodium_init();
            uint32_t temp[3];
            crypto_generichash((unsigned char*)temp, sizeof(temp), (unsigned char*)guid, guid_size, 0, 0);

            // convert the three integers into three 10 character decimal numbers
            char tempstr[3][12];
            for (int i = 0; i < 3; i++) sprintf(tempstr[i], "%010u", temp[i]);

            // concatenate the three numbers, drop the first digits because its less random
            // the output buffer is always 40 bytes
            snprintf(output, 23, "%s%s%s\n", tempstr[0] + 3, tempstr[1] + 2, tempstr[2] + 3);
            *outlength = strlen(output);
        }
    }
    return ok;
}

void generic_hook_init()
{
    addPlayerInput_addr = (uintptr_t)hook_function(addPlayerInput_addr, 8, method_to_voidptr(&Game::addPlayerInput_hook));
    getStringFromRegistry_addr = (uintptr_t)hook_function(getStringFromRegistry_addr, 5, getStringFromRegistry_hook);
}
