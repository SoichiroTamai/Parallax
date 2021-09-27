#include "System/System.h"

#include "Texture.h"


// 2D画像(resource)リソースから、最適なビューを作成する
// ・resource		… 2D画像リソース
// ・ppSRV			… 作成されたShaderResourceViewを受け取るための変数のアドレス
// ・ppRTV			… 作成されたRenderTargetViewを受け取るための変数のアドレス
// ・ppDSV			… 作成されたDepthStencilViewを受け取るための変数のアドレス
static bool CreateViewsFromTexture2D(ID3D11Texture2D* resource, ID3D11ShaderResourceView** ppSRV, ID3D11RenderTargetView** ppRTV, ID3D11DepthStencilView** ppDSV)
{
	// リソースが無い
	if (resource == nullptr)return false;

	// テクスチャ本体の情報取得
	D3D11_TEXTURE2D_DESC desc;
	resource->GetDesc(&desc);

	//===========================================================
	//
	// RenderTargetViewを作成する
	//
	//===========================================================

	// レンダーターゲットフラグがついてる時のみ
	if (ppRTV && desc.BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		// 作成するビューの設定
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = desc.Format;	// Format
		// 単品のテクスチャ(通常テクスチャ)の場合
		if (desc.ArraySize == 1) {
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;			// 単品テクスチャ
		}
		// テクスチャ配列の場合
		else {
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;		// テクスチャ配列
			rtvDesc.Texture2DArray.ArraySize = desc.ArraySize;			// 要素数
			rtvDesc.Texture2DArray.FirstArraySlice = 0;
			rtvDesc.Texture2DArray.MipSlice = 0;
		}

		// レンダーターゲットビュー作成
		HRESULT hr = D3D.GetDev()->CreateRenderTargetView(resource, &rtvDesc, ppRTV);
		if (FAILED(hr))
		{
			assert(0 && "RenderTargetViewの作成に失敗");
			return false;
		}
	}

	//===========================================================
	//
	// ShaderResourceViewの情報を作成する
	//
	//===========================================================
	// シェーダリソースビューフラグがついてる時のみ
	if (ppSRV && desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		// 作成するビューの設定
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		// テクスチャがZバッファの場合は、最適なフォーマットにする
		if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			switch (desc.Format) {
				// 16ビット
			case DXGI_FORMAT_R16_TYPELESS:
				srvDesc.Format = DXGI_FORMAT_R16_UNORM;
				break;
				// 32ビット
			case DXGI_FORMAT_R32_TYPELESS:
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
				break;
				// 24ビット(Zバッファ) + 8ビット(ステンシルバッファ) 
			case DXGI_FORMAT_R24G8_TYPELESS:
				srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;
			default:
				assert(0 && "[ShaderResource] 対応していないフォーマットです");
				break;
			}
		}
		// Zバッファでない場合は、そのまま同じフォーマットを使用
		else
		{
			srvDesc.Format = desc.Format;
		}

		// 単品のテクスチャ(通常テクスチャ)の場合
		if (desc.ArraySize == 1)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			if (srvDesc.Texture2D.MipLevels <= 0)srvDesc.Texture2D.MipLevels = -1;
		}
		// テクスチャ配列の場合
		else {
			// さらにキューブマップの場合
			if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) {
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			}
			// 通常テクスチャ配列
			else {
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			}
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
			srvDesc.Texture2DArray.ArraySize = desc.ArraySize;	// 要素数
			srvDesc.Texture2DArray.FirstArraySlice = 0;
		}

		// シェーダリソースビュー作成
		HRESULT hr = D3D.GetDev()->CreateShaderResourceView(resource, &srvDesc, ppSRV);
		if (FAILED(hr))
		{
			assert(0 && "ShaderResourceViewの作成に失敗");
			return false;
		}
	}

	//===========================================================
	//
	// DepthStencilViewを作成する
	//
	//===========================================================
	// Zバッファフラグがついてる時のみ
	if (ppDSV && desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) {
		// 作成するビューの設定
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

		// テクスチャー作成時に指定したフォーマットと互換性があり、深度ステンシルビューとして指定できるフォーマットを指定する
		switch (desc.Format) {
			// 16ビット
		case DXGI_FORMAT_R16_TYPELESS:
			dsvDesc.Format = DXGI_FORMAT_D16_UNORM;
			break;
			// 32ビット
		case DXGI_FORMAT_R32_TYPELESS:
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			break;
			// 24ビット(Zバッファ) + 8ビット(ステンシルバッファ) 
		case DXGI_FORMAT_R24G8_TYPELESS:
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default:
			assert(0 && "[DepthStencil] 対応していないフォーマットです");
			break;
		}

		// 単品のテクスチャ(通常テクスチャ)の場合
		if (desc.ArraySize == 1) {
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
		}
		// テクスチャ配列の場合
		else {
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = desc.ArraySize;
			dsvDesc.Texture2DArray.MipSlice = 0;
			dsvDesc.Texture2DArray.FirstArraySlice = 0;
		}

		//-------------------------------
		// デプスステンシルビュー作成
		//-------------------------------
		HRESULT hr = D3D.GetDev()->CreateDepthStencilView(resource, &dsvDesc, ppDSV);
		if (FAILED(hr)) {
			assert(0 && "DepthStencilViewの作成に失敗");
			return false;
		}
	}

	return true;
}


