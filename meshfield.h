/*==============================================================================

   メッシュフィールドの表示 [meshfield.h]
														 Author : Youhei Sato
														 Date   : 2025/09/19
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MESHFIELD_H
#define MESHFIELD_H

#include <d3d11.h>
#include <DirectXMath.h>


void MeshField_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void MeshField_Finalize();
void MeshField_Draw();

#endif // MESHFIELD_H
