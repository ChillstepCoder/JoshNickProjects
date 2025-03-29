// WWiseAudioEngine.h
#pragma once
#include <windows.h>
#include <iostream>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AkFilePackageLowLevelIO.h>
#include <AkFilePackageLowLevelIODeferred.h>
#include <AkFilePackage.h>
#include <AkFilePackageLUT.h>
#include <Wwise_IDs.h>
#include <string>

// Default paths - will be overridden if provided in init()
#define DEFAULT_WWISE_BANK_PATH AKTEXT("../WwiseProjects/RacingGame/GeneratedSoundBanks/Windows/")
#define DEFAULT_BANKNAME_INIT L"Init.bnk"
#define DEFAULT_BANKNAME_MAIN L"Main.bnk"

namespace JAGEngine {
    class WWiseAudioEngine
    {
    public:
        WWiseAudioEngine();
        ~WWiseAudioEngine();

        // Initialize with default paths
        bool init();

        // Initialize with custom paths
        bool init(const AkOSChar* bankPath, const wchar_t* initBankName = DEFAULT_BANKNAME_INIT,
            const wchar_t* mainBankName = DEFAULT_BANKNAME_MAIN);

        // Initialize with project name (formats path automatically)
        bool initWithProject(const std::string& projectName,
            const wchar_t* initBankName = DEFAULT_BANKNAME_INIT,
            const wchar_t* mainBankName = DEFAULT_BANKNAME_MAIN);

        void update();
        void cleanup();
        bool isInitialized() const { return m_isInitialized; }

        // Getters for current paths
        const AkOSChar* getBankPath() const { return m_bankPath; }
        const wchar_t* getInitBankName() const { return m_initBankName; }
        const wchar_t* getMainBankName() const { return m_mainBankName; }

    private:
        // Common initialization code shared by all init methods
        bool initializeEngine();
        bool loadBanks();

        bool m_isInitialized = false;
        CAkFilePackageLowLevelIODeferred m_lowLevelIO;
        uint32_t m_frameCounter = 0;

        // Paths storage
        AkOSChar m_bankPath[256];
        wchar_t m_initBankName[64];
        wchar_t m_mainBankName[64];
    };
}