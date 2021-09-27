#include "System/System.h"

#include "SpriteShader.h"

bool SpriteShader::Init()
{
	Release();

	//-------------------------------------
	// 頂点シェーダ
	//-------------------------------------
	{
		// コンパイル済みのシェーダーヘッダーファイルをインクルード
		#include "SpriteShader_VS.inc"

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

		// 頂点インプットレイアウト作成
		if (FAILED(D3D.GetDev()->CreateInputLayout(
			&layout[0],
			layout.size(),
			compiledBuffer,
			sizeof(compiledBuffer),
			&m_VLayout))
		){
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
		#include "SpriteShader_PS.inc"

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
	m_cb1.Create();

	//-------------------------------------
	// ステート作成
	//-------------------------------------
	m_smp0_Point = D3D.CreateSamplerState(SamplerFilterMode::Point, 0, SamplerAddressingMode::Wrap, false);
	m_smp0_Linear = D3D.CreateSamplerState(SamplerFilterMode::Linear, 0, SamplerAddressingMode::Wrap, false);

	m_ds = D3D.CreateDepthStencilState(false, false);

	m_rs = D3D.CreateRasterizerState(D3D11_CULL_NONE, D3D11_FILL_SOLID, true, false);

	return true;
}

void SpriteShader::Release()
{
	SafeRelease(m_VS);
	SafeRelease(m_PS);
	SafeRelease(m_VLayout);
	m_cb0.Release();

	SafeRelease(m_ds);
	SafeRelease(m_rs);
	SafeRelease(m_smp0_Point);
	SafeRelease(m_smp0_Linear);

	SafeRelease(m_saveState.DS);
	SafeRelease(m_saveState.RS);
	SafeRelease(m_saveState.Smp0);

}

void SpriteShader::Begin(bool linear, bool disableZBuffer)
{
	// 既にBeginしている
	if (m_isBegin)return;
	m_isBegin = true;

	//---------------------------------------
	// 2D用正射影行列作成
	//---------------------------------------
	UINT pNumVierports = 1;
	D3D11_VIEWPORT vp;
	D3D.GetDevContext()->RSGetViewports(&pNumVierports, &vp);
	m_mProj2D = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);

	// 定数バッファ書き込み
	m_cb1.Work().mProj = m_mProj2D;
	m_cb1.Write();

	//---------------------------------------
	// 現在のステート記憶
	//---------------------------------------
	// DepthStencilState記憶
	D3D.GetDevContext()->OMGetDepthStencilState(&m_saveState.DS, &m_saveState.StencilRef);
	// RasterizerState記憶
	D3D.GetDevContext()->RSGetState(&m_saveState.RS);
	// SamplerState記億
	D3D.GetDevContext()->PSGetSamplers(0, 1, &m_saveState.Smp0);

	//---------------------------------------
	// 使用するステートをセット
	//---------------------------------------
	// Z判定、Z書き込み無効のステートをセット
	if (disableZBuffer) {
		D3D.GetDevContext()->OMSetDepthStencilState(m_ds, 0);
	}
	// Samplerステートをセット
	if (linear) {
		D3D.GetDevContext()->PSSetSamplers(0, 1, &m_smp0_Linear);
	}
	else {
		D3D.GetDevContext()->PSSetSamplers(0, 1, &m_smp0_Point);
	}
	// Rasterizerステートをセット
	D3D.GetDevContext()->RSSetState(m_rs);

	//---------------------------------------
	// シェーダ
	//---------------------------------------

	// シェーダをセット
	D3D.GetDevContext()->VSSetShader(m_VS, 0, 0);
	D3D.GetDevContext()->PSSetShader(m_PS, 0, 0);

	// 頂点レイアウトセット
	D3D.GetDevContext()->IASetInputLayout(m_VLayout);

	// 定数バッファセット
	D3D.GetDevContext()->VSSetConstantBuffers(0, 1, m_cb0.GetAddress());
	D3D.GetDevContext()->PSSetConstantBuffers(0, 1, m_cb0.GetAddress());

	D3D.GetDevContext()->VSSetConstantBuffers(1, 1, m_cb1.GetAddress());
	D3D.GetDevContext()->PSSetConstantBuffers(1, 1, m_cb1.GetAddress());
}

void SpriteShader::End()
{
	if (!m_isBegin)return;
	m_isBegin = false;

	//---------------------------------------
	// 記憶してたステートに戻す
	//---------------------------------------
	D3D.GetDevContext()->OMSetDepthStencilState(m_saveState.DS, m_saveState.StencilRef);
	D3D.GetDevContext()->RSSetState(m_saveState.RS);
	D3D.GetDevContext()->PSSetSamplers(0, 1, &m_saveState.Smp0);
	SafeRelease(m_saveState.DS);
	SafeRelease(m_saveState.RS);
	SafeRelease(m_saveState.Smp0);
}

void SpriteShader::DrawTex(const Texture* tex, int x, int y, int w, int h, const Math::Rectangle* srcRect, const Math::Color* color, const Math::Vector2& pivot)
{
	if (tex == nullptr)return;

	// もし開始していない場合は開始する(最後にEnd())
	bool bBgn = m_isBegin;
	if (!bBgn)Begin();

	// テクスチャ(ShaderResourceView)セット
	D3D.GetDevContext()->PSSetShaderResources(0, 1, tex->GetSRViewAddress());

	// 色
	if (color) {
		m_cb0.Work().Color = *color;
	}
	m_cb0.Write();

	// UV
	Math::Vector2 uvMin = { 0, 0 };
	Math::Vector2 uvMax = { 1, 1 };
	if (srcRect)
	{
		uvMin.x = srcRect->x / (float)tex->GetInfo().Width;
		uvMin.y = srcRect->y / (float)tex->GetInfo().Height;

		uvMax.x = (srcRect->x + srcRect->width) / (float)tex->GetInfo().Width;
		uvMax.y = (srcRect->y + srcRect->height) / (float)tex->GetInfo().Height;
	}

	// 頂点作成
	float x1 = (float)x;
	float y1 = (float)y;
	float x2 = (float)(x + w);
	float y2 = (float)(y + h);

	// 基準点(Pivot)ぶんずらす
	x1 -= pivot.x * w;
	x2 -= pivot.x * w;
	y1 -= pivot.y * h;
	y2 -= pivot.y * h;

	Vertex vertex[] = {
		{ {x1, y1, 0},	{uvMin.x, uvMax.y} },
		{ {x1, y2, 0},	{uvMin.x, uvMin.y} },
		{ {x2, y1, 0},	{uvMax.x, uvMax.y} },
		{ {x2, y2, 0},	{uvMax.x, uvMin.y} }

	};

	// 描画
	D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, vertex, sizeof(Vertex));

	// セットしたテクスチャを解除しておく
	ID3D11ShaderResourceView* srv = nullptr;
	D3D.GetDevContext()->PSSetShaderResources(0, 1, &srv);

	// この関数でBeginした場合は、Endしておく
	if (!bBgn)End();
}

