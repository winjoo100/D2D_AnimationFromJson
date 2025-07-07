#include "pch.h"
#include "AnimationClip.h"
#include "AsepriteParser.h"
#include "D2DRender.h"
#include "AssetManager.h"
#include <wincodec.h> // WIC (이미지 로드용)

void AssetManager::LoadTexture(
    ID2D1DeviceContext7* d2dContext,
    const std::wstring& key,
    const std::filesystem::path& filepath)
{
    // 이미 동일한 키로 로드된 텍스처가 있으면 중복 로드하지 않음
    if (m_textures.find(key) != m_textures.end())
        return;

    // WIC 팩토리 생성
    // WIC 팩토리는 WIC (Windows Imaging Component) 의 중심허브 같은 존재로,
    // 이미지 파일을 디코딩하고 변환하는데 필요한 모든 객체들을 생성한다.
    Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory2,       // 최신 버전 WIC 팩토리
        nullptr,
        CLSCTX_INPROC_SERVER,           // 로컬 프로세스에서 인스턴스 생성
        IID_PPV_ARGS(&wicFactory));     // 결과 포인터
    if (FAILED(hr)) return;             // 실패하면 함수 종료

    // WIC 디코더 생성 - 파일에서 이미지 디코딩
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(
        filepath.c_str(),               // 파일 경로
        nullptr,                        // GUID - 디코더 자동 선택
        GENERIC_READ,                   // 읽기 모드
        WICDecodeMetadataCacheOnDemand, // 메타데이터는 필요할 때 읽음
        &decoder);
    if (FAILED(hr)) return;             // 실패하면 함수 종료

    // 이미지 첫 번째 프레임 가져오기
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return;

    // 픽셀 포맷 변환기 생성
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return;

    // 포맷 변환 초기화
    hr = converter->Initialize(
        frame.Get(),                    // 원본 프레임
        GUID_WICPixelFormat32bppPBGRA,  // Direct2D 호환되는 32bpp PBGRA 포맷으로 변환
        WICBitmapDitherTypeNone,        // 디더링 없음
        nullptr,                        // 팔레트 사용 안함
        0.0f,                           // 알파 임계값 (미사용)
        WICBitmapPaletteTypeCustom);    // 팔레트 타입 (미사용)
    if (FAILED(hr)) return;

    // WIC 비트맵을 Direct2D 비트맵으로 변환
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
    hr = d2dContext->CreateBitmapFromWicBitmap(
        converter.Get(),
        nullptr,
        &bitmap);

    // 변환 성공 시 텍스처 맵에 저장
    if (SUCCEEDED(hr))
    {
        m_textures[key] = bitmap;
    }
}


void AssetManager::LoadAseprite(ID2D1DeviceContext7* d2dContext, const std::wstring& key, const std::filesystem::path& jsonPath)
{
    // 이미 같은 키로 로드된 클립이 있으면 중복 로드하지 않음
    if (m_clipsMap.find(key) != m_clipsMap.end())
        return;

    // Aseprite JSON 파일을 파싱해서 클립 맵 생성
    auto parsedMap = AsepriteParser::Load(jsonPath); // std::unordered_map<std::string, AnimationClip>로 가정

    // 파싱 결과가 비어 있으면 빈 클립을 등록하고 종료
    if (parsedMap.empty())
    {
        m_clipsMap[key] = {}; // 파싱 실패 또는 빈 결과
        return;
    }

    // 대응되는 PNG 텍스처 로드 시도
    std::filesystem::path pngPath = jsonPath;
    pngPath.replace_extension(L".png");

    LoadTexture(d2dContext, key, pngPath);

    auto itTex = m_textures.find(key);
    if (itTex == m_textures.end() || !itTex->second)
    {
        m_clipsMap[key] = {}; // 텍스처 로드 실패
        return;
    }

    // 텍스처를 각 클립에 연결하고 벡터로 변환
    AnimationClips clips;
    for (auto& [name, clip] : parsedMap)
    {
        clip.SetBitmap(itTex->second);
        clips.emplace_back(name, std::move(clip));
    }

    m_clipsMap[key] = std::move(clips); // 최종 저장
}

const AssetManager::AnimationClips& AssetManager::GetClips(const std::wstring& key) const
{
    static AnimationClips empty;
    auto it = m_clipsMap.find(key);
    if (it != m_clipsMap.end())
        return it->second;
    return empty;
}

void AssetManager::LoadSpriteTexture(ID2D1DeviceContext7* d2dContext, const std::wstring& key, const std::filesystem::path& filepath)
{
    // 기존 LoadTexture와 같은 내용으로 텍스처 로드 후 m_spriteTextures에 저장
    if (m_spriteTextures.find(key) != m_spriteTextures.end())
        return;

    Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory2, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(
        filepath.c_str(), nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return;

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppPBGRA, // Direct2D 호환 포맷
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
    hr = d2dContext->CreateBitmapFromWicBitmap(
        converter.Get(),
        nullptr,
        &bitmap);
    if (SUCCEEDED(hr))
    {
        m_spriteTextures[key] = bitmap;
    }
}

Microsoft::WRL::ComPtr<ID2D1Bitmap1> AssetManager::GetSpriteTexture(const std::wstring& key) const
{
    auto it = m_spriteTextures.find(key);
    if (it != m_spriteTextures.end())
        return it->second;
    return nullptr;
}
