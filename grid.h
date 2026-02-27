/*==============================================================================

   XZ平面グリッドの表示 [grid.h]
														 Author : Youhei Sato
														 Date   : 2025/09/11
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef GRID_H
#define GRID_H

#include <d3d11.h>

void Grid_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Grid_Finalize();
void Grid_Draw();

#endif // GRID_H