void SpriteShader::DrawPoint(int x, int y, const Math::Color* color)
{
	// もし開始していない場合は開始する(最後にEnd())
	bool bBgn = m_isBegin;
	if (!bBgn)Begin();

	// 白テクスチャ
	D3D.GetDevContext()->PSSetShaderResources(0, 1, D3D.GetWhiteTex()->GetSRViewAddress());

	// 色
	if (color) {
		m_cb0.Work().Color = *color;
	}
	m_cb0.Write();


	// 描画
	Vertex vertex[] = {
		{ {(float)x, (float)y, 0},	{0, 0} },
	};
	D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_POINTLIST, 1, vertex, sizeof(Vertex));

	// この関数でBeginした場合は、Endしておく
	if (!bBgn)End();
}

void SpriteShader::DrawLine(int x1, int y1, int x2, int y2, const Math::Color* color)
{
	// もし開始していない場合は開始する(最後にEnd())
	bool bBgn = m_isBegin;
	if (!bBgn)Begin();

	// 白テクスチャ
	D3D.GetDevContext()->PSSetShaderResources(0, 1, D3D.GetWhiteTex()->GetSRViewAddress());

	// 色
	if (color) {
		m_cb0.Work().Color = *color;
	}
	m_cb0.Write();


	// 描画
	Vertex vertex[] = {
		{ {(float)x1, (float)y1, 0},	{0, 0} },
		{ {(float)x2, (float)y2, 0},	{1, 0} },
	};
	D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 2, vertex, sizeof(Vertex));

	// この関数でBeginした場合は、Endしておく
	if (!bBgn)End();
}

void SpriteShader::DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const Math::Color* color, bool fill)
{
	// もし開始していない場合は開始する(最後にEnd())
	bool bBgn = m_isBegin;
	if (!bBgn)Begin();

	// 白テクスチャ
	D3D.GetDevContext()->PSSetShaderResources(0, 1, D3D.GetWhiteTex()->GetSRViewAddress());

	// 色
	if (color) {
		m_cb0.Work().Color = *color;
	}
	m_cb0.Write();


	// 描画
	Vertex vertex[] = {
		{ {(float)x1, (float)y1, 0},	{0, 0} },
		{ {(float)x2, (float)y2, 0},	{1, 0} },
		{ {(float)x3, (float)y3, 0},	{0, 0} },
		{ {(float)x1, (float)y1, 0},	{1, 0} },
	};
	D3D.DrawVertices(
		fill ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
		4, vertex, sizeof(Vertex));


	// この関数でBeginした場合は、Endしておく
	if (!bBgn)End();
}

