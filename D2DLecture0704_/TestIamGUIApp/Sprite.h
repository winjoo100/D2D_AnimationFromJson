#pragma once
#include "pch.h"

class Sprite
{
public:
    Sprite() = default;
    ~Sprite() = default;

    void SetBitmap(Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap)
    {
        m_bitmap = std::move(bitmap);
    }

    ID2D1Bitmap1* GetBitmap() const { return m_bitmap.Get(); }

    D2D1_SIZE_F GetSize() const
    {
        if (m_bitmap)
            return m_bitmap->GetSize();
        else
            return { 0.f, 0.f };
    }

private:
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_bitmap;
};
