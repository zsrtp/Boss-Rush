#pragma once

#include "display/console.h"
#include "types.h"

namespace mod
{
    void main();
    void onFrameDraw();

    bool onFinish(void* d_a_alink);

    void ook();
    void diababa();
    void dangoro();
    void fyrus();
    void dekutoad();
    void morpheel();
    void deathsword();
    void stallord();
    void darkhammer();
    void blizetta();
    void darknut();
    void armogohma();
    void arealfos();
    void argarok();
    void zant();
    void ganon();

    void (*onFrameDraw_Return)() = nullptr;
    bool (*onGanonFinish_Return)(void* d_a_alink) = nullptr;
    s32 (*createItemForPresentDemo_Return)(const float pos[3],
                                           s32 item,
                                           u8 unk3,
                                           s32 unk4,
                                           s32 unk5,
                                           const float unk6[3],
                                           const float unk7[3]) = nullptr;
    s32 (*createItemForTrBoxDemo_Return)(const float pos[3],
                                         s32 item,
                                         s32 unk3,
                                         s32 unk4,
                                         const float unk5[3],
                                         const float unk6[3]) = nullptr;

    struct bossFightInfo
    {
        char stage[8];
        u8 room;
        u8 state;
        u8 spawnpoint;
        u8 expectedItemID;
        void (*preHook)();
    };

    libtp::display::Console* console;

    bossFightInfo bossFights[16] = {
        {"D_MN05B", 51, 0xFF, 0x00, 0x40, ook},         // -Ook
        {"D_MN05A", 50, 0xFF, 0x01, 0x22, diababa},     // Diababa
        {"D_MN04B", 51, 0xFF, 0x03, 0x43, dangoro},     // -Dangoro
        {"D_MN04A", 50, 0xFF, 0x01, 0x22, fyrus},       // Fyrus
        {"D_MN01B", 51, 0xFF, 0x00, 0x44, dekutoad},    // -Deku Toad
        {"D_MN01A", 50, 0xFF, 0x02, 0x22, morpheel},    // Morpheel
        {"D_MN10B", 51, 0xFF, 0x01, 0x41, deathsword},  // -Death Sword
        {"D_MN10A", 50, 0xFF, 0x01, 0x22, stallord},    // Stallord
        {"D_MN11B", 51, 0xFF, 0x00, 0x42, darkhammer},  // -Dark Hammer
        {"D_MN11A", 50, 0xFF, 0x00, 0x22, blizetta},    // Blizetta
        {"D_MN06B", 51, 0xFF, 0x00, 0x46, darknut},     // -Darknut
        {"D_MN06A", 50, 0xFF, 0x01, 0x22, armogohma},   // Armogohma
        {"D_MN07B", 51, 0xFF, 0x02, 0x47, arealfos},    // -Arealfos
        {"D_MN07A", 50, 0xFF, 0x00, 0x22, argarok},     // Argarok
        {"D_MN08D", 53, 0xFF, 0x00, 0x22, zant},        // Zant
        {"D_MN09A", 50, 0xFF, 0x00, 0x00, ganon}        // Ganon
    };

    // TODO: Create a class for this
    // TODO: Find a way to determine 50hz mode

    u32 drawFrameCalls;

    // Number of all frames while mini-game was active
    u32 totalFrames;

    // Timer for boss rush
    u32 frameTimer;
    u32 hours;
    u32 minutes;
    u32 seconds;

    // Helpers
    u32 lastInput;
    u32 HUDbuttonCombo;
    u32 StartbuttonCombo;
    u32 StartbuttonCombo_BossOnly;
    u8 currentBossFight;
    u8 clawshotItemWheelIndex;
    u8 itemWheelPos;

    bool HUD;
    bool gameActive;

}  // namespace mod