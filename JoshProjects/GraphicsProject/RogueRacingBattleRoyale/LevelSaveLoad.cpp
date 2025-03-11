// LevelSaveLoad.cpp

#include "LevelSaveLoad.h"
#include <fstream>
#include <algorithm>
#include <regex>
#include <iostream>
#include <direct.h>
#include <io.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

const std::string LevelSaveLoad::LEVELS_DIRECTORY = "levels/";

bool LevelSaveLoad::saveLevel(const SavedLevel& level) {
  // Create directory if it doesn't exist
  _mkdir(LEVELS_DIRECTORY.c_str());

  std::string filename = LEVELS_DIRECTORY + constructFilename(level.difficulty, level.name);
  std::ofstream file(filename, std::ios::binary);

  if (!file.is_open()) {
    std::cout << "Failed to open file for writing: " << filename << std::endl;
    return false;
  }

  // Write difficulty and name
  file.write(reinterpret_cast<const char*>(&level.difficulty), sizeof(int));
  size_t nameLength = level.name.length();
  file.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
  file.write(level.name.c_str(), nameLength);

  // Write track nodes
  size_t nodeCount = level.nodes.size();
  file.write(reinterpret_cast<const char*>(&nodeCount), sizeof(size_t));
  for (const auto& node : level.nodes) {
    auto pos = node.getPosition();
    float roadWidth = node.getRoadWidth();
    auto offroadWidth = node.getOffroadWidth();
    auto barrierDist = node.getBarrierDistance();
    bool isStartLine = node.isStartLine();

    file.write(reinterpret_cast<const char*>(&pos), sizeof(glm::vec2));
    file.write(reinterpret_cast<const char*>(&roadWidth), sizeof(float));
    file.write(reinterpret_cast<const char*>(&offroadWidth), sizeof(glm::vec2));
    file.write(reinterpret_cast<const char*>(&barrierDist), sizeof(glm::vec2));
    file.write(reinterpret_cast<const char*>(&isStartLine), sizeof(bool));
  }

  // Write objects
  size_t objectCount = level.objects.size();
  file.write(reinterpret_cast<const char*>(&objectCount), sizeof(size_t));
  for (const auto& obj : level.objects) {
    file.write(reinterpret_cast<const char*>(&obj.templateIndex), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&obj.position), sizeof(glm::vec2));
  }

  // Write track settings
  file.write(reinterpret_cast<const char*>(&level.startConfig), sizeof(SplineTrack::StartPositionConfig));
  file.write(reinterpret_cast<const char*>(&level.grassColor), sizeof(glm::vec3));
  file.write(reinterpret_cast<const char*>(&level.offroadColor), sizeof(glm::vec3));
  file.write(reinterpret_cast<const char*>(&level.grassNoiseScale), sizeof(float));
  file.write(reinterpret_cast<const char*>(&level.grassNoiseIntensity), sizeof(float));
  file.write(reinterpret_cast<const char*>(&level.barrierPrimaryColor), sizeof(glm::vec3));
  file.write(reinterpret_cast<const char*>(&level.barrierSecondaryColor), sizeof(glm::vec3));
  file.write(reinterpret_cast<const char*>(&level.barrierPatternScale), sizeof(float));
  file.write(reinterpret_cast<const char*>(&level.musicTrackId), sizeof(AkUniqueID));
  file.write(reinterpret_cast<const char*>(&level.roadLOD), sizeof(int));

  file.close();
  return true;
}

std::vector<LevelSaveLoad::LevelMetadata> LevelSaveLoad::getLevelList() {
  std::vector<LevelMetadata> levels;

  // Check if directory exists
  if (_access(LEVELS_DIRECTORY.c_str(), 0) != 0) {
    return levels;
  }

  // Get search path
  std::string searchPath = LEVELS_DIRECTORY + "*.txt";

  // Convert to wide string for Windows API
  wchar_t wSearchPath[MAX_PATH];
  MultiByteToWideChar(CP_UTF8, 0, searchPath.c_str(), -1, wSearchPath, MAX_PATH);

  WIN32_FIND_DATAW findData;
  HANDLE hFind = FindFirstFileW(wSearchPath, &findData);

  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      // Convert filename back to narrow string
      char filename[MAX_PATH];
      WideCharToMultiByte(CP_UTF8, 0, findData.cFileName, -1,
        filename, MAX_PATH, NULL, NULL);

      std::regex levelPattern(R"((\d+)_([^\.]+)\.txt)");
      std::smatch matches;
      std::string filenameStr(filename);

      if (std::regex_match(filenameStr, matches, levelPattern)) {
        LevelMetadata metadata;
        metadata.filename = filenameStr;
        metadata.difficulty = std::stoi(matches[1].str());
        metadata.levelName = matches[2].str();
        levels.push_back(metadata);
      }
    } while (FindNextFileW(hFind, &findData) != 0);
    FindClose(hFind);
  }

  std::sort(levels.begin(), levels.end(),
    [](const LevelMetadata& a, const LevelMetadata& b) {
      if (a.difficulty != b.difficulty) {
        return a.difficulty < b.difficulty;
      }
      return a.levelName < b.levelName;
    });

  return levels;
}