void SpriteShader::DrawCircle(int x, int y, int radius, const Math::Color* color, bool fill)
{
	if (radius <= 0)return;

	// もし開始していない場合は開始する(最後にEnd())
	bool bBgn = m_isBegin;
	if (!bBgn)Begin();

	// 白テクスチャ
	D3D.GetDevContext()->PSSetShaderResources(0, 1, D3D.GetWhiteTex()->GetSRViewAddress());

	// 色
	if (color) {
		m_cb0.Work().Color = *color;
	}
	m_cb0.Write();

	// 頂点
	if (fill)
	{
		int faceNum = radius + 1;
		if (faceNum > 300)faceNum = 300;
		std::vector<Vertex> vertex(faceNum * 3);		// 半径により頂点数を調整

		// 描画
		for (int i = 0; i < faceNum; i++)
		{
			int idx = i * 3;
			vertex[idx].Pos.x = (float)x;
			vertex[idx].Pos.y = (float)y;

			vertex[idx+1].Pos.x = x + cos(DirectX::XMConvertToRadians(i * (360.0f / (faceNum - 1)))) * (float)radius;
			vertex[idx+1].Pos.y = y + sin(DirectX::XMConvertToRadians(i * (360.0f / (faceNum - 1)))) * (float)radius;
			vertex[idx+1].Pos.z = 0;

			vertex[idx+2].Pos.x = x + cos(DirectX::XMConvertToRadians((i+1) * (360.0f / (faceNum - 1)))) * (float)radius;
			vertex[idx+2].Pos.y = y + sin(DirectX::XMConvertToRadians((i+1) * (360.0f / (faceNum - 1)))) * (float)radius;
			vertex[idx+2].Pos.z = 0;
		}

		D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (int)vertex.size(), &vertex[0], sizeof(Vertex));
	}
	else
	{
		int numVertex = radius + 1;
		if (numVertex > 300)numVertex = 300;
		std::vector<Vertex> vertex(numVertex);		// 半径により頂点数を調整

		// 描画
		for (int i = 0; i < numVertex; i++)
		{
			vertex[i].Pos.x = x + cos(DirectX::XMConvertToRadians(i * (360.0f / (numVertex - 1)))) * (float)radius;
			vertex[i].Pos.y = y + sin(DirectX::XMConvertToRadians(i * (360.0f / (numVertex - 1)))) * (float)radius;
			vertex[i].Pos.z = 0;
		}

		D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, numVertex, &vertex[0], sizeof(Vertex));
	}

	// この関数でBeginした場合は、Endしておく
	if (!bBgn)End();
}

void SpriteShader::DrawBox(int x, int y, int extentX, int extentY, const Math::Color* color, bool fill)
{
	// もし開始していない場合は開始する(最後にEnd())
	bool bBgn = m_isBegin;
	if (!bBgn)Begin();

	// 白テクスチャ
	D3D.GetDevContext()->PSSetShaderResources(0, 1, D3D.GetWhiteTex()->GetSRViewAddress());

	// 色
	if (color) {
		m_cb0.Work().Color = *color;
	}
	m_cb0.Write();

	Math::Vector3 p1 = { (float)x - extentX, (float)y - extentY, 0 };
	Math::Vector3 p2 = { (float)x - extentX, (float)y + extentY, 0 };
	Math::Vector3 p3 = { (float)x + extentX, (float)y + extentY, 0 };
	Math::Vector3 p4 = { (float)x + extentX, (float)y - extentY, 0 };

	// 描画
	if (fill)
	{
		Vertex vertex[] = {
			{ p1, {0,0}},
			{ p2, {0,0}},
			{ p4, {0,0}},
			{ p3, {0,0}}
		};

		D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, vertex, sizeof(Vertex));
	}
	else
	{
		Vertex vertex[] = {
			{ p1, {0,0}},
			{ p2, {0,0}},
			{ p3, {0,0}},
			{ p4, {0,0}},
			{ p1, {0,0}}
		};

		D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 5, vertex, sizeof(Vertex));
	}

	// この関数でBeginした場合は、Endしておく
	if (!bBgn)End();
}

// 切り抜き範囲を設定する
// ・rect			… 範囲

void SpriteShader::SetScissorRect(const Math::Rectangle& rect)
{
	// ラスタライザステート作成・セット
	ID3D11RasterizerState* rs = D3D.CreateRasterizerState(D3D11_CULL_BACK, D3D11_FILL_SOLID, true, true);
	D3D.GetDevContext()->RSSetState(rs);
	rs->Release();

	D3D11_RECT rc;
	rc.left = rect.x;
	rc.top = rect.y;
	rc.right = rect.x + rect.width;
	rc.bottom = rect.y + rect.height;
	D3D.GetDevContext()->RSSetScissorRects(1, &rc);
}

