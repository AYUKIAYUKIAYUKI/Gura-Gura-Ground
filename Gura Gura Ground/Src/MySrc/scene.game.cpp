//============================================================================
// 
// ゲームシーン [scene.game.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "scene.game.h"

// 遷移先のシーン
#include "scene.manager.h"
#include "scene.title.h"

// インプット取得のため
#include "API.input.manager.h"

// オブジェクト生成・破棄のため
#include "API.renderer.h"
#include "API.object.manager.h"
#include "field.h"
#include "player.h"
#include "IronBall.h"

#include "gimmick.manager.h"

/* デバッグ */
namespace
{
	// 任意のジオメトリのセットアップ
	const auto Init = [](CAny* pAny) -> bool
		{
			// 立方体の各軸への辺の長さ
			const float fSpan = 0.25f;
			const float fHalf = fSpan * 0.5f;

			// ジオメトリのバインド
			//pAny->SetGeometry(DirectX::GeometricPrimitive::CreateSphere(CRenderer::RefInstance().GetContext(), fSpan, 64, false));
			pAny->SetGeometry(DirectX::GeometricPrimitive::CreateBox(CRenderer::RefInstance().GetContext(), { fSpan, fSpan, fSpan }, false, true));

			// 形状設定
			//pAny->SetCollisionShape(std::make_unique<btSphereShape>(fHalf));
			pAny->SetCollisionShape(std::make_unique<btBoxShape>(btVector3(fHalf, fHalf, fHalf)));

			// 初期姿勢設定
			const btQuaternion InitRot(1.0f, 1.0f, 1.0f, 1.0f);
			const btVector3    InitPos(0.0f, 10.0f, 0.0f);
			pAny->SetMotionState(std::make_unique<btDefaultMotionState>(btTransform(InitRot, InitPos)));

			// 質量設定
			const btScalar  Mass = 0.02f;
			btVector3 Inertia(0.0f, 0.0f, 0.0f);
			pAny->GetCollisionShape()->calculateLocalInertia(Mass, Inertia);

			// 剛体情報
			btRigidBody::btRigidBodyConstructionInfo boxCI(Mass, pAny->GetMotionState().get(), pAny->GetCollisionShape().get(), Inertia);
			pAny->SetRigidBody(std::make_unique<btRigidBody>(boxCI));

			// 弾性係数をセット
			pAny->GetRigidBody()->setRestitution(0.8f);

			// 摩擦力をセット
			//pAny->GetRigidBody()->setFriction(0.25f);

			// 回転抵抗をセット
			//pAny->GetRigidBody()->setRollingFriction(0.25f);

			// ワールドに追加
			CWorld::RefInstance().AddRigidBody(pAny->GetRigidBody());

			return true;
		};

