#include "pch.h"
#include <fstream>
#include "json.hpp"
#include "AsepriteParser.h"
using json = nlohmann::json;

AnimationClips AsepriteParser::Load(const std::filesystem::path& jsonPath)
{
    AnimationClips clips;

    // 1. JSON 파일 읽기
    std::ifstream file(jsonPath);
    if (!file.is_open())
        throw std::runtime_error("Json 파일을 열 수 없습니다.");

    json j;
    file >> j;

    // 2. frameTags 읽어서 AnimationClip 생성
    if (!j.contains("meta") || !j["meta"].contains("frameTags"))
        throw std::runtime_error("frameTags 없음");

    const auto& frames = j["frames"];

    for (const auto& tag : j["meta"]["frameTags"])
    {
        AnimationClip clip;

        int from = tag["from"].get<int>();
        int to = tag["to"].get<int>();

        for (int i = from; i <= to; ++i)
        {
            const auto& frameData = frames[i];

            Frame frame;
            frame.srcRect.left = frameData["frame"]["x"].get<UINT>();
            frame.srcRect.top = frameData["frame"]["y"].get<UINT>();
            frame.srcRect.right = frame.srcRect.left + frameData["frame"]["w"].get<UINT>();
            frame.srcRect.bottom = frame.srcRect.top + frameData["frame"]["h"].get<UINT>();
            frame.duration = frameData["duration"].get<float>() / 1000.0f; // ms → s

            clip.AddFrame(frame);
        }


        std::string name = tag["name"].get<std::string>();
        clips.emplace_back(name, std::move(clip));
    }

    return clips;
}
