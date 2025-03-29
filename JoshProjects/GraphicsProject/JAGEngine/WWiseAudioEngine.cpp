// WWiseAudioEngine.cpp
#include "WWiseAudioEngine.h"
#include "JAGErrors.h"
#include <cstring>

namespace JAGEngine {

    WWiseAudioEngine::WWiseAudioEngine() {
        // Initialize paths with defaults
        wcsncpy_s(m_bankPath, 256, DEFAULT_WWISE_BANK_PATH, _TRUNCATE);
        wcsncpy_s(m_initBankName, 64, DEFAULT_BANKNAME_INIT, _TRUNCATE);
        wcsncpy_s(m_mainBankName, 64, DEFAULT_BANKNAME_MAIN, _TRUNCATE);
    }

    WWiseAudioEngine::~WWiseAudioEngine() {
        if (m_isInitialized) {
            cleanup();
        }
    }

    bool WWiseAudioEngine::init() {
        // Use default paths that were set in constructor
        return initializeEngine();
    }

    bool WWiseAudioEngine::init(const AkOSChar* bankPath, const wchar_t* initBankName, const wchar_t* mainBankName) {
        // Set custom paths
        wcsncpy_s(m_bankPath, 256, bankPath, _TRUNCATE);
        wcsncpy_s(m_initBankName, 64, initBankName, _TRUNCATE);
        wcsncpy_s(m_mainBankName, 64, mainBankName, _TRUNCATE);

        return initializeEngine();
    }

    bool WWiseAudioEngine::initWithProject(const std::string& projectName,
        const wchar_t* initBankName,
        const wchar_t* mainBankName) {
        // Format the bank path based on project name
        std::string basePath = "../WwiseProjects/" + projectName + "/GeneratedSoundBanks/Windows/";

        // Convert from UTF-8 to wide char
        wchar_t formattedPath[256];
        MultiByteToWideChar(CP_UTF8, 0, basePath.c_str(), -1, formattedPath, 256);

        // Call the regular init with our formatted path
        return init(formattedPath, initBankName, mainBankName);
    }

    bool WWiseAudioEngine::initializeEngine() {
        if (m_isInitialized) {
            fatalError("AK: Tried to initialize Audio Engine twice!");
            return false;
        }

        // Initialize Memory Manager
        AkMemSettings memSettings;
        AK::MemoryMgr::GetDefaultSettings(memSettings);
        if (AK::MemoryMgr::Init(&memSettings) != AK_Success) {
            std::cout << "AK: Memory Manager Failed to Initialize." << std::endl;
            return false;
        }
        std::cout << "AK: Memory Manager Initialized." << std::endl;

        // Initialize Stream Manager
        AkStreamMgrSettings stmSettings;
        AK::StreamMgr::GetDefaultSettings(stmSettings);
        if (!AK::StreamMgr::Create(stmSettings)) {
            std::cout << "AK: Stream Manager Failed to Create." << std::endl;
            return false;
        }
        std::cout << "AK: Stream Manager Created Successfully." << std::endl;

        // Initialize Streaming Device
        AkDeviceSettings deviceSettings;
        AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);
        if (m_lowLevelIO.Init(deviceSettings) != AK_Success) {
            std::cout << "AK: Could not create the streaming device and Low-Level I/O system." << std::endl;
            return false;
        }
        std::cout << "AK: Created the streaming device and Low-Level I/O system." << std::endl;

        // Initialize Sound Engine
        AkInitSettings initSettings;
        AkPlatformInitSettings platformInitSettings;
        AK::SoundEngine::GetDefaultInitSettings(initSettings);
        AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

        if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success) {
            std::cout << "AK: Could not initialize the Sound Engine." << std::endl;
            return false;
        }
        std::cout << "AK: Sound Engine Initialized!" << std::endl;

        // Set up bank path and language
        m_lowLevelIO.SetBasePath(m_bankPath);
        AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));

        // Now load the banks
        if (!loadBanks()) {
            return false;
        }

        // Set up default listener
        const AkGameObjectID DEFAULT_LISTENER_ID = 0;  // Using 0 as default listener
        AKRESULT listenerResult = AK::SoundEngine::RegisterGameObj(DEFAULT_LISTENER_ID, "DefaultListener");
        if (listenerResult != AK_Success) {
            std::cout << "Failed to register default listener" << std::endl;
            return false;
        }

        AKRESULT outputResult = AK::SoundEngine::SetDefaultListeners(&DEFAULT_LISTENER_ID, 1);
        if (outputResult != AK_Success) {
            std::cout << "Failed to set default listener" << std::endl;
            return false;
        }

        m_isInitialized = true;
        return true;
    }

    bool WWiseAudioEngine::loadBanks() {
        AkBankID bankID;
        AKRESULT result;

        // Load Init bank first
        result = AK::SoundEngine::LoadBank(m_initBankName, bankID);
        if (result != AK_Success) {
            std::cout << "Failed to load Init bank. Error code: " << result << std::endl;
            std::wcout << L"Bank path: " << m_bankPath << L", Init bank: " << m_initBankName << std::endl;
            return false;
        }
        std::cout << "Init Bank loaded successfully." << std::endl;

        // Load Main bank
        result = AK::SoundEngine::LoadBank(m_mainBankName, bankID);
        if (result != AK_Success) {
            std::cout << "Failed to load Main bank. Error code: " << result << std::endl;
            std::wcout << L"Bank path: " << m_bankPath << L", Main bank: " << m_mainBankName << std::endl;
            return false;
        }
        std::cout << "Main Bank loaded successfully." << std::endl;

        return true;
    }

    void WWiseAudioEngine::update() {
        if (m_isInitialized) {
            AK::SoundEngine::RenderAudio();
        }
    }

    void WWiseAudioEngine::cleanup() {
        if (m_isInitialized) {
            AK::SoundEngine::Term();

            m_lowLevelIO.Term();
            if (AK::IAkStreamMgr::Get())
                AK::IAkStreamMgr::Get()->Destroy();

            AK::MemoryMgr::Term();

            m_isInitialized = false;
        }
    }
}