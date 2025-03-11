// LevelSaveLoad.h

#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "SplineTrack.h"
#include "ObjectManager.h"
#include <AK/SoundEngine/Common/AkTypes.h>

class LevelSaveLoad {
public:
  struct LevelMetadata {
    std::string filename;
    std::string levelName;
    int difficulty;
  };

  struct SavedLevel {
    int difficulty;
    std::string name;
    std::vector<TrackNode> nodes;
    struct SavedObject {
      size_t templateIndex;
      glm::vec2 position;
    };
    std::vector<SavedObject> objects;
    SplineTrack::StartPositionConfig startConfig;
    glm::vec3 grassColor;
    glm::vec3 offroadColor;
    float grassNoiseScale;
    float grassNoiseIntensity;
    glm::vec3 barrierPrimaryColor;
    glm::vec3 barrierSecondaryColor;
    float barrierPatternScale;
    AkUniqueID musicTrackId = 0;
    int roadLOD;
  };


  static bool saveLevel(const SavedLevel& level);
  static bool loadLevel(const std::string& filename, SavedLevel& outLevel);
  static std::vector<LevelMetadata> getLevelList();
  static std::string sanitizeFilename(const std::string& input);
  static std::string constructFilename(int difficulty, const std::string& name);

private:
  static const std::string LEVELS_DIRECTORY;
  
};
