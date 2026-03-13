/*==============================================================================

   GDI日本語テキスト → テクスチャ生成 [text_texture.cpp]

==============================================================================*/
#include "text_texture.h"
#include "texture.h"
#include "direct3d.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>

// キャッシュ: "text|fontSize" → テクスチャID
static std::unordered_map<std::string, int> g_TextTextureCache;

void TextTexture_Initialize(void)
{
	g_TextTextureCache.clear();
}

void TextTexture_Finalize(void)
{
	TextTexture_ClearCache();
}

void TextTexture_ClearCache(void)
{
	for (auto& pair : g_TextTextureCache) {
		Texture_Release(pair.second);
	}
	g_TextTextureCache.clear();
}

int TextTexture_Create(const char* text, int fontSize)
{
	if (!text || text[0] == '\0') return -1;

	// キャッシュキーを作る
	char keyBuf[512];
	snprintf(keyBuf, sizeof(keyBuf), "%s|%d", text, fontSize);
	std::string key = keyBuf;

	// キャッシュにあればそれを返す
	auto it = g_TextTextureCache.find(key);
	if (it != g_TextTextureCache.end()) {
		return it->second;
	}

	// === 1. フォントを作る ===
	HFONT hFont = CreateFontA(
		fontSize,       // 文字の高さ
		0,              // 幅（自動）
		0, 0,           // 角度
		FW_BOLD,        // 太さ
		FALSE,          // 斜体
		FALSE,          // 下線
		FALSE,          // 取り消し線
		SHIFTJIS_CHARSET,  // 日本語文字セット
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		"MS Gothic"     // フォント名
	);
	if (!hFont) return -1;

	// === 2. テキストサイズを計測する ===
	HDC hdc = CreateCompatibleDC(NULL);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	// テキストのピクセルサイズを取得
	SIZE textSize = {};
	GetTextExtentPoint32A(hdc, text, (int)strlen(text), &textSize);

	int texWidth = textSize.cx + 4;   // 少し余裕を持たせる
	int texHeight = textSize.cy + 4;

	// テクスチャサイズは4の倍数に揃える
	texWidth = (texWidth + 3) & ~3;
	texHeight = (texHeight + 3) & ~3;

	// === 3. DIBSection（ピクセル直接アクセス可能なビットマップ）を作る ===
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = texWidth;
	bmi.bmiHeader.biHeight = -texHeight;  // 負 = 上から下
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	BYTE* pBitmapPixels = nullptr;
	HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pBitmapPixels, NULL, 0);
	if (!hBitmap) {
		SelectObject(hdc, hOldFont);
		DeleteDC(hdc);
		DeleteObject(hFont);
		return -1;
	}

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

	// === 4. 背景を黒にクリア ===
	RECT rc = { 0, 0, texWidth, texHeight };
	HBRUSH hBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
	FillRect(hdc, &rc, hBlack);

	// === 5. 文字を白で描画 ===
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	TextOutA(hdc, 2, 2, text, (int)strlen(text));

	GdiFlush();

	// === 6. GDI の BGRA → DirectX の RGBA に変換 ===
	// GDI は B,G,R,A の順。DirectX は R,G,B,A の順。
	// 白文字なので、輝度をアルファ値として使う（背景透過）
	std::vector<BYTE> rgbaPixels(texWidth * texHeight * 4);
	for (int y = 0; y < texHeight; y++) {
		for (int x = 0; x < texWidth; x++) {
			int srcIdx = (y * texWidth + x) * 4;
			int dstIdx = (y * texWidth + x) * 4;
			BYTE b = pBitmapPixels[srcIdx + 0];
			BYTE g = pBitmapPixels[srcIdx + 1];
			BYTE r = pBitmapPixels[srcIdx + 2];
			// 輝度をアルファ値にする（文字部分だけ不透明に）
			BYTE alpha = r;  // 白文字なので R=G=B
			rgbaPixels[dstIdx + 0] = 255;   // R（白）
			rgbaPixels[dstIdx + 1] = 255;   // G（白）
			rgbaPixels[dstIdx + 2] = 255;   // B（白）
			rgbaPixels[dstIdx + 3] = alpha;  // A（文字の濃さ）
		}
	}

	// === 7. GDI リソース解放 ===
	SelectObject(hdc, hOldBitmap);
	SelectObject(hdc, hOldFont);
	DeleteObject(hBitmap);
	DeleteDC(hdc);
	DeleteObject(hFont);

	// === 8. DirectX テクスチャとして登録 ===
	// タグ用のワイド文字列を作る
	int wlen = MultiByteToWideChar(932, 0, keyBuf, -1, NULL, 0);
	std::vector<wchar_t> wTag(wlen);
	MultiByteToWideChar(932, 0, keyBuf, -1, wTag.data(), wlen);

	int texId = Texture_CreateFromMemory(rgbaPixels.data(), texWidth, texHeight, wTag.data());

	// キャッシュに登録
	if (texId >= 0) {
		g_TextTextureCache[key] = texId;
	}

	return texId;
}
