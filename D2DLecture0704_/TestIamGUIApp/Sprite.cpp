#include "Sprite.h"

void DrawSprite(ID2D1DeviceContext7* d2dContext, const Sprite& sprite, float x, float y, float width, float height)
{
    ID2D1Bitmap1* bitmap = sprite.GetBitmap();
    if (!bitmap) return;

    D2D1_SIZE_F size = bitmap->GetSize();

    // 이미지 전체를 srcRect로 지정
    D2D1_RECT_F srcRect = D2D1::RectF(0.f, 0.f, size.width, size.height);

    // 화면에 그릴 위치와 크기
    D2D1_RECT_F destRect = D2D1::RectF(x, y, x + width, y + height);

    d2dContext->DrawBitmap(
        bitmap,
        destRect,
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        srcRect);
}