// リソースを取得
ID3D11Texture2D* Texture::GetResource() const
{
	if (m_srv == nullptr)return nullptr;

	ID3D11Resource* res;
	m_srv->GetResource(&res);

	ID3D11Texture2D* tex2D;
	if (FAILED(res->QueryInterface<ID3D11Texture2D>(&tex2D)))
	{
		res->Release();
		return nullptr;
	}
	res->Release();
	tex2D->Release();

	return tex2D;
}

bool Texture::Load(const std::string & filename, bool renderTarget, bool depthStencil, bool generateMipmap)
{
	Release();
	if (filename.empty())return false;

	// ファイル名をWideCharへ変換
	std::wstring wFilename = sjis_to_wide(filename);

	//------------------------------------
	// 画像読み込み
	//------------------------------------

	// Bind Flags
	UINT bindFlags = 0;
	bindFlags |= D3D11_BIND_SHADER_RESOURCE;
	if (renderTarget)bindFlags |= D3D11_BIND_RENDER_TARGET;
	if (depthStencil)bindFlags |= D3D11_BIND_DEPTH_STENCIL;


	// ※DirectX Texライブラリを使用して画像を読み込む

	DirectX::TexMetadata meta;
	DirectX::ScratchImage image;

	bool bLoaded = false;

	// WIC画像読み込み
	//  WIC_FLAGS_ALL_FRAMES … gifアニメなどの複数フレームを読み込んでくれる
	if (SUCCEEDED(DirectX::LoadFromWICFile(wFilename.c_str(), DirectX::WIC_FLAGS_ALL_FRAMES, &meta, image)))
	{
		bLoaded = true;
	}

	// DDS画像読み込み
	if (bLoaded == false) {
		if (SUCCEEDED(DirectX::LoadFromDDSFile(wFilename.c_str(), DirectX::DDS_FLAGS_NONE, &meta, image)))
		{
			bLoaded = true;
		}
	}

	// TGA画像読み込み
	if (bLoaded == false) {
		if (SUCCEEDED(DirectX::LoadFromTGAFile(wFilename.c_str(), &meta, image)))
		{
			bLoaded = true;
		}
	}

	// HDR画像読み込み
	if (bLoaded == false) {
		if (SUCCEEDED(DirectX::LoadFromHDRFile(wFilename.c_str(), &meta, image)))
		{
			bLoaded = true;
		}
	}

	// 読み込み失敗
	if (bLoaded == false)
	{
		return false;
	}

	// ミップマップ生成
	if (meta.mipLevels == 1 && generateMipmap)
	{
		DirectX::ScratchImage mipChain;
		if (SUCCEEDED(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain)))
		{
			image.Release();
			image = std::move(mipChain);
		}
	}

	//------------------------------------
	// テクスチャリソース作成
	//------------------------------------
	ID3D11Texture2D* tex2D = nullptr;
	if (FAILED(DirectX::CreateTextureEx(
		D3D.GetDev(),						// Direct3D Device
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		D3D11_USAGE_DEFAULT,								// Usage
		bindFlags,							// Bind Flags
		0,									// CPU Access Flags
		0,									// MiscFlags
		false,								// ForceSRGB
		(ID3D11Resource**)&tex2D)
	)) {

		return false;
	}

	//------------------------------------
	// テクスチャリソース(m_resource)から、各ビューを作成する
	//------------------------------------
	if (CreateViewsFromTexture2D(tex2D, &m_srv, &m_rtv, &m_dsv) == false)
	{
		tex2D->Release();
		Release();
		return false;
	}

	// 画像情報取得
	tex2D->GetDesc(&m_desc);
	tex2D->Release();

	m_filepath = filename;

	return true;
}

