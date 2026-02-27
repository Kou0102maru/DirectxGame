/*==============================================================================

   ビルボード用シェーダー [shader_billboard.h]
														 Author : Youhei Sato
														 Date   : 2025/11/14
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_BILLBOARD_H
#define	SHADER_BILLBOARD_H

#include <d3d11.h>
#include <DirectXMath.h>

bool ShaderBillBoard_Initialize();
void ShaderBillBoard_Finalize();

void ShaderBillBoard_SetWorldMatrix(const DirectX::XMMATRIX& matrix);

void ShaderBillBoard_SetColor(const DirectX::XMFLOAT4& color);

struct UVParameter
{
	DirectX::XMFLOAT2 scale;
	DirectX::XMFLOAT2 translation;
};

void ShaderBillBoard_SetUVParameter(const UVParameter& parameter);

void ShaderBillBoard_Begin();

#endif // SHADER_BILLBOARD_H