	void AAA()
	{
		for (int i = 0; i < 1; ++i)
		{
			CObject::Create<CAny>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT, Init);
		}
	}

	// コントローラー複数接続のテスト
	void BBB()
	{
		if (CInputManager::RefInstance().GetTrackerGamePad(0).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
		{
			AAA();
		}

		if (CInputManager::RefInstance().GetTrackerGamePad(1).b == DirectX::GamePad::ButtonStateTracker::PRESSED)
		{
			AAA();
			AAA();
		}

		if (CInputManager::RefInstance().GetTrackerGamePad(2).y == DirectX::GamePad::ButtonStateTracker::PRESSED)
		{
			AAA();
			AAA();
			AAA();
		}
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneGame::CSceneGame()
	: m_pPlayer(nullptr)
{
	// フィールドのジオメトリのセットアップ
	const auto InitFiled = [](CField* pField) -> bool
		{
			// ジオメトリのバインド
			pField->SetGeometry(DirectX::GeometricPrimitive::CreateBox(CRenderer::RefInstance().GetContext(), { 10.0f, 0.2f, 10.0f }, false, true));
			//pField->SetGeometry(DirectX::GeometricPrimitive::CreateCylinder(CRenderer::RefInstance().GetContext(), 0.2f, 10.0f, 64, false));

			// 形状設定
			pField->SetCollisionShape(std::make_unique<btBoxShape>(btVector3(5.0f, 0.1f, 5.0f)));
			//pField->SetCollisionShape(std::make_unique<btCylinderShape>(btVector3(5.0f, 0.1f, 5.0f)));

			// 初期姿勢設定
			const btQuaternion InitRot(0.0f, 0.0f, 0.0f, 1.0f);
			const btVector3    InitPos(0.0f, 1.1f, 0.0f);
			pField->SetMotionState(std::make_unique<btDefaultMotionState>(btTransform(InitRot, InitPos)));

			// 質量設定
			const btScalar Mass = 1.0f;
			btVector3 Inertia(0.0f, 0.0f, 0.0f);
			pField->GetCollisionShape()->calculateLocalInertia(Mass, Inertia);

			// 剛体情報
			btRigidBody::btRigidBodyConstructionInfo FieldCI(Mass, pField->GetMotionState().get(), pField->GetCollisionShape().get(), Inertia);
			pField->SetRigidBody(std::make_unique<btRigidBody>(FieldCI));

			// ワールドにリジッドボディを追加
			CWorld::RefInstance().AddRigidBody(pField->GetRigidBody());

			/*--------------------------------------------------------------*/

			// ジオメトリのバインド
			pField->SetGeometryC(DirectX::GeometricPrimitive::CreateCone(CRenderer::RefInstance().GetContext(), 1.0f, 1.0f, 64, false));

			// 形状設定
			pField->SetCollisionShapeC(std::make_unique<btConeShape>(0.5f, 1.0f));

			// 初期姿勢設定
			const btQuaternion InitRotC(0.0f, 0.0f, 0.0f, 1.0f);
			const btVector3    InitPosC(0.0f, 0.5f, 0.0f);
			pField->SetMotionStateC(std::make_unique<btDefaultMotionState>(btTransform(InitRotC, InitPosC)));

			// 質量設定
			const btScalar MassC = 100.0f;
			btVector3 InertiaC(0.0f, 0.0f, 0.0f);
			pField->GetCollisionShapeC()->calculateLocalInertia(MassC, InertiaC);

			// 剛体情報
			btRigidBody::btRigidBodyConstructionInfo ConeCI(MassC, pField->GetMotionStateC().get(), pField->GetCollisionShapeC().get(), InertiaC);
			pField->SetRigidBodyC(std::make_unique<btRigidBody>(ConeCI));

			// ワールドにリジッドボディを追加
			CWorld::RefInstance().AddRigidBody(pField->GetRigidBodyC());

			/*--------------------------------------------------------------*/

			// フィールドに…コーンの先端？を原点として、シリンダーの傾きを制限するヒンジの作成…？
			btVector3 PivotF = { 0.0f, -0.1f, 0.0f };
			btVector3 PivotC = { 0.0f,  0.5f, 0.0f };
			pField->SetConstraintC(std::make_unique<btPoint2PointConstraint>(*pField->GetRigidBody().get(), *pField->GetRigidBodyC().get(), PivotF, PivotC));

			// ヒンジの接続の強さを設定
			//pField->GetConstraintC()->setParam();

			// ワールドにヒンジを追加
			CWorld::RefInstance().RefDynamicsWorldConst()->addConstraint(pField->GetConstraintC().get(), false);

			return true;
		};

	// フィールドの生成
	CObject::Create<CField>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT, InitFiled);

	// プレイヤーの生成
	if (!m_pPlayer)
	{
		m_pPlayer = CObject::Create<CPlayer>(OBJ::TYPE::PLAYER, OBJ::LAYER::DEFAULT, CPlayer::s_fpDefaultFactory);
	}

	//ギミックマネージャーの生成
	CGimmickManager::RefInstance();
}

//============================================================================
// デストラクタ
//============================================================================
CSceneGame::~CSceneGame()
{}

//============================================================================
// 更新処理
//============================================================================
void CSceneGame::Update()
{
	BBB();

	if (CInputManager::RefInstance().EnhancedEnter())
	{
		Change();
	}

	// シーンの更新
	CGimmickManager::RefInstance().Update();
}

//============================================================================
// シーン変更
//============================================================================
void CSceneGame::Change()
{
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAllObject();

	// タイトルシーンへ
	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneTitle>());
}