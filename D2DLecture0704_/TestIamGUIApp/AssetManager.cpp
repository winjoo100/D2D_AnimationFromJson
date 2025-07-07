#include "pch.h"
#include "AnimationClip.h"
#include "AsepriteParser.h"
#include "D2DRender.h"
#include "AssetManager.h"
#include <wincodec.h> // WIC (�̹��� �ε��)

void AssetManager::LoadTexture(
    ID2D1DeviceContext7* d2dContext,
    const std::wstring& key,
    const std::filesystem::path& filepath)
{
    // �̹� ������ Ű�� �ε�� �ؽ�ó�� ������ �ߺ� �ε����� ����
    if (m_textures.find(key) != m_textures.end())
        return;

    // WIC ���丮 ����
    // WIC ���丮�� WIC (Windows Imaging Component) �� �߽���� ���� �����,
    // �̹��� ������ ���ڵ��ϰ� ��ȯ�ϴµ� �ʿ��� ��� ��ü���� �����Ѵ�.
    Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory2,       // �ֽ� ���� WIC ���丮
        nullptr,
        CLSCTX_INPROC_SERVER,           // ���� ���μ������� �ν��Ͻ� ����
        IID_PPV_ARGS(&wicFactory));     // ��� ������
    if (FAILED(hr)) return;             // �����ϸ� �Լ� ����

    // WIC ���ڴ� ���� - ���Ͽ��� �̹��� ���ڵ�
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(
        filepath.c_str(),               // ���� ���
        nullptr,                        // GUID - ���ڴ� �ڵ� ����
        GENERIC_READ,                   // �б� ���
        WICDecodeMetadataCacheOnDemand, // ��Ÿ�����ʹ� �ʿ��� �� ����
        &decoder);
    if (FAILED(hr)) return;             // �����ϸ� �Լ� ����

    // �̹��� ù ��° ������ ��������
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return;

    // �ȼ� ���� ��ȯ�� ����
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return;

    // ���� ��ȯ �ʱ�ȭ
    hr = converter->Initialize(
        frame.Get(),                    // ���� ������
        GUID_WICPixelFormat32bppPBGRA,  // Direct2D ȣȯ�Ǵ� 32bpp PBGRA �������� ��ȯ
        WICBitmapDitherTypeNone,        // ����� ����
        nullptr,                        // �ȷ�Ʈ ��� ����
        0.0f,                           // ���� �Ӱ谪 (�̻��)
        WICBitmapPaletteTypeCustom);    // �ȷ�Ʈ Ÿ�� (�̻��)
    if (FAILED(hr)) return;

    // WIC ��Ʈ���� Direct2D ��Ʈ������ ��ȯ
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
    hr = d2dContext->CreateBitmapFromWicBitmap(
        converter.Get(),
        nullptr,
        &bitmap);

    // ��ȯ ���� �� �ؽ�ó �ʿ� ����
    if (SUCCEEDED(hr))
    {
        m_textures[key] = bitmap;
    }
}


void AssetManager::LoadAseprite(ID2D1DeviceContext7* d2dContext, const std::wstring& key, const std::filesystem::path& jsonPath)
{
    // �̹� ���� Ű�� �ε�� Ŭ���� ������ �ߺ� �ε����� ����
    if (m_clipsMap.find(key) != m_clipsMap.end())
        return;

    // Aseprite JSON ������ �Ľ��ؼ� Ŭ�� �� ����
    auto parsedMap = AsepriteParser::Load(jsonPath); // std::unordered_map<std::string, AnimationClip>�� ����

    // �Ľ� ����� ��� ������ �� Ŭ���� ����ϰ� ����
    if (parsedMap.empty())
    {
        m_clipsMap[key] = {}; // �Ľ� ���� �Ǵ� �� ���
        return;
    }

    // �����Ǵ� PNG �ؽ�ó �ε� �õ�
    std::filesystem::path pngPath = jsonPath;
    pngPath.replace_extension(L".png");

    LoadTexture(d2dContext, key, pngPath);

    auto itTex = m_textures.find(key);
    if (itTex == m_textures.end() || !itTex->second)
    {
        m_clipsMap[key] = {}; // �ؽ�ó �ε� ����
        return;
    }

    // �ؽ�ó�� �� Ŭ���� �����ϰ� ���ͷ� ��ȯ
    AnimationClips clips;
    for (auto& [name, clip] : parsedMap)
    {
        clip.SetBitmap(itTex->second);
        clips.emplace_back(name, std::move(clip));
    }

    m_clipsMap[key] = std::move(clips); // ���� ����
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
    // ���� LoadTexture�� ���� �������� �ؽ�ó �ε� �� m_spriteTextures�� ����
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
        GUID_WICPixelFormat32bppPBGRA, // Direct2D ȣȯ ����
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
