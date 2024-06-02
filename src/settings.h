#pragma once
#include <string>
#include "SimpleIni.h"

struct Setting {
    const wchar_t* section;
    const wchar_t* name;
    const wchar_t* comment;
    int resourceid;
    bool dirty = false;
    virtual void load(const CSimpleIni& ini) = 0;
    virtual void save(CSimpleIni& ini) = 0;
protected:
    Setting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid) :
        section(section), name(name), comment(comment), resourceid(resourceid) {};
};

struct StringSetting : public Setting {
    std::wstring value;
    StringSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, std::wstring value) :
        Setting(section, name, comment, resourceid), value(value) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
};

struct BoolSetting : public Setting {
    bool value;
    BoolSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, bool value) :
        Setting(section, name, comment, resourceid), value(value) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
    operator bool() const { return value; };
};

struct IntSetting : public Setting {
    int value;
    IntSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, int value) :
        Setting(section, name, comment, resourceid), value(value) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
    operator int() const { return value; };
};

struct ColorSetting : public Setting {
    uint32_t value;
    ColorSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, uint32_t value) :
        Setting(section, name, comment, resourceid), value(value) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
    operator uint32_t() const { return value; };
};

// space separated list of colors
struct ColorListSetting : public Setting {
    std::vector<uint32_t> value;
    ColorListSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, std::initializer_list<uint32_t> values) :
        Setting(section, name, comment, resourceid), value(values) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
};

class Settings {
    CSimpleIni ini;
    std::vector<Setting*> settings;
public:
    Settings();
    bool load();
    bool save(bool force);
    // Store a buddy color in the settings file.
    // Buddy colors are stored in a separate [buddycolors] section.
    // If color is set to InvalidColor, the entry is removed for the given player name.
    void setBuddyColor(std::wstring name, uint32_t color) {
        if (color != InvalidColor) ini.SetValue(L"buddycolors", name.c_str(), ISO88591ToWideString(GetStringFromColor(color)).c_str());
        else ini.Delete(L"buddycolors", name.c_str(), false);
    };
    BoolSetting showConnectsInChat = {
        L"general", L"showConnectsInChat",
        L"; Show a message in the status chat when a player connects to the server",
        0, false };
    BoolSetting showIDInChat = {
        L"general", L"showIDInChat",
        L"; Show player IDs in chat messages, name changes and joins/disconnects",
        0, false };
    BoolSetting showIDInKills = {
        L"general", L"showIDInKills",
        L"; Show player IDs in kill messages",
        0, false };
    BoolSetting showIDInNametags = {
        L"general", L"showIDInNametags",
        L"; Show player IDs in nametags",
        0, false };
    BoolSetting showVoteInConsole = {
        L"general", L"showVoteInConsole",
        L"; Show who starts a vote or votes in the console",
        0, false };
    BoolSetting lowerNametags = {
        L"general", L"lowerNametags",
        L"; Gradually lower nametags when u get closer to other players",
        0, true };
    ColorSetting debugTextColor = {
        L"general", L"debugTextColor",
        L"; Sets the color of console.showStats, console.showFPS, etc",
        0, 0xffff00 /*yellow*/ };
    BoolSetting smootherGameplay = {
        L"general", L"smootherGameplay",
        L"; Enables some patches that make the game run smoother. This\n"
        L"; may have a very small performance impact on older systems.\n"
        L"; The patches affect the floating point precision the game runs\n"
        L"; with. When playing in multiplayer the time it takes for the\n"
        L"; server to process your input will be decreased.",
        0, true };
    ColorListSetting presetBuddyColors = {
        L"general", L"presetBuddyColors",
        L"; Sets the colors chosen when the ADD BUDDY button is repeatedly clicked on the scoreboard",
        0, { DefaultBuddyColor /*lime*/, 0x1e90ff /*dodgerblue*/, 0xffff00 /*yellow*/, 0x8a2be2 /*blueviolet*/} };
    BoolSetting correctedLookSensitivity = {
        L"general", L"correctedLookSensitivity",
        L"; The game has higher look left/right sensitivity when you are on-foot and moving\n"
        L"; This option enables a workaround that makes the on-foot sensitivity to always be the same\n"
        L"; WARNING: Enabling this may feel unusual because of the decreased sensitivity!",
        0, false };
    BoolSetting enable3DMineMap = {
        L"general", L"enable3DMineMap",
        L"; Enable 3D map showing friendly mines. This is a temporary option.",
        0, false };
    BoolSetting enable3DSupplyMap = {
        L"general", L"enable3DSupplyMap",
        L"; Enable 3D map showing heal, ammo, repair points. This is a temporary option.",
        0, false };
    BoolSetting crashCreateFullDump = {
        L"general", L"crashCreateFullDump",
        L"; When the game crashes create a full memory dump, which are much larger (500MB+).",
        0, false };
    IntSetting crashDumpsToKeep = {
        L"general", L"crashDumpsToKeep",
        L"; Number of crash dumps to keep. Set to 0 to never delete dumps.",
        0, 5 };
};

extern Settings g_settings;
