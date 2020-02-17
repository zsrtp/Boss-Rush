#include "main.h"

#include <display/console.h>
#include <tp/JFWSystem.h>

#include <cstdint>
#include <cstring>

namespace mod
{
    void main()
    {
        libtp::display::Console console{"AECX",
                                        "Boss Rush",
                                        "Fast pace fighting action, beat boss after boss",
                                        "as quickly as possible. At the end of each fight",
                                        "you'll be teleported to the next one...",
                                        "Version: 0.1a"};
        return;
    }
}  // namespace mod