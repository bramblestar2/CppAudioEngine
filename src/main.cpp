#include <iostream>
#include "Audio/AudioManager.h"

int main()
{
    AudioManager audioManager;
    audioManager.load("C:\\Users\\thega\\Music\\Sharks - Opal (Official Music Video) [Disciple].wav");

    std::cin.get();

    return 0;
}
