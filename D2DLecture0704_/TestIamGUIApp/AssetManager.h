#pragma once

#include <unordered_map>
#include <filesystem>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <d2d1_1.h>
#include "AnimationClip.h"

// 예시입니다.
class AssetManager
{
public:
    using AnimationClips = std::vector<std::pair<std::string, AnimationClip>>;
   
    AssetManager() = default;
    ~AssetManager() = default;

    ////////////////////////////////////////////////////////////////////////////////
    // 애니메이션 ( JSON )
    ////////////////////////////////////////////////////////////////////////////////
    // 
    // 텍스처 로드
    //
    void LoadTexture(ID2D1DeviceContext7* d2dContext,
                        const std::wstring& key,
                        const std::filesystem::path& filepath);


    ////////////////////////////////////////////////////////////////////////////////
    // 
    // JSON 파일 로드 -> 애니메이션 클립 생성
    //
    void LoadAseprite(ID2D1DeviceContext7* d2dContext,
                        const std::wstring& key,
                        const std::filesystem::path& jsonPath);


    ////////////////////////////////////////////////////////////////////////////////
    // 
    // 키로 클립 조회 (없으면 빈 벡터 반환)
    //
    const AnimationClips& GetClips(const std::wstring& key) const;


    ////////////////////////////////////////////////////////////////////////////////
    // 단일 Sprite ( PNG )
    // 
    // PNG 로드
    //
    void LoadSpriteTexture(ID2D1DeviceContext7* d2dContext, const std::wstring& key, const std::filesystem::path& filepath);


    ////////////////////////////////////////////////////////////////////////////////
    //
    // 단일 PNG 조회
    //
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> GetSpriteTexture(const std::wstring& key) const;


private:

    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID2D1Bitmap1>> m_textures;
    std::unordered_map<std::wstring, AnimationClips> m_clipsMap;

    // 단일 PNG 이미지 전용 텍스처 맵 (키와 비트맵)
    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID2D1Bitmap1>> m_spriteTextures;
};