bool Texture::Create(ID3D11Texture2D* pTexture2D)
{
	Release();
	if (pTexture2D == nullptr)return false;

	//---------------------------------------------
	// 画像リソースから、各ビューを作成する
	//---------------------------------------------
	if (CreateViewsFromTexture2D(pTexture2D, &m_srv, &m_rtv, &m_dsv) == false) {
		Release();
		return false;
	}

	// ファイル名記憶
	m_filepath = "";

	// 画像情報取得
	pTexture2D->GetDesc(&m_desc);

	return true;

}

bool Texture::Create(const D3D11_TEXTURE2D_DESC & desc, const D3D11_SUBRESOURCE_DATA * fillData)
{
	Release();

	//--------------------------------------------
	// 2Dテクスチャの生成
	//--------------------------------------------
	ID3D11Texture2D* resource;
	HRESULT hr = D3D.GetDev()->CreateTexture2D(&desc, fillData, &resource);
	if (FAILED(hr)) {
		Release();
		return false;
	}

	//--------------------------------------------
	// m_pTexture2Dから、各ビューを作成する
	//--------------------------------------------
	if (CreateViewsFromTexture2D(resource, &m_srv, &m_rtv, &m_dsv) == false)
	{
		Release();
		return false;
	}

	// 画像情報取得
	resource->GetDesc(&m_desc);
	resource->Release();

	return true;
}

bool Texture::Create(int w, int h, DXGI_FORMAT format, UINT arrayCnt, const D3D11_SUBRESOURCE_DATA * fillData)
{
	Release();

	// 作成する2Dテクスチャ設定
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = format;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Width = (UINT)w;
	desc.Height = (UINT)h;
	desc.CPUAccessFlags = 0;
	desc.MipLevels = 1;
	desc.ArraySize = arrayCnt;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	// 作成
	if (Create(desc, fillData) == false)return false;

	return true;
}

bool Texture::CreateRenderTarget(int w, int h, DXGI_FORMAT format, UINT arrayCnt, const D3D11_SUBRESOURCE_DATA* fillData, UINT miscFlags)
{
	Release();

	// 作成する2Dテクスチャ設定
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = format;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.Width = (UINT)w;
	desc.Height = (UINT)h;
	desc.CPUAccessFlags = 0;
	desc.MipLevels = 1;
	desc.ArraySize = arrayCnt;
	desc.MiscFlags = miscFlags;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	// 作成
	if (Create(desc, fillData) == false)return false;

	return true;
}

bool Texture::CreateDepthStencil(int w, int h, DXGI_FORMAT format, UINT arrayCnt, const D3D11_SUBRESOURCE_DATA* fillData, UINT miscFlags)
{
	Release();

	// 作成する2Dテクスチャ設定
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = format;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	desc.Width = (UINT)w;
	desc.Height = (UINT)h;
	desc.CPUAccessFlags = 0;
	desc.MipLevels = 1;
	desc.ArraySize = arrayCnt;
	desc.MiscFlags = miscFlags;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	// 作成
	if (Create(desc, fillData) == false)return false;

	return true;
}

void Texture::SetSRView(ID3D11ShaderResourceView* srv)
{
	if (srv == nullptr)return;

	Release();
	m_srv = srv;	// セット
	m_srv->AddRef();// 参照カウンタを増やしておく

	// 画像情報取得
	GetResource()->GetDesc(&m_desc);
}

void Texture::Release()
{
	SafeRelease(m_srv);
	SafeRelease(m_rtv);
	SafeRelease(m_dsv);

	m_filepath = "";
}
