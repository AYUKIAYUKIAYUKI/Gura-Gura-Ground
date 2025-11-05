
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// オブジェクト、ヘッダーファイル [object.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// 名前空間OBJを定義
//****************************************************
namespace OBJ
{
	/* この場所はメンバに持たせるタイプ分け用のenumなどのみ定義しています */

	//****************************************************
	// タイプ識別用の列挙型を定義
	//****************************************************
	enum class TYPE : unsigned char
	{
		NONE = 0, // 無し
		PLAYER,   // プレイヤー
		MAX
	};

	//****************************************************
	// 描画優先度指定用の列挙型を定義
	//****************************************************
	enum class LAYER : unsigned char
	{
		BG = 0,  // 背景
		DEFAULT, // 通常
		FRONT,   // 前面
		STENCIL, // ステンシル
		UI,      // UI
		MAX
	};

	//****************************************************
	// トランスフォーム情報の構造体
	//****************************************************
	struct Transform
	{
		DirectX::XMFLOAT3 Size  = { 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4 Rot   = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 Pos   = { 0.0f, 0.0f, 0.0f };
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
	};

	//============================================================================
	// トランスフォーム情報から2D用のW行列を作成
	//============================================================================
	inline void CalcMtxWorldBy2DTransform(Transform& TF)
	{
		TF.World =
			DirectX::XMMatrixScaling(TF.Size.x, TF.Size.y, 0.0f) *
			DirectX::XMMatrixRotationZ(TF.Rot.z) *
			DirectX::XMMatrixTranslation(TF.Pos.x, TF.Pos.y, 0.0f);
	}

	//============================================================================
	// トランスフォーム情報から3D用のW行列を作成
	//============================================================================
	inline void CalcMtxWorldBy3DTransform(Transform& TF)
	{
		TF.World =
			DirectX::XMMatrixScaling(TF.Size.x, TF.Size.y, TF.Size.z) *
			DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(TF.Rot.x, TF.Rot.y, TF.Rot.z, TF.Rot.w)) *
			DirectX::XMMatrixTranslation(TF.Pos.x, TF.Pos.y, TF.Pos.z);
	}

	//============================================================================
	// ウィンドウサイズにおける画面真ん中の位置を作成
	//============================================================================
	inline DirectX::XMFLOAT2 CalcCenterOfWindow()
	{
		return { 1280.0f * 0.5f, 720.0f * 0.5f };
	}
}

//****************************************************
// 前方宣言
//****************************************************
class CObject;

//****************************************************
// コンセプトを定義
//****************************************************

// オブジェクトを継承していることのコンセプト
template <typename T>
concept DerivedFromObject = std::derived_from<T, CObject>;

#define CONCEPT_ε 0
#define CONCEPT_ρ 0
// オブジェクトのポインタを受け取る関数オブジェクトであることのコンセプト
#if CONCEPT_ε
template <typename T>
concept ObjectFactoryFunction = requires(T t, CObject * pObj)
{
	{ t(pObj) } -> std::same_as<bool>;
};
#elif CONCEPT_ρ
concept ObjectFactoryFunction = std::is_pointer_v<T>
&& std::is_function_v<std::remove_pointer_t<T>>;
#endif

//****************************************************
// オブジェクトクラスを定義
//****************************************************
class CObject
{
public:

	//****************************************************
	// special function
	//****************************************************

	// コンストラクタ
	CObject(OBJ::TYPE Type, OBJ::LAYER Layer);

	// デストラクタ
	virtual ~CObject() = default;

	//****************************************************
	// virtual function
	//****************************************************
	virtual void Update() = 0; // 更新処理
	virtual void Draw()   = 0; // 描画処理

	//****************************************************
	// function
	//****************************************************

	// 生成処理
#if CONCEPT_ε || CONCEPT_ρ
	template <DerivedFromObject T, ObjectFactoryFunction U>
	static T* Create(OBJ::TYPE Type, OBJ::LAYER Layer, U&& fpInitialize);
#else
	template <DerivedFromObject T, typename U>
	static T* Create(OBJ::TYPE Type, OBJ::LAYER Layer, U&& fpInitialize);
#endif

	//****************************************************
	// inline function
	//****************************************************

	// タイプの操作用
	inline OBJ::TYPE GetType() const         { return m_Type; }
	inline void      SetType(OBJ::TYPE Type) { m_Type = Type; }

	// 描画優先度の操作用
	inline OBJ::LAYER GetLayer() const           { return m_Layer; }
	inline void       SetLayer(OBJ::LAYER Layer) { m_Layer = Layer; }

	// 死亡フラグの設定
	inline bool GetDeath() const             { return m_bDeath; }
	inline void SetDeath(bool bState = true) { m_bDeath = bState; }

private:

	//****************************************************
	// data
	//****************************************************
	OBJ::TYPE  m_Type;   // オブジェクトタイプ
	OBJ::LAYER m_Layer;  // 描画優先度
	bool       m_bDeath; // 死亡フラグ
};

// テンプレート実装ファイル
#include "API.object.tpp"