#include <assert.h>
#include "direct3d.h"
#include "texture.h"
#include "model.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "WICTextureLoader11.h"
#include "shader3d.h"
#include "shader3d_unlit.h"
#include"shader_depth.h"


// 3D vertex structure
struct Vertex3d
{
	XMFLOAT3 position; // position
	XMFLOAT3 normal;   // normal
	XMFLOAT4 color;    // color
	XMFLOAT2 texcoord; // UV
};

static int g_TextureWhite = -1;


MODEL* ModelLoad( const char *FileName, float scale, bool bBlender)
{
	MODEL* model = new MODEL;

	model->AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(model->AiScene);

	model->VertexBuffer = new ID3D11Buffer*[model->AiScene->mNumMeshes];
	model->IndexBuffer = new ID3D11Buffer*[model->AiScene->mNumMeshes];

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// Create vertex buffer
		{
			Vertex3d* vertex = new Vertex3d[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				if (bBlender) {
					vertex[v].position = XMFLOAT3(mesh->mVertices[v].x * scale, -mesh->mVertices[v].z * scale, mesh->mVertices[v].y * scale);
					vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, -mesh->mNormals[v].z, mesh->mNormals[v].y);
				}
				else {
					vertex[v].position = XMFLOAT3(mesh->mVertices[v].x * scale, mesh->mVertices[v].y * scale, mesh->mVertices[v].z * scale);
					vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				}

				vertex[v].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				if (mesh->mTextureCoords[0]) {
					vertex[v].texcoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				} else {
					vertex[v].texcoord = XMFLOAT2(0.0f, 0.0f);
				}

				// Get AABB
				if (v == 0 && m == 0) {
					model->local_aabb.min = vertex[v].position;
					model->local_aabb.max = vertex[v].position;
				}
				else {
					model->local_aabb.min.x = std::min(model->local_aabb.min.x, vertex[v].position.x);
					model->local_aabb.min.y = std::min(model->local_aabb.min.y, vertex[v].position.y);
					model->local_aabb.min.z = std::min(model->local_aabb.min.z, vertex[v].position.z);
					model->local_aabb.max.x = std::max(model->local_aabb.max.x, vertex[v].position.x);
					model->local_aabb.max.y = std::max(model->local_aabb.max.y, vertex[v].position.y);
					model->local_aabb.max.z = std::max(model->local_aabb.max.z, vertex[v].position.z);
				}
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(Vertex3d) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = vertex;

			Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer[m]);

			delete[] vertex;
		}


		// Create index buffer
		{
			unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				assert(face->mNumIndices == 3);

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index;

			Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer[m]);

			delete[] index;
		}

	}

	g_TextureWhite = Texture_Load(L"resource/texture/white.png");

	// If textures are embedded in FBX
	for (unsigned int i = 0; i < model->AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = model->AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture;
		ID3D11Resource* resource;

		CreateWICTextureFromMemory(
			Direct3D_GetDevice(),
			Direct3D_GetContext(),
			(const uint8_t*)aitexture->pcData,
			(size_t)aitexture->mWidth,
			&resource,
			&texture);

		assert(texture);

		resource->Release(); // !!!!!!!!!!

		model->Texture[aitexture->mFilename.data] = texture;
	}

	// Get the directory path of the FBX file
	const std::string modelPath(FileName);

	// Find the last '/' or '\\' position (Windows compatible)
	size_t pos = modelPath.find_last_of("/\\");
	std::string directory;

	if (pos != std::string::npos) {
		directory = modelPath.substr(0, pos);  // Extract directory from path
	}
	else {
		directory = "";  // No separator in path (filename only)
	}

	// If textures are provided as separate files (not embedded in FBX)
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiString filename;
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &filename);

		if (filename.length == 0) {
			continue;
		}

		if (model->Texture.count(filename.C_Str())) {
			continue;
		}

		// Strip unnecessary folder prefix from texture path
		std::string str_filename = filename.C_Str();
		size_t n = str_filename.find_last_of("\\/");
		str_filename = str_filename.substr(n + 1);

		if (model->Texture.count(str_filename)) {
			continue;
		}

		ID3D11ShaderResourceView* texture;
		ID3D11Resource* resource;

		std::string texfilename = directory + "/" + str_filename;

		int len = MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, nullptr, 0);
		wchar_t* pWideFilename = new wchar_t[len];
		MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, pWideFilename, len);

		CreateWICTextureFromFile(
			Direct3D_GetDevice(),
			Direct3D_GetContext(),
			pWideFilename,
			&resource,
			&texture);

		delete[] pWideFilename;

		// Texture file not found -> skip (fallback to white texture)
		if (!texture) {
			continue;
		}

		resource->Release(); // !!!!!!!!!!

		model->Texture[filename.C_Str()] = texture;
	}

	return model;
}