bool LevelSaveLoad::loadLevel(const std::string& filename, SavedLevel& outLevel) {
  std::ifstream file(LEVELS_DIRECTORY + filename, std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Failed to open file for reading: " << filename << std::endl;
    return false;
  }

  // Read difficulty and name
  file.read(reinterpret_cast<char*>(&outLevel.difficulty), sizeof(int));
  size_t nameLength;
  file.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
  outLevel.name.resize(nameLength);
  file.read(&outLevel.name[0], nameLength);

  // Read nodes
  size_t nodeCount;
  file.read(reinterpret_cast<char*>(&nodeCount), sizeof(size_t));
  outLevel.nodes.clear();

  for (size_t i = 0; i < nodeCount; i++) {
    glm::vec2 pos;
    float roadWidth;
    glm::vec2 offroadWidth;
    glm::vec2 barrierDist;
    bool isStartLine;

    file.read(reinterpret_cast<char*>(&pos), sizeof(glm::vec2));
    file.read(reinterpret_cast<char*>(&roadWidth), sizeof(float));
    file.read(reinterpret_cast<char*>(&offroadWidth), sizeof(glm::vec2));
    file.read(reinterpret_cast<char*>(&barrierDist), sizeof(glm::vec2));
    file.read(reinterpret_cast<char*>(&isStartLine), sizeof(bool));

    TrackNode node(pos);
    node.setRoadWidth(roadWidth);
    node.setOffroadWidth(offroadWidth);
    node.setBarrierDistance(barrierDist);
    if (isStartLine) {
      // The track will handle setting this properly when loaded
      node.setStartLine(true);
    }
    outLevel.nodes.push_back(node);
  }

  // Read objects
  size_t objectCount;
  file.read(reinterpret_cast<char*>(&objectCount), sizeof(size_t));
  outLevel.objects.clear();

  for (size_t i = 0; i < objectCount; i++) {
    SavedLevel::SavedObject obj;
    file.read(reinterpret_cast<char*>(&obj.templateIndex), sizeof(size_t));
    file.read(reinterpret_cast<char*>(&obj.position), sizeof(glm::vec2));
    outLevel.objects.push_back(obj);
  }

  // Read track settings
  file.read(reinterpret_cast<char*>(&outLevel.startConfig), sizeof(SplineTrack::StartPositionConfig));
  file.read(reinterpret_cast<char*>(&outLevel.grassColor), sizeof(glm::vec3));
  file.read(reinterpret_cast<char*>(&outLevel.offroadColor), sizeof(glm::vec3));
  file.read(reinterpret_cast<char*>(&outLevel.grassNoiseScale), sizeof(float));
  file.read(reinterpret_cast<char*>(&outLevel.grassNoiseIntensity), sizeof(float));
  file.read(reinterpret_cast<char*>(&outLevel.barrierPrimaryColor), sizeof(glm::vec3));
  file.read(reinterpret_cast<char*>(&outLevel.barrierSecondaryColor), sizeof(glm::vec3));
  file.read(reinterpret_cast<char*>(&outLevel.barrierPatternScale), sizeof(float));
  file.read(reinterpret_cast<char*>(&outLevel.musicTrackId), sizeof(AkUniqueID));
  file.read(reinterpret_cast<char*>(&outLevel.roadLOD), sizeof(int));

  return true;
}

std::string LevelSaveLoad::sanitizeFilename(const std::string& input) {
  std::string result = input;

  // Replace spaces with underscores
  std::replace(result.begin(), result.end(), ' ', '_');

  // Remove any non-alphanumeric characters (except underscores)
  result.erase(
    std::remove_if(result.begin(), result.end(),
      [](char c) {
        return !(std::isalnum(c) || c == '_');
      }),
    result.end()
  );

  return result;
}

std::string LevelSaveLoad::constructFilename(int difficulty, const std::string& name) {
  return std::to_string(difficulty) + "_" + sanitizeFilename(name) + ".txt";
}
