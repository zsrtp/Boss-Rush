#include "main.h"

#include <data/stages.h>
#include <display/console.h>
#include <patch.h>
#include <tools.h>
#include <tp/JFWSystem.h>
#include <tp/d_a_alink.h>
#include <tp/d_com_inf_game.h>
#include <tp/f_ap_game.h>
#include <tp/f_op_actor_mng.h>
#include <tp/f_op_scene_req.h>
#include <tp/m_do_controller_pad.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace mod
{
    void main()
    {
        console = new libtp::display::Console("AECX", "Boss Rush", "Fast paced fighting action, beat boss after boss",
                                              "as quickly as possible. At the end of each fight",
                                              "you'll be teleported to the next one...", "Version: 0.3b");

        // Hook to a suitable function
        // This one runs every frame
        onFrameDraw_Return = libtp::patch::hookFunction(libtp::tp::f_ap_game::fapGm_Execute, onFrameDraw);

        onGanonFinish_Return = libtp::patch::hookFunction(libtp::tp::d_a_alink::procGanonFinish, onFinish);

        HUDbuttonCombo = libtp::tp::m_do_controller_pad::Button_R | libtp::tp::m_do_controller_pad::Button_Z;
        StartbuttonCombo = libtp::tp::m_do_controller_pad::Button_R | libtp::tp::m_do_controller_pad::Button_Start;

        return;
    }

    bool onFinish(void* d_a_alink)
    {
        if (gameActive)
        {
            gameActive = false;
            libtp::display::print(10, "gg & gz!");
        }
        return onGanonFinish_Return(d_a_alink);
    }

    void onFrameDraw()
    {
        // Clear console after 900 frames
        if (drawFrameCalls < 900)
        {
            drawFrameCalls++;
        }
        else if (drawFrameCalls == 900)
        {
            drawFrameCalls++;
            // Init console for our purpose(s)
            libtp::display::clearConsole(0, 25);

            // Fully transparent
            libtp::display::setConsoleColor(0xFF, 0xFF, 0xFF, 0x0);
            libtp::display::setConsole(true, 25);
        }
        else
        {
            // Check for button combo
            // TODO: create libtp wrapper function

            u32 cInput = libtp::tp::m_do_controller_pad::cpadInfo.buttonInput;

            if (cInput != lastInput)
            {
                if ((cInput & HUDbuttonCombo) == HUDbuttonCombo)
                {
                    // Toggle HUD
                    HUD = !HUD;
                    libtp::display::setConsole(HUD, 0);
                }

                if (!gameActive && (cInput & StartbuttonCombo) == StartbuttonCombo)
                {
                    // Start gamemode, reset timers etc.
                    frameTimer = 0;
                    totalFrames = 0;
                    hours = 0;
                    minutes = 0;
                    seconds = 0;
                    currentBossFight = 0;
                    itemWheelPos = 0;

                    libtp::display::clearConsole(0, 25);

                    gameActive = true;

                    // Make sure user has everything
                    for (u8 i = 0; i < currentBossFight; i++)
                    {
                        bossFights[i].preHook();
                    }

                    // Trigger first fight
                    bossFights[currentBossFight].preHook();
                    libtp::tools::triggerSaveLoad(bossFights[currentBossFight].stage, bossFights[currentBossFight].room,
                                                  bossFights[currentBossFight].spawnpoint, bossFights[currentBossFight].state);
                }
                else if ((cInput & StartbuttonCombo) == StartbuttonCombo)
                {
                    gameActive = false;
                }
            }  // End input check

            if (gameActive)
            {
                // Check for events if not loading
                bossFightInfo* boss = &bossFights[currentBossFight];
                if (!libtp::tp::f_op_scene_req::isLoading && boss->flagVar > 0 &&
                    ((libtp::tp::d_com_inf_game::dComIfG_gameInfo.localAreaNodes[boss->flagOffset] & boss->flagVar) ==
                     boss->flagVar))
                {
                    // Desired flag has been reached, load next one
                    currentBossFight++;
                    boss = &bossFights[currentBossFight];

                    boss->preHook();
                    libtp::tools::triggerSaveLoad(boss->stage, boss->room, boss->spawnpoint, boss->state);
                }

                frameTimer++;
                totalFrames++;
                // Update every second to save some processing power
                if (frameTimer == 30)
                {
                    frameTimer = 0;
                    seconds++;

                    if (seconds == 60)
                    {
                        seconds = 0;
                        minutes++;
                        if (minutes == 60)
                        {
                            minutes = 0;
                            hours++;
                        }
                    }
                }
            }

            // Print HUD if active
            if (HUD)
            {
                sprintf(libtp::tp::jfw_system::systemConsole->consoleLine[5].line, "Boss: %d", currentBossFight + 1);
                sprintf(libtp::tp::jfw_system::systemConsole->consoleLine[6].line, "%02d:%02d:%02d", hours, minutes, seconds);

                sprintf(libtp::tp::jfw_system::systemConsole->consoleLine[24].line, "Frames: %u", totalFrames);
            }
        }

        lastInput = libtp::tp::m_do_controller_pad::cpadInfo.buttonInput;
        return onFrameDraw_Return();
    }

    // Prehooks
    void ook()
    {
        // Give basic equip (heroe's cloths, za, ordon sword, shield)
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xD1] |= 2;  // ZA
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xD2] |=
            (128 | 16 | 4 | 1);  // hero cloth, hylian shield, ordon shield, ordon sword

        // Give bottles 2 * fairy tear
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xA7] = 115;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xA8] = 115;

        // Item wheel bottles
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 11;
        itemWheelPos++;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 12;
        itemWheelPos++;

        // Give ending blow only
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.eventBits[0x29] = 0x24;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.eventBits[0x2A] = 0x00;
    }
    void diababa()
    {
        // Add gale
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0x9C] = 0x40;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 0;
        itemWheelPos++;
    }
    void dangoro()
    {
        // Iron boots
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0x9F] = 0x45;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 0x3;
        itemWheelPos++;
    }
    void fyrus()
    {
        // Add Heroe's Bow + ammo
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xEC] = 30;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xA0] = 0x43;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 0x4;
        itemWheelPos++;
    }

    void dekutoad()
    {
        // Add Waterbombbag + ammo
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xED] = 15;  // Ammo bomb bag 1
        // Bomb Bag + Waterbombs in bomb bag slot 1
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xAB] = 0x71;
        // Reference id for bomb bag slot 1
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 15;
        itemWheelPos++;
    }

    void morpheel()
    {
        // Add clawshot
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xA5] = 0x44;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 0x9;

        clawshotItemWheelIndex = itemWheelPos;
        itemWheelPos++;
    }

    void deathsword()
    {
        // Set transform flag
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.eventBits[0xD] |= 0x4;
        // Give midna
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.eventBits[0xC] |= 0x10;
        // Give senses
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.eventBits[0x43] |= 0x8;

        // MDH Flag (or white midna but that's sad)
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.eventBits[0x1E] |= 0x8;

        // midna on back
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0x30] |= 0x8;

        // Midna Charge
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.eventBits[0x5] |= 0x1;

        // Set Master sword inventory flag
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xD2] |= 0x2;

        // Equip Master sword (0x49 / 73)
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0x014] = 0x49;
    }
    void stallord()
    {
        // Give spinner
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0x09E] = 0x41;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 0x2;
        itemWheelPos++;
    }

    void darkhammer()
    {
        // Nothing I guess
    }
    void blizetta()
    {
        // Give ball and chain
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0x0A2] = 0x42;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 0x6;
        itemWheelPos++;
    }
    void darknut()
    {
        // Nothing I guess
    }
    void armogohma()
    {
        // Dominion rod
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0x0A4] = 0x46;
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + itemWheelPos] = 0x8;
        itemWheelPos++;
    }
    void arealfos()
    {
        // Nothing I guess
    }
    void argarok()
    {
        // Purge old clawshot from items
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xA5] = 0xFF;

        // Add 2nd clawshot to items
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xA6] = 0x47;

        // Set Double Clawshot into itemwheel position where clawshot was
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB4 + clawshotItemWheelIndex] = 0xA;

        // Remove from X/Y equip
        u8* x = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xB];
        u8* y = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xC];

        if (*x == 9)
            *x = 0xFF;
        else if (*y == 9)
            *y = 0xFF;
    }
    void phantomZant()
    {
        // Nothing I think
    }
    void zant()
    {
        // Light sword
        libtp::tp::d_com_inf_game::dComIfG_gameInfo.scratchPad.wQuestLogData[0xD6] = 0xFF;
    }
    void ganon()
    {
        // Nothing I guess
    }
}  // namespace mod