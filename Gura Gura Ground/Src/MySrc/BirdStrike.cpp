//============================================================================
// 
// 鳥の群れ [BirdStrike.cpp]
// Author : 元地弘汰
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "BirdStrike.h"
#include "API.gltf.manager.h"
#include <random>
#include "API.object.manager.h"
#include "player.h"
#include "API.collision.h"
#include "API.sound.manager.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.ghost.h"

// エフェクト
#include "route.h"
#include "Shadow.h"
#include "dust.h"

//****************************************************
// usingディレクティブ
//****************************************************
using namespace DirectX;
using namespace useful;

//****************************************************
// 無名名前空間の定義
//****************************************************
namespace
{
	// フィールドサイズ
	float g_fFieldSpan = 15.0f;
	float g_fFieldHalf = g_fFieldSpan * 0.5f;

	// その他設定値
	float ShpireHalf = 3.0f;		//生成球半径
	float SideHeight = 8.0f;		//生成高さ
	int MoveTime = 60 * 2;			//移動時間
	std::vector<XMFLOAT3> Target;	//始点・終点を設定するための配列


	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{

	}

	std::vector<XMFLOAT3> InitalizePosVec(float height)
	{
		std::vector<XMFLOAT3> targets;
		for (int i = 0; i < 7; ++i)
		{
			targets.push_back({ -20.0f + (40.f / 7.f * i), height, 20.0f });
		}
		for (int i = 0; i < 4; ++i)
		{
			targets.push_back({ 20.0f, height, 20.0f - (40.f / 7.f * i) });
		}
		for (int i = 0; i < 7; ++i)
		{
			targets.push_back({ 20.0f - (40.f / 7.f * i), height, -20.0f });
		}
		for (int i = 0; i < 4; ++i)
		{
			targets.push_back({ -20.0f, height, -20.0f + (40.f / 7.f * i) });
		}
		for (int i = 0; i < 4; ++i)
		{
			targets.erase(targets.begin() + ((i + 1) * (7 - i)));
		}
		return targets;
	}

	namespace A {
		int GetRandomMT(int min, int max) {
			std::random_device rnd;				// 非決定的な乱数生成器でシード生成機を生成
			std::mt19937 mt(rnd());				//  メルセンヌツイスターの32ビット版、引数は初期シード
			std::uniform_int_distribution<> rand_num(min, max);     // 指定範囲無いの一様乱数
			return (rand_num(mt));
		}
		XMFLOAT3 LerpFloat3(const XMFLOAT3& a, const XMFLOAT3& b, float t)
		{
			return {
				a.x + (b.x - a.x) * t,
				a.y + (b.y - a.y) * t,
				a.z + (b.z - a.z) * t
			};
		}
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBirdStrike::CBirdStrike(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::MOVING)
	, m_nTime(0)
{
	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Bird"));
}

//============================================================================
// デストラクタ
//============================================================================
CBirdStrike::~CBirdStrike()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CBirdStrike::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// デフォルトのゴーストを生成
	SetCollider(CGhost::CreateGhost(GetTransform(), Collision::SHAPETYPE::SPHERE, fWidth, fHeight, fDepth));

	// コライダーをゴーストにキャスト
	CGhost* const pGs = dynamic_cast<CGhost*>(GetCollider());

	//パラメーター値取得
	const auto& param = m_ObstacleEditer.m_ParamSets[GetParamSetIndex()].subParams[GetSubParamIndex()];
	float height = param.ObstacleSpawnY;

	//配列を設定
	m_Targets = InitalizePosVec(height);

	int TargetMax = m_Targets.size() - 1;

	//始点と終点を設定
	int StartNum = A::GetRandomMT(0, TargetMax);
	XMFLOAT3 Vec3 = m_Targets[StartNum];
	int GoalNum = StartNum + A::GetRandomMT(8, 11);
	if (GoalNum > TargetMax) GoalNum -= TargetMax;

	m_Start = m_Targets[StartNum];
	m_Goal = m_Targets[GoalNum];
	OBJ::Transform transform{};

	transform.Pos = Vec3;
	pGs->SetWorldTransform(transform);
	/* ！！！ 影の生成 ！！！ */
	CShadow* pShadow = CObjectManager::CreateRaw<CShadow>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT);
	pShadow->SetTrackTarget(shared_from_this());

	/* ！！！ 警告表示の作成 ！！！ */
	CRoute* pRoute = CObjectManager::CreateRaw<CRoute>();
	std::shared_ptr<CObstacle> spObstacle = std::dynamic_pointer_cast<CObstacle>(shared_from_this());
	pRoute->SetTrackTarget(spObstacle);

	// 効果音：跳ねる音
	CSoundManger::RefInstance().Play("BirdStrike", false, -0.5f, 1.2f);}

//============================================================================
// 更新処理
//============================================================================
void CBirdStrike::Update()
{
	// 挙動
	Action();

	//プレイヤーとの接触
	ToPlayer();

	// 障害物の更新
	CObstacle::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBirdStrike::Draw()
{
	// 障害物の描画
	CObstacle::Draw();
}

//============================================================================
// 挙動
//============================================================================
void CBirdStrike::Action()
{
	if (m_nTime > MoveTime)SetDeath();
	CGhost* const pGs = dynamic_cast<CGhost*>(GetCollider());

	//経過の割合を時間で計測
	float timeValue = (1.0f / MoveTime) * m_nTime;

	XMFLOAT3 F3Value = A::LerpFloat3(m_Start, m_Goal, timeValue);

	OBJ::Transform transform = pGs->GetWorldTransform();
	transform.Pos = F3Value;

	pGs->SetWorldTransform(transform);

	//時間を経過させる
	++m_nTime;
}

//============================================================================
// プレイヤーとの接触
//============================================================================
void CBirdStrike::ToPlayer()
{
	CGhost* const pGs = dynamic_cast<CGhost*>(GetCollider());
	// オブジェクトのリストを取得
	const auto& rPlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	for (const auto& e : rPlayerList)
	{
		std::shared_ptr<CPlayer> pPlayerObj = std::dynamic_pointer_cast<CPlayer>(e);

		// プレイヤー型にキャスト可能なら
		if (pPlayerObj)
		{
			CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(pPlayerObj->GetCollider());
			// 衝突判定

			Collision::MyContactCallbackGhostAndRigidBody CallBack(pGs, pRigidBody);
			CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pGs->GetGhost(), pRigidBody->GetRigidBody(), CallBack);

			// 衝突が確認出来たら
			if (CallBack.m_bHit)
			{
				pPlayerObj->EnableBird();
			}
		}
	}
}