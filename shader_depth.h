/*==============================================================================

   ê[ìxèÓïÒèëÇ´çûÇ›óp [shader_depth.h]
														 Author : Youhei Sato
														 Date   : 2025/11/21
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_DEPTHT_H
#define	SHADER_DEPTHT_H

#include <d3d11.h>
#include <DirectXMath.h>

bool ShaderDepth_Initialize();
void ShaderDepth_Finalize();

void ShaderDepth_SetWorldMatrix(const DirectX::XMMATRIX& matrix);

void ShaderDepth_Begin();

#endif // SHADER_DEPTH_H

