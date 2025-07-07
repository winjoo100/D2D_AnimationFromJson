#include "pch.h"
#include "GameTimer.h"
#include "D2DRender.h"

////////////////////////////////////////////////////////////////////////////
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////
#include <shobjidl.h>            // IFileOpenDialog
#include <filesystem>            // C++17 std::filesystem

/////////////////////////////////////////////////////////////////////////
#include "TestMainApp.h"

using AnimationClips = std::vector<std::pair<std::string, AnimationClip>>;
using namespace sample;
/////////////////////////////////////////////////////////////////////////////
//
std::wstring ConvertToWString(const std::string& str)
{
    size_t len = 0;
    mbstowcs_s(&len, nullptr, 0, str.c_str(), _TRUNCATE);
    if (len == 0)
        return L"";

    std::wstring wstr(len, L'\0');
    mbstowcs_s(&len, &wstr[0], len, str.c_str(), _TRUNCATE);
    wstr.resize(len - 1); // Remove the null terminator added by mbstowcs_s  
    return wstr;
}

std::string WStringToString(const std::wstring& wstr)
{
    size_t len = 0;
    wcstombs_s(&len, nullptr, 0, wstr.c_str(), _TRUNCATE);
    if (len == 0)
        return "";
    std::string str(len, '\0');
    wcstombs_s(&len, &str[0], len, wstr.c_str(), _TRUNCATE);
    str.resize(len - 1); // Remove the null terminator added by wcstombs_s
    return str;
}


/////////////////////////////////////////////////////////////////////////////
// 
bool TestMainApp::Initialize()
{
    const wchar_t* className = L"D2DLesson2";
    const wchar_t* windowName = L"D2DLesson2";

    if (false == __super::Create(className, windowName, 1024, 800))
    {
        return false;
    }

    m_Renderer = std::make_shared<D2DRenderer>();
    m_Renderer->Initialize(m_hWnd);

    // [ImGUI] ���ؽ�Ʈ & �鿣�� �ʱ�ȭ
    // 3-1) ImGui ���ؽ�Ʈ ����
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    
    ImGui_ImplWin32_Init(m_hWnd);

    ID3D11Device* pd3dDevice = m_Renderer->GetD3DDevice();

    // 2) ��� ���ؽ�Ʈ ���
    ID3D11DeviceContext* pd3dDeviceContext = nullptr;
    pd3dDeviceContext = m_Renderer->GetD3DContext();

    // [ImGUI] DirectX 11 �鿣�� �ʱ�ȭ
    ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);

    // Ÿ�̸� �ʱ�ȭ
    m_GameTimer.Reset();

    return true;
}

void TestMainApp::Run()
{
    MSG msg = { 0 };

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
               
            DispatchMessage(&msg);
        }
        else
        {
            UpdateTime();
            UpdateInput();
            UpdateLogic();
            Render();        
        }
    }
}
void TestMainApp::Finalize()
{
    // [ImGUI] DirectX 11 �鿣�� ����
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (m_Renderer != nullptr)
    {
        m_Renderer->Uninitialize();
        m_Renderer.reset();
    }
}

bool TestMainApp::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    {
        return true; // ImGui�� �޽����� ó�������� true ��ȯ
    }

    return false;
}


void TestMainApp::UpdateTime()
{
    m_GameTimer.Tick();
}

void TestMainApp::UpdateInput()
{
}

void TestMainApp::UpdateLogic()
{
    LoadAssets();

    if (m_selectedAssetKey.empty()) return; // ���õ� �ּ� Ű�� ��������� ����

    const auto& clips = m_AssetManager.GetClips(m_selectedAssetKey);
    if (clips.empty())
        return;

    if (m_bChangedFile)
    {
        m_curSprites.clear();
        m_bChangedFile = false;

        // Ŭ������ SpriteAnimator ����
        for (const auto& [name, clip] : clips)
        {
            SpriteAnimator anim;
            anim.SetClip(&clip);
            anim.SetLooping(true);
            m_curSprites.push_back(anim);
        }
    }

    // �ִϸ��̼� �÷��̾� ������Ʈ
    for (auto& sprite : m_curSprites)
    {
        sprite.Update(m_GameTimer.DeltaTime());
    }
}


