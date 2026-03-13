/*==============================================================================

   GDI日本語テキスト → テクスチャ生成 [text_texture.h]

==============================================================================*/
#ifndef TEXT_TEXTURE_H
#define TEXT_TEXTURE_H

#include <d3d11.h>

// テキストテクスチャの初期化・終了
void TextTexture_Initialize(void);
void TextTexture_Finalize(void);

// 日本語テキストからテクスチャを生成する
//
// text     : 表示したい文字列（Shift_JIS）
// fontSize : フォントサイズ（ピクセル）
// 戻り値   : テクスチャ管理番号（Sprite_Draw で使用可能）。失敗時 -1。
//
// ※同じ text + fontSize の組み合わせはキャッシュされる（二重生成しない）
//
int TextTexture_Create(const char* text, int fontSize);

// キャッシュを全てクリア（シーン切り替え時などに呼ぶ）
void TextTexture_ClearCache(void);

#endif // TEXT_TEXTURE_H
