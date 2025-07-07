#pragma once

#include <unordered_map>
#include <filesystem>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <d2d1_1.h>
#include "AnimationClip.h"

// �����Դϴ�.
class AssetManager
{
public:
    using AnimationClips = std::vector<std::pair<std::string, AnimationClip>>;
   
    AssetManager() = default;
    ~AssetManager() = default;

    ////////////////////////////////////////////////////////////////////////////////
    // �ִϸ��̼� ( JSON )
    ////////////////////////////////////////////////////////////////////////////////
    // 
    // �ؽ�ó �ε�
    //
    void LoadTexture(ID2D1DeviceContext7* d2dContext,
                        const std::wstring& key,
                        const std::filesystem::path& filepath);


    ////////////////////////////////////////////////////////////////////////////////
    // 
    // JSON ���� �ε� -> �ִϸ��̼� Ŭ�� ����
    //
    void LoadAseprite(ID2D1DeviceContext7* d2dContext,
                        const std::wstring& key,
                        const std::filesystem::path& jsonPath);


    ////////////////////////////////////////////////////////////////////////////////
    // 
    // Ű�� Ŭ�� ��ȸ (������ �� ���� ��ȯ)
    //
    const AnimationClips& GetClips(const std::wstring& key) const;


    ////////////////////////////////////////////////////////////////////////////////
    // ���� Sprite ( PNG )
    // 
    // PNG �ε�
    //
    void LoadSpriteTexture(ID2D1DeviceContext7* d2dContext, const std::wstring& key, const std::filesystem::path& filepath);


    ////////////////////////////////////////////////////////////////////////////////
    //
    // ���� PNG ��ȸ
    //
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> GetSpriteTexture(const std::wstring& key) const;


private:

    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID2D1Bitmap1>> m_textures;
    std::unordered_map<std::wstring, AnimationClips> m_clipsMap;

    // ���� PNG �̹��� ���� �ؽ�ó �� (Ű�� ��Ʈ��)
    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID2D1Bitmap1>> m_spriteTextures;
};