void ModelRelease(MODEL* model)
{
	if (!model) return;

	if (model->AiScene)


	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		model->VertexBuffer[m]->Release();
		model->IndexBuffer[m]->Release();
	}

	delete[] model->VertexBuffer;
	delete[] model->IndexBuffer;

	for (std::pair<const std::string, ID3D11ShaderResourceView*> pair : model->Texture)
	{
		pair.second->Release();
	}

	aiReleaseImport(model->AiScene);
	model->AiScene = nullptr;

	delete model;
	model = nullptr;
}

void ModelDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld, const DirectX::XMFLOAT4& tint)
{
	// Set shader draw pipeline
	Shader3d_Begin();

	// Set primitive topology
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set world transform matrix to vertex shader
	Shader3d_SetWorldMatrix(mtxWorld);


	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiString texture;
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

		if (texture.length != 0 && model->Texture.count(texture.data) > 0) {
			Direct3D_GetContext()->PSSetShaderResources(0, 1, &model->Texture[texture.data]);
			Shader3d_SetColor({ tint.x, tint.y, tint.z, tint.w });
		}
		else {
			// No texture or missing file -> white texture + material color
			Texture_SetTexture(g_TextureWhite);
			aiColor3D diffuse;
			aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			Shader3d_SetColor({ diffuse.r * tint.x, diffuse.g * tint.y, diffuse.b * tint.z, tint.w });
		}

		// Set vertex buffer to draw pipeline
		UINT stride = sizeof(Vertex3d);
		UINT offset = 0;
		Direct3D_GetContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// Set index buffer to draw pipeline
		Direct3D_GetContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// Execute polygon drawing
		Direct3D_GetContext()->DrawIndexed(model->AiScene->mMeshes[m]->mNumFaces * 3, 0, 0);
	}
}

void ModelDepthDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld)
{
	// Set depth shader draw pipeline
	ShaderDepth_Begin();

	// Set primitive topology
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set world transform matrix to vertex shader
	ShaderDepth_SetWorldMatrix(mtxWorld);

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		// Set vertex buffer to draw pipeline
		UINT stride = sizeof(Vertex3d);
		UINT offset = 0;
		Direct3D_GetContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// Set index buffer to draw pipeline
		Direct3D_GetContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// Execute polygon drawing
		Direct3D_GetContext()->DrawIndexed(model->AiScene->mMeshes[m]->mNumFaces * 3, 0, 0);
	}
}

void ModelUnlitDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld)
{
	// Set unlit shader draw pipeline
	Shader3dUnlit_Begin();

	// Set primitive topology
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set world transform matrix to vertex shader
	Shader3dUnlit_SetWorldMatrix(mtxWorld);

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiString texture;
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

		if (texture.length != 0) {
			Direct3D_GetContext()->PSSetShaderResources(0, 1, &model->Texture[texture.data]);
			Shader3dUnlit_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		else {
			Texture_SetTexture(g_TextureWhite);
			aiColor3D diffuse;
			aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			Shader3dUnlit_SetColor({ diffuse.r, diffuse.g, diffuse.b, 1.0f });
		}

		// Set vertex buffer to draw pipeline
		UINT stride = sizeof(Vertex3d);
		UINT offset = 0;
		Direct3D_GetContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// Set index buffer to draw pipeline
		Direct3D_GetContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// Execute polygon drawing
		Direct3D_GetContext()->DrawIndexed(model->AiScene->mMeshes[m]->mNumFaces * 3, 0, 0);
	}
}

AABB Model_GetAABB(MODEL* model, const DirectX::XMFLOAT3& position)
{
	return {
		{ position.x + model->local_aabb.min.x, position.y + model->local_aabb.min.y, position.z + model->local_aabb.min.z },
		{ position.x + model->local_aabb.max.x, position.y + model->local_aabb.max.y, position.z + model->local_aabb.max.z },
	};
}
