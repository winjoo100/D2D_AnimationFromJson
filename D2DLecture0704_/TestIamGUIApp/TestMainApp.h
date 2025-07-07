#pragma once

#include "NzWndBase.h"
#include "GameTimer.h"
#include "AnimationClip.h"
#include "AssetManager.h"
#include "Sprite.h"

#include <wrl/client.h>


class GameTimer;
class Sprite;

namespace sample
{
    class D2DRenderer;
}

class TestMainApp : public NzWndBase
{
public:
    TestMainApp() = default;
    virtual ~TestMainApp() = default;

    bool Initialize();
    void Run();
    void Finalize();

    bool OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:

    void UpdateTime();
    void UpdateInput();
    void UpdateLogic();

    void Render();
    void RenderImGUI();

    void LoadAssets();

    void OnResize(int width, int height) override;
    void OnClose() override;

    void BrowseForFolder();
    void UpdateFileList();

    // 2025.07.04 멤버함수(장난감) 추가 _ 백승주
    // 애니메이션 배율 업데이트
    void UpdateAnimationPlaybackSpeed(float speed);

    ////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<sample::D2DRenderer> m_Renderer = nullptr;

    GameTimer       m_GameTimer;
    AssetManager    m_AssetManager;


    std::string     m_pathInput;
    std::wstring    m_folderPath;
    std::wstring    m_selectedFile;

    std::wstring    m_selectedAssetKey;

    std::vector<SpriteAnimator> m_curSprites;

    std::vector<std::wstring> m_fileList;

    bool m_showFolderPanel = false;
    bool m_bChangedFile = false;

    std::shared_ptr<Sprite> m_mySprite;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_singleSpriteTexture;



    ////////////////////////////////////////////////////////////////////////////////
    // 2025.07.04 멤버변수(장난감) 추가 _ 백승주
    ////////////////////////////////////////////////////////////////////////////////
    //
    // ImGui로 조절할 값
    //
    float m_singleSpriteWidth = 100.f;
    float m_singleSpriteHeight = 100.f;

    float m_singleSpritePosX = 100.f;
    float m_singleSpritePosY = 100.f;

    float m_animationDuration = 0.1f; // 예시 기본값 (초)
};
