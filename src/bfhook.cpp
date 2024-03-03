#include "pch.h"

void patch_Particle_handleUpdate_crash()
{
    // fix crash when game is minimized
    // the crash occurs in Particle::handleUpdate
    // it calculates an integer index value 0..99 based on the lifetime of the particle, and the object template
    //  has various values that change over time (gravity, drag, size, ..) in OverTimeDistribution objects.
    //  this object can only handle 100 values, when the game is minimized the particle gets stuck somehow and
    //  this index value can reach over 10k, eventually overflowing heap allocations and crashing.
    //  this bug may be responsible for glitchy particles too, when the game is maxmimized, as random stuff is 
    //  being read from the heap
    // side effect: jet particles in SW xpack are odd
    BEGIN_ASM_CODE(mo)
        test eax, 99
        jl resume           ; to original code after jmp eax
        fstp st(0)          ; pop fpu stack, is this needed ?
        mov eax, 00538BDCh  ; jump to end of function, increments time to live and destroys if needed
        jmp eax
        resume :
    MOVE_CODE_AND_ADD_CODE(mo, 0x005389C9, 5, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_scoreboard_column_widths()
{
    // fix ping display on scoreboard
    // the issue is that the ping column width is only 30 (pixel?), whereas for a proper 3 digit column, atleast 35 is needed
    // this code widens the ping column (id 6), and moves the id column (id 7) by 5 pixels, there are 3 always empty columns (8, 9, 10; all width 15)
    // before the 3 award icon columns on the far right side, column id 8 will be squished to width 10 but its unused anyway
    // there are two lists for the two teams, the patching needs to be done twice
    patchBytes(0x006DFB7E, { 0x68, 0x3b, 0x01, 0, 0 }); // push 315  axis - id column (7) - pos x
    patchBytes(0x006DFB8B, { 0x68, 0x68, 0x01, 0, 0 }); // push 360  axis - unused column (8) - pos x
    patchBytes(0x006DFAC0, { 0x68, 0x3b, 0x01, 0, 0 }); // push 315  allied - id column (7) - pos x
    patchBytes(0x006DFACD, { 0x68, 0x68, 0x01, 0, 0 }); // push 360  allied - unused column (8) - pos x
    // also widen name column by squishing the unused column between name(1) and score(3)
    patchBytes(0x006DFA7F, { 0x68, 0xAE, 0x00, 0, 0 }); // push 174  allied - unknown column (2) - pos x
    patchBytes(0x006DFB3D, { 0x68, 0xAE, 0x00, 0, 0 }); // push 174  axis - unknown column (2) - pos x
}

void bfhook_init()
{
    init_hooksystem(NULL);

    patch_Particle_handleUpdate_crash();
    patch_scoreboard_column_widths();
}
