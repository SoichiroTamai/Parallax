#include "System/System.h"

#include "GenerateShadowMapShader.h"


void GenerateShadowMapShader::Begin()
{
	// 現在のRTとZとViewportを記憶
	if (m_saveRT)return;
	if (m_saveZ)return;

	// 現在のRTとZとViewportを記憶
	D3D.GetDevContext()->OMGetRenderTargets(1, &m_saveRT, &m_saveZ);
	D3D.GetDevContext()->RSGetViewports(&m_numVP, &m_saveVP);


	// RT変更
	D3D.GetDevContext()->OMSetRenderTargets(1, m_dirLightShadowMap->GetRTViewAddress(), m_dirLightZBuffer->GetDSView());
	// Viewport変更
	D3D11_VIEWPORT vp = { 0, 0, m_dirLightShadowMap->GetWidth(), m_dirLightShadowMap->GetHeight(), 0, 1 };
	D3D.GetDevContext()->RSSetViewports(1, &vp);

	// クリア
	D3D.GetDevContext()->ClearRenderTargetView(m_dirLightShadowMap->GetRTView(), Math::Color(1, 1, 1, 1));
	D3D.GetDevContext()->ClearDepthStencilView(m_dirLightZBuffer->GetDSView(), D3D11_CLEAR_DEPTH, 1, 0);

	// ライトのビュー行列を適当に作成
	Matrix mV;
	mV.LookTo({ 0.5f, -1, 0.5f }, { 0, 1, 0 });
	Matrix mTrans;
	mTrans.CreateTranslation(0, 0, -20);
	mV = mTrans * mV;
	mV.Inverse();

	// ライトの射影行列を適当に作成　50mx50mの正射影行列
	Matrix mP;
	mP.CreateProjection_Orthographic(50, 50, 0, 100);

	// ビュー*射影をセット
	m_cb0.Work().mLightVP = mV * mP;

	// ModelShaderで使用するためライトの定数バッファに行列を入れておく
	SHADER.m_cb8_Light.Work().DL_mLightVP = mV * mP;

	// シェーダーや定数バッファをセット
	SetToDevice();
}

void GenerateShadowMapShader::End()
{
	// 記憶してたRTとZとViewportを復元
	if (m_saveRT == nullptr)return;
	if (m_saveZ == nullptr)return;

	D3D.GetDevContext()->OMSetRenderTargets(1, &m_saveRT, m_saveZ);
	D3D.GetDevContext()->RSSetViewports(1, &m_saveVP);
	SafeRelease(m_saveRT);
	SafeRelease(m_saveZ);

}

void GenerateShadowMapShader::SetToDevice()
{
	// 頂点シェーダをセット
	D3D.GetDevContext()->VSSetShader(m_VS, 0, 0);
	// 頂点レイアウトをセット
	D3D.GetDevContext()->IASetInputLayout(m_inputLayout);

	// ピクセルシェーダをセット
	D3D.GetDevContext()->PSSetShader(m_PS, 0, 0);

	//---------------------
	// 定数バッファをセット
	//---------------------
	// オブジェクト定数バッファ
	D3D.GetDevContext()->VSSetConstantBuffers(0, 1, m_cb0.GetAddress());
	D3D.GetDevContext()->PSSetConstantBuffers(0, 1, m_cb0.GetAddress());
}

void GenerateShadowMapShader::DrawMeshDepth(const Mesh* mesh, const std::vector<Material>& materials)
{
	if (mesh == nullptr)return;

	// 定数バッファ書き込み
	m_cb0.Write();

	// メッシュ情報をセット
	mesh->SetToDevice();

	// 全サブセット
	for (UINT subi = 0; subi < mesh->GetSubsets().size(); subi++)
	{
		// 面が１枚も無い場合はスキップ
		if (mesh->GetSubsets()[subi].FaceCount == 0)continue;

		// マテリアル
		const Material& material = materials[mesh->GetSubsets()[subi].MaterialNo];

		//-----------------------
		// テクスチャセット
		//-----------------------
		// BaseColor
		D3D.GetDevContext()->PSSetShaderResources(0, 1, material.BaseColorTex->GetSRViewAddress());

		//-----------------------
		// サブセット描画
		//-----------------------
		mesh->DrawSubset(subi);
	}
}

bool GenerateShadowMapShader::Init()
{
	//-------------------------------------
	// 頂点シェーダ
	//-------------------------------------
	{
		// コンパイル済みのシェーダーヘッダーファイルをインクルード
		#include "GenerateShadowMapShader_VS.inc"

		// 頂点シェーダー作成
		if (FAILED(D3D.GetDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}

		// １頂点の詳細な情報
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	//-------------------------------------
	// ピクセルシェーダ
	//-------------------------------------
	{
		// コンパイル済みのシェーダーヘッダーファイルをインクルード
		#include "GenerateShadowMapShader_PS.inc"

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

	//-------------------------------------
	// 深度マップやZバッファを作成
	//-------------------------------------
	m_dirLightShadowMap = std::make_shared<Texture>();
	m_dirLightShadowMap->CreateRenderTarget(1024, 1024, DXGI_FORMAT_R32_FLOAT);
	m_dirLightZBuffer = std::make_shared<Texture>();
	m_dirLightZBuffer->CreateDepthStencil(1024, 1024, DXGI_FORMAT_R32_TYPELESS);

	return true;
}

void GenerateShadowMapShader::Release()
{
	SafeRelease(m_VS);
	SafeRelease(m_inputLayout);
	SafeRelease(m_PS);
	m_cb0.Release();
}