void TestMainApp::Render()
{
    if (m_Renderer == nullptr) return;

   
    m_Renderer->RenderBegin();

    int count = m_curSprites.size();
    // ���� �ִϸ��̼��� ��� �����ִ� ������
    // ȭ�� �߾��� �������� �ִϸ��̼��� ����
    int animationIndex = 0;
    for (auto& ap : m_curSprites)
    {
        if (ap.IsValid() == false) continue; // ��ȿ���� ���� �÷��̾�� ��ŵ

        const Frame& frame = ap.GetCurrentFrame();

        int xOffset = static_cast<float>(animationIndex * frame.Width()) + 200.f;
        int yOffset = 300;
       
        D2D1_RECT_F renderRect = D2D1::RectF(xOffset, yOffset, xOffset + frame.Width(), yOffset + frame.Height());
        
        m_Renderer->DrawBitmap(ap.GetClip()->GetBitmap(), renderRect, frame.ToRectF());

        animationIndex++;
    }

    // Render()�� ���� PNG �׸��� �߰� (�ִϸ��̼� Ŭ�� ������ ���� �Ǵ� �Ŀ�)
    if (m_singleSpriteTexture)
    {
        // �гο� �ִ� �����̴��� RECT ����
        D2D1_RECT_F rect = D2D1::RectF(
            m_singleSpritePosX,
            m_singleSpritePosY,
            m_singleSpritePosX + m_singleSpriteWidth,
            m_singleSpritePosY + m_singleSpriteHeight);

        m_Renderer->DrawBitmap(m_singleSpriteTexture.Get(), rect);
    }

    m_Renderer->RenderEnd(false);

    RenderImGUI();

    // Present, ImGUI ������ enddraw �� �и�
    m_Renderer->Present();
}


void TestMainApp::RenderImGUI()
{
    ID3D11DeviceContext* pd3dDeviceContext = nullptr;
    pd3dDeviceContext = m_Renderer->GetD3DContext();
    ID3D11RenderTargetView* rtvs[] = { m_Renderer->GetD3DRenderTargetView() };

    if (pd3dDeviceContext == nullptr || rtvs[0] == nullptr)
    {
        return; // ������ ���ؽ�Ʈ�� �䰡 ������ ����
    }
    m_Renderer->GetD3DContext()->OMSetRenderTargets(1, rtvs, nullptr);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ���� UI â ǥ��
    //static bool showDemo = true;
    //ImGui::ShowDemoWindow(&showDemo);

    ////////////////////////////////////////////////////////////////////////////////
    //
    // ���� �޴���
    //
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // �޴� �������� ������ �г� ���� �÷��� ���
            if (ImGui::MenuItem("Open Folder", "Ctrl+O"))
            {
                m_showFolderPanel = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // ��޸��� �гη� ���� : ���� ������ + ��������Ʈ ��Ʈ�ѷ� ��ħ
    //
    if (m_showFolderPanel)
    {
        ImGui::Begin("Folder Browser & Sprite Control", &m_showFolderPanel, ImGuiWindowFlags_AlwaysAutoResize);

        ////////////////////////////////////////////////////////////////////////////////
        // 
        // ���� ������ �κ� __
        //
        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::Button("Browse..."))
        {
            BrowseForFolder();
            m_pathInput = std::filesystem::path(m_folderPath).u8string();
        }

        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            if (false == m_folderPath.empty())
            {
                UpdateFileList();
                m_selectedFile.clear();
            }
        }

        ImGui::Text("Folder: %ls", m_folderPath.c_str());

        // ���� ��� ������
        if (false == m_folderPath.empty())
        {
            if (ImGui::BeginListBox("Files", ImVec2(300.0f, 8 * ImGui::GetTextLineHeightWithSpacing())))
            {
                for (const auto& name : m_fileList)
                {
                    bool isSelected = (m_selectedFile == name);
                    if (ImGui::Selectable(WStringToString(name).c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        m_selectedFile = name;
                    }

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                    {
                        m_selectedFile = name;
                    }
                }

                ImGui::EndListBox();
            }

            if (!m_selectedFile.empty())
            {
                ImGui::Text("Selected File: %ls", m_selectedFile.c_str());
            }
        }

        ImGui::Separator(); // ���м�



        ////////////////////////////////////////////////////////////////////////////////
        //
        // ���� PNG ũ��
        //
        ////////////////////////////////////////////////////////////////////////////////
        
        // Sprite(PNG) Control ���� �߰� (����)
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sprite Control");  // ����� �ؽ�Ʈ (��������)
        
        // ũ�� ���� �����̴� �߰�
        ImGui::SliderFloat("Sprite Width", &m_singleSpriteWidth, 10.f, 500.f);
        ImGui::SliderFloat("Sprite Height", &m_singleSpriteHeight, 10.f, 500.f);

        // ��ġ ���� �����̴� �߰� (X, Y ��ǥ)
        ImGui::SliderFloat("Sprite Position X", &m_singleSpritePosX, 0.f, 800.f);
        ImGui::SliderFloat("Sprite Position Y", &m_singleSpritePosY, 0.f, 600.f);

        ImGui::Separator(); // ���м�



        ////////////////////////////////////////////////////////////////////////////////
        //
        // �ִϸ��̼� �ӵ� ����
        //
        ////////////////////////////////////////////////////////////////////////////////
        
        // �ִϸ��̼� �ӵ� ���� �����̴� (��: ù ��° SpriteAnimator ���)
        if (!m_curSprites.empty())
        {
            // Animation(JSON) Control ���� �߰� (����)
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Animation Control");  // ����� �ؽ�Ʈ (��������)

            // ù ��° �ִϸ��̼� �ӵ��� �������� �ʱ� �����̴� �� ����
            static float playbackSpeed = m_curSprites[0].GetPlaybackSpeed();

            if (ImGui::SliderFloat("Animation Playback Speed", &playbackSpeed, 0.1f, 3.0f, "%.2f"))
            {
                // ��� SpriteAnimator�� �ӵ� ����
                UpdateAnimationPlaybackSpeed(playbackSpeed);
            }
        }


        ImGui::End(); // Folder Browser �г� ��
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 
    // ImGui ������
    //
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void TestMainApp::LoadAssets()
{
    std::filesystem::path fullPath =
        m_folderPath / std::filesystem::path(m_selectedFile);

    auto ext = fullPath.extension();
    if (ext.empty()) return; // Ȯ���ڰ� ������ ����

    std::filesystem::path keyPath = fullPath;
    keyPath.replace_extension(); // Ȯ���� �����ϰ� Ű�� ��������

    std::wstring keyWide = keyPath.wstring();

    if (keyWide == m_selectedAssetKey)
    {
        // �̹� �ε�� �����̸� �ٽ� �ε����� ����
        return;
    }
    m_selectedAssetKey = keyWide; // ���õ� ���� Ű ����
  
    if (ext == L".png")
    {
        m_AssetManager.LoadSpriteTexture(m_Renderer->GetD2DContext(), keyWide, fullPath);
        m_singleSpriteTexture = m_AssetManager.GetSpriteTexture(keyWide);
    }
    else if (ext == L".json")
    {
        m_AssetManager.LoadAseprite(m_Renderer->GetD2DContext(), keyWide, fullPath);
    }

    m_bChangedFile = true; // ������ ����Ǿ����� ǥ��
}


void TestMainApp::OnResize(int width, int height)
{
    __super::OnResize(width, height);

    if (m_Renderer != nullptr) m_Renderer->Resize(width, height);
}

void TestMainApp::OnClose()
{
    std::cout << "OnClose" << std::endl;
}

void TestMainApp::BrowseForFolder()
{
    HRESULT hr;
    IFileOpenDialog* pDialog = nullptr;

    // COM ��ȭ���� ����
    hr = CoCreateInstance(
        CLSID_FileOpenDialog, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDialog)
    );

    if (FAILED(hr) || !pDialog) return;

    // ���� ���� ���� ����
    DWORD opts = 0;
    if (SUCCEEDED(pDialog->GetOptions(&opts)))
        pDialog->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

    // ���̾�α� ǥ��
    hr = pDialog->Show(m_hWnd);

    if (SUCCEEDED(hr))
    {
        IShellItem* pItem = nullptr;
        hr = pDialog->GetResult(&pItem);

        if (SUCCEEDED(hr) && pItem)
        {
            PWSTR pszFolder = nullptr;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolder);
            if (SUCCEEDED(hr) && pszFolder)
            {
                m_folderPath = pszFolder;        // ���õ� ���� ��� ����
                CoTaskMemFree(pszFolder);
            }
            pItem->Release();
        }   
    }

    pDialog->Release();
}

void TestMainApp::UpdateFileList()
{
    m_fileList.clear();
    for (const auto& entry : std::filesystem::directory_iterator(m_folderPath))
    {
        if (entry.is_regular_file())
        {
            // ���ϸ� �߰�, ���丮�� ����
            if (entry.path().extension() == L".png" ||
                entry.path().extension() == L".json")
                // �̹��� ���ϸ� �߰�
                m_fileList.push_back(entry.path().filename().wstring());
        }
    }
}

void TestMainApp::UpdateAnimationPlaybackSpeed(float speed)
{
    for (auto& sprite : m_curSprites)
    {
        sprite.SetPlaybackSpeed(speed);
    }
}

