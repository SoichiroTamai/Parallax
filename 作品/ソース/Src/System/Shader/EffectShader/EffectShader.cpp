#include "System/System.h"

#include "EffectShader.h"


void EffectShader::DrawLine(const Math::Vector3 & p1, const Math::Vector3 & p2, const Math::Color & color)
{
	// 定数バッファ書き込み
	m_cb0.Write();

	// 頂点レイアウトをセット
	D3D.GetDevContext()->IASetInputLayout(m_inputLayout);

	// 
	Vertex vertex[2] = {
		{p1, {0,0}, color},
		{p2, {1,0}, color},
	};

	// 頂点を描画
	D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 2, &vertex[0], sizeof(Vertex));
}

void EffectShader::DrawVertices(const std::vector<Vertex>& vertices, D3D_PRIMITIVE_TOPOLOGY topology)
{
	// 定数バッファ書き込み
	m_cb0.Write();

	// 頂点レイアウトをセット
	D3D.GetDevContext()->IASetInputLayout(m_inputLayout);

	// 頂点を描画
	D3D.DrawVertices(topology, vertices.size(), &vertices[0], sizeof(Vertex));
}

void EffectShader::DrawMesh(const Mesh* mesh, const std::vector<Material>& materials)
{
	if (mesh == nullptr)return;

	// 定数バッファ書き込み
	m_cb0.Write();

	// メッシュ情報をセット
	mesh->SetToDevice();

	// 頂点レイアウトをセット
	D3D.GetDevContext()->IASetInputLayout(m_inputLayout_Model);

	// 全サブセット
	for (UINT subi = 0; subi < mesh->GetSubsets().size(); subi++)
	{
		// 面が１枚も無い場合はスキップ
		if (mesh->GetSubsets()[subi].FaceCount == 0)continue;

		// マテリアルセット
		const Material& material = materials[ mesh->GetSubsets()[subi].MaterialNo ];

		//-----------------------
		// テクスチャセット
		//-----------------------

		ID3D11ShaderResourceView* srvs[1] = {};
		// BaseColor
		srvs[0] = material.BaseColorTex->GetSRView();
		// セット
		D3D.GetDevContext()->PSSetShaderResources(0, _countof(srvs), srvs);

		//-----------------------
		// サブセット描画
		//-----------------------
		mesh->DrawSubset(subi);
	}
}

bool EffectShader::Init()
{
	//-------------------------------------
	// 頂点シェーダ
	//-------------------------------------
	{
		// コンパイル済みのシェーダーヘッダーファイルをインクルード
		#include "EffectShader_VS.inc"

		// 頂点シェーダー作成
		if (FAILED(D3D.GetDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}

		{
			// １頂点の詳細な情報
			std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			// 頂点入力レイアウト作成
			if (FAILED(D3D.GetDev()->CreateInputLayout(
				&layout[0],			// 入力エレメント先頭アドレス
				layout.size(),		// 入力エレメント数
				&compiledBuffer[0],				// 頂点バッファのバイナリデータ
				sizeof(compiledBuffer),			// 上記のバッファサイズ
				&m_inputLayout))					// 
				) {
				assert(0 && "CreateInputLayout失敗");
				Release();
				return false;
			}
		}

		// DrawModel用
		{
			// １頂点の詳細な情報
			std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			// 頂点入力レイアウト作成
			if (FAILED(D3D.GetDev()->CreateInputLayout(
				&layout[0],			// 入力エレメント先頭アドレス
				layout.size(),		// 入力エレメント数
				&compiledBuffer[0],				// 頂点バッファのバイナリデータ
				sizeof(compiledBuffer),			// 上記のバッファサイズ
				&m_inputLayout_Model))					// 
				) {
				assert(0 && "CreateInputLayout失敗");
				Release();
				return false;
			}		
		}
	}


	//-------------------------------------
	// ピクセルシェーダ
	//-------------------------------------
	{
		// コンパイル済みのシェーダーヘッダーファイルをインクルード
		#include "EffectShader_PS.inc"

		if (FAILED(D3D.GetDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	//-------------------------------------
	// 定数バッファ作成
	//-------------------------------------
	m_cb0.Create();

	return true;
}

void EffectShader::Release()
{
	SafeRelease(m_VS);
	SafeRelease(m_PS);
	SafeRelease(m_inputLayout);
	SafeRelease(m_inputLayout_Model);
	m_cb0.Release();
}

void EffectShader::SetToDevice()
{
	// 頂点シェーダをセット
	D3D.GetDevContext()->VSSetShader(m_VS, 0, 0);
	// 頂点レイアウトをセット
	D3D.GetDevContext()->IASetInputLayout(m_inputLayout);

	// ピクセルシェーダをセット
	D3D.GetDevContext()->PSSetShader(m_PS, 0, 0);

	// 定数バッファをセット
	D3D.GetDevContext()->VSSetConstantBuffers(0, 1, m_cb0.GetAddress());
	D3D.GetDevContext()->PSSetConstantBuffers(0, 1, m_cb0.GetAddress());
}

// テクスチャセット

void EffectShader::SetTexture(ID3D11ShaderResourceView * srv)
{
	if (srv)
	{
		D3D.GetDevContext()->PSSetShaderResources(0, 1, &srv);
	}
	else
	{
		// テクスチャが無い場合は、白テクスチャをセットする
		D3D.GetDevContext()->PSSetShaderResources(0, 1, D3D.GetWhiteTex()->GetSRViewAddress());
	}
}
