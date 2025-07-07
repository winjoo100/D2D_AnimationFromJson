#pragma once


// ���� �Դϴ�. �� �̷��� ������ �ʿ�� �����ϴ�.
struct Frame
{
    D2D1_RECT_U srcRect;   // ��Ʈ���� �߶� ����

    float        duration; // �� �������� �����Ǵ� �ð�(��)

    float Width() const { return srcRect.right - srcRect.left; }
    float Height() const { return srcRect.bottom - srcRect.top; }

    D2D1_RECT_F ToRectF() const
    {
        return D2D1::RectF(
            static_cast<float>(srcRect.left),
            static_cast<float>(srcRect.top),
            static_cast<float>(srcRect.right),
            static_cast<float>(srcRect.bottom));
    }
};

// AnimationClip: ������ ������ + �ؽ�ó(immutable)
class AnimationClip
{
public:
    AnimationClip() = default;
    ~AnimationClip() = default;

    // �ؽ�ó ��Ʈ ���� (AssetManager���� �� ���� ȣ��)
    void SetBitmap(Microsoft::WRL::ComPtr<ID2D1Bitmap1> sheet)
    {
        m_sheet = std::move(sheet);
    }

    // AsepriteParser ��� �������� �� ���� ä�� ����
    void AddFrame(const Frame& frame)
    {
        m_frames.push_back(frame);
        m_totalDuration += frame.duration;
    }

    // ������ ������ ��ȸ
    const std::vector<Frame>& GetFrames() const { return m_frames; }
    float                       GetTotalDuration() const { return m_totalDuration; }
    ID2D1Bitmap1* GetBitmap() const { return m_sheet.Get(); }

private:
    std::vector<Frame>                  m_frames;
    float                                m_totalDuration = 0.f;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_sheet;
};

class SpriteAnimator
{
public:
    SpriteAnimator() = default;

    ~SpriteAnimator() = default;

    bool IsValid() const { return m_clip != nullptr; }

    // ����� Ŭ�� ���� (nullptr ��� �� �÷��� ����)
    void SetClip(const AnimationClip* clip)
    {
        m_clip = clip;
        m_elapsed = 0.f;
    }

    // ���� ��� ���� Ŭ�� ��ȸ
    const AnimationClip* GetClip() const { return m_clip; }

    // ���� ���(true) / ��ȸ ���(false)
    void SetLooping(bool loop) { m_loop = loop; }

    // deltaTime�� ���ؼ� elapsed ����
    void Update(float deltaTime)
    {
        if (!m_clip) return;
        m_elapsed += deltaTime * m_playbackSpeed;

        float total = m_clip->GetTotalDuration();
        if (m_loop)
        {
            // ���� ���: elapsed�� total ���� ������
            if (m_elapsed >= total)
                m_elapsed = std::fmod(m_elapsed, total);
        }
        else
        {
            // ����� ���: ���� �����ϸ� ����
            if (m_elapsed > total)
                m_elapsed = total;
        }
    }

    // ���� ��� ���� ������ �ε��� ��ȸ
    const Frame& GetCurrentFrame() const
    {
        static Frame dummy{ {0,0,0,0}, 0.f };

        if (!m_clip || m_clip->GetFrames().empty())
            return dummy;

        

        float accum = 0.f;
        for (auto& frame : m_clip->GetFrames())
        {
            accum += frame.duration;

            if (m_elapsed < accum)
                return frame;
        }
        // elapsed�� totalDuration �̻��� �� ������ ������ ��ȯ
        return m_clip->GetFrames().back();
    }

    // ��� ��ġ(��) ��ȸ/����
    float GetElapsed() const { return m_elapsed; }
    void  SetElapsed(float t) { m_elapsed = t; }

    // 2025.07.04 ����Լ�(�峭��) �߰� _ �����
    // ����ӵ� ���� ��ȸ/����
    float GetPlaybackSpeed() const { return m_playbackSpeed; }
    void  SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }

private:
    const AnimationClip* m_clip = nullptr;  // immutable clip ������
    float                m_elapsed = 0.f;      // ��� ��ġ
    bool                 m_loop = true;     // ���� ��� ����


    // 2025.07.04 �������(�峭��) �߰� _ �����
    float m_playbackSpeed = 1.0f;  // ��� �ӵ� ���� (�⺻ 1.0f)
};
