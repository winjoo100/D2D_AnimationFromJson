#pragma once

#include <filesystem>
#include "AnimationClip.h" 

using AnimationClips = std::vector<std::pair<std::string, AnimationClip>>;
class AsepriteParser
{
public:
    AsepriteParser() = default;
    ~AsepriteParser() = default;

    // JSON 경로를 받아서 애니메이션 클립들 반환
    static AnimationClips Load(const std::filesystem::path& jsonPath);
};


