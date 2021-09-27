#include"ModelComponent.h"
#include "../Component/CameraComponent.h"

#include "../Game/Scene.h"

void ModelComponent::SetModedl(const std::shared_ptr<ModelInfo>& rModel)
{
	// 使用しているモデルをセット
	m_spModel = rModel;

	// 念の為、コピー用配列をクリア
	m_coppiedNodes.clear();

	// ノードのコピー
	if (rModel)
	{
		m_coppiedNodes = rModel->GetOriginalNodes();
	}
}

void ModelComponent::Draw()
{
	// 有効じゃない時はスキップ
	if (m_enable == false) { return; }

	// モデルが無い時はスキップ
	if (m_spModel == nullptr) { return; }

	// 描画設定
	SHADER.m_modelShader.SetToDevice();

	// 全てのノードを一つ一つ描画
	for (UINT i = 0; i < m_coppiedNodes.size(); i++)
	{
		auto& rNode = m_coppiedNodes[i];
		
		//メッシュがない時はスキップ
		if (rNode.m_spMesh == nullptr) { continue; }

		
		//// 視錐台カリング ( 視錐台の範囲外の物体は描画しない ) < ------------------------------------------
		//// -----------
		//// 物体
		////------------
		//// メッシュのAABBから、物体の行列も考慮してOBbへ変換
		//DirectX::BoundingOrientedBox obb;
		//DirectX::BoundingOrientedBox::CreateFromBoundingBox(obb, rNode.m_spMesh->GetBoundingBox());
		//obb.Transform(obb, rNode.m_localTransform);


		//// -----------
		//// 視錐台
		////------------
		//// カメラの情報から視錐台のデータを作成
		//DirectX::BoundingFrustum vf;
		//DirectX::BoundingFrustum::CreateFromMatrix(vf, m_owner.GetCameraComponent()->GetProjtMatrix());
		//Matrix mCam = m_owner.GetCameraComponent()->GetViewtMatrix();
		//mCam.Inverse();											// ビュー行列を逆行列にすると、カメラ行列になる
		//vf.Origin = mCam.GetTranslation();						// カメラの座標
		//vf.Orientation = Quaternion().CreatFormMatrix(vf,mCam);	// カメラの回転

		//if (vf.Intersects(obb))
		//{
			// ------------------------------------------------------------------------------------------------>
			// 行列セット
			SHADER.m_modelShader.SetWorldMatrix(rNode.m_localTransform * m_owner.GetMatrix());
			// 描画
			SHADER.m_modelShader.DrawMesh(rNode.m_spMesh.get(), m_spModel->GetMaterials());

		//}
	}

	// 画面中央にオブジェクトがある？
	if (Scene::GetInstance().GetCenterRayResult().m_hit)
	{
		// 画面中央のオブジェクトの参照先を作成
		std::shared_ptr<GameObject> sp_gameObj = Scene::GetInstance().GetCenterObject().lock();

		// 移動中可能なオブジェクト？
		if (sp_gameObj->GetTag() == TAG_MoveObject)
		{
			// 画面中央のオブジェクトと名前が同じ？
			if (sp_gameObj->GetName() == m_owner.GetName())
			{
				//------------------------
				// 輪郭描画
				//------------------------

				// 表面をカリング(非表示)にするラスタライザステートをセット
				D3D.GetDevContext()->RSSetState(SHADER.m_rs_CullFront);
				//輪郭シェーダーの情報を設定.
				SHADER.m_modelShader.SetToDevice_Outline();

				for (UINT i = 0; i < m_coppiedNodes.size(); i++)
				{
					auto& rNode = m_coppiedNodes[i];

					// メッシュがない場合はスキップ
					if (rNode.m_spMesh == nullptr) { continue; }
					if (rNode.m_name == "Light") { continue; }
					if (rNode.m_name == "Camera") { continue; }

					// 行列セット
					SHADER.m_modelShader.SetWorldMatrix(rNode.m_localTransform * m_owner.GetMatrix());

					// 描画
					SHADER.m_modelShader.DrawMesh_Outline(rNode.m_spMesh.get());
				}

				// 裏面をカリング(非表示)にするラスタライザステートに戻す
				D3D.GetDevContext()->RSSetState(SHADER.m_rs_CullBack);
			}
		}
	}
}

void ModelComponent::DrawShadowMap()
{
	// 有効じゃないときはスキップ
	if (m_enable == false)return;
	// モデルがないときはスキップ
	if (m_spModel == nullptr)return;


	// 全ノード(メッシュ)を描画
	for (UINT i = 0; i < m_coppiedNodes.size(); i++)
	{
		auto& rNode = m_coppiedNodes[i];

		// メッシュがない場合はスキップ
		if (rNode.m_spMesh == nullptr) { continue; }

		// 行列セット
		SHADER.m_genShadowMapShader.SetWorldMatrix(rNode.m_localTransform * m_owner.GetMatrix());

		// 描画
		SHADER.m_genShadowMapShader.DrawMeshDepth(rNode.m_spMesh.get(), m_spModel->GetMaterials());
	}
}