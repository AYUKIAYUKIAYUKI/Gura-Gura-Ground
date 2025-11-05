
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// ユースフル、ヘッダーファイル [useful.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// プリプロセッサディレクティブ
//****************************************************
#define NONUSE_DX9

//****************************************************
// 名前空間usefulを定義
//****************************************************
namespace useful
{
	//****************************************************
	// using宣言
	//****************************************************
	using std::unique_ptr;
	using std::make_unique;
	using std::to_string;

	//****************************************************
	// 型エイリアスを定義
	//****************************************************
	
	// std::unique_ptr
	template <typename T> using up = unique_ptr<T>;

#ifndef NONUSE_DX9
	using XCol = D3DXCOLOR;
	using Vec2 = D3DXVECTOR2;
	using Vec3 = D3DXVECTOR3;
	using Mtx  = D3DXMATRIX;
#endif // NONUSE_DX9

#ifndef NONUSE_DX11
	using Vec2 = DirectX::XMFLOAT2;
	using Vec3 = DirectX::XMFLOAT3;
	using Vec4 = DirectX::XMFLOAT4;
	using VecX = DirectX::XMVECTOR;
	using Mtx  = DirectX::XMMATRIX;
#endif // NONUSE_DX11

	//****************************************************
	// コンセプトの定義
	//****************************************************

#ifndef NONUSE_DX9
	// メンバにSRTの現在値・目標値の両方ががあるクラスであることのコンセプト
	template <typename T>
	concept HasSRTAndChangeable = requires(T t, const Vec3& SRT)
	{
		{ t.GetPos() }    -> std::same_as<const Vec3&>;
		{ t.SetPos(SRT) } -> std::same_as<void>;

		{ t.GetPosTarget() }    -> std::same_as<const Vec3&>;
		{ t.SetPosTarget(SRT) } -> std::same_as<void>;

		{ t.GetRot() }    -> std::same_as<const Vec3&>;
		{ t.SetRot(SRT) } -> std::same_as<void>;

		{ t.GetRotTarget() }    -> std::same_as<const Vec3&>;
		{ t.SetRotTarget(SRT) } -> std::same_as<void>;

		{ t.GetSize() }    -> std::same_as<const Vec3&>;
		{ t.SetSize(SRT) } -> std::same_as<void>;

		{ t.GetSizeTarget() }    -> std::same_as<const Vec3&>;
		{ t.SetSizeTarget(SRT) } -> std::same_as<void>;
	};
#endif // NONUSE_DX9
	
	//****************************************************
	// 静的定数を定義
	//****************************************************

#ifndef NONUSE_DX9
	// D3DXCOLOR初期化用
	static const XCol XCOL_ZERO_INIT = XCol(0.0f, 0.0f, 0.0f, 0.0f);
	static const XCol XCOL_ONE_INIT  = XCol(1.0f, 1.0f, 1.0f, 1.0f);

	// D3DXVECTOR2初期化用
	static const Vec2 VEC2_ZERO_INIT = Vec2(0.0f, 0.0f);
	static const Vec2 VEC2_ONE_INIT  = Vec2(1.0f, 1.0f);

	// D3DXVECTOR3初期化用
	static const Vec3 VEC3_ZERO_INIT = Vec3(0.0f, 0.0f, 0.0f);
	static const Vec3 VEC3_ONE_INIT  = Vec3(1.0f, 1.0f, 1.0f);
#endif // NONUSE_DX9

#ifndef NONUSE_DX11
	// DirectX::XMFLOAT2初期化用
	static const Vec2 VEC2_ZERO_INIT = Vec2(0.0f, 0.0f);
	static const Vec2 VEC2_ONE_INIT  = Vec2(1.0f, 1.0f);

	// DirectX::XMFLOAT3初期化用
	static const Vec3 VEC3_ZERO_INIT = Vec3(0.0f, 0.0f, 0.0f);
	static const Vec3 VEC3_ONE_INIT  = Vec3(1.0f, 1.0f, 1.0f);

	// DirectX::XMFLOAT34初期化用
	static const Vec4 VEC4_ZERO_INIT = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
	static const Vec4 VEC4_ONE_INIT  = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
#endif // NONUSE_DX11

#ifndef NONUSE_DX9
	//****************************************************
	// レイ情報用構造体を定義
	//****************************************************
	struct RAY
	{
		// デフォルトコンストラクタ
		RAY()
			: Origin(VEC3_ZERO_INIT)
			, Direction(VEC3_ZERO_INIT)
		{}

		Vec3 Origin;	// レイの始点
		Vec3 Direction;	// レイの方向
	};
#endif // NONUSE_DX9

	//****************************************************
	// インライン関数テンプレートを定義
	//****************************************************

	// '~' 演算子のオーバーロード
	template <typename T, typename U = unsigned char>
	inline T operator~(T l)
	{
		return static_cast<T>(~static_cast<U>(l));
	}

	// '&' 演算子のオーバーロード
	template <typename T, typename U = unsigned char>
	inline T operator&(T l, T r)
	{
		return static_cast<T>(static_cast<U>(l) & static_cast<U>(r));
	}

	// '|' 演算子のオーバーロード
	template <typename T, typename U = unsigned char>
	inline T operator|(T l, T r)
	{
		return static_cast<T>(static_cast<U>(l) | static_cast<U>(r));
	}

	/**
	 * 論理積の比較文の省略形です、列挙子に'None'の定義が前提です
	 *
	 * Flags -> 'None'と比較したいフラグ
	 */
	template <typename T>
	inline bool AndCheck(T Flags)
	{
		return Flags != T::None;
	}

	/**
	 * ポインタ型のメンバをnullチェック
	 * 返り値はチェックしたポインタをそのまま
	 *
	 * Ptr   -> チェックの対象
	 * pName -> エラーログ用の変数名
	 */
	template <typename T> inline T* PtrCheck(T* Ptr, const char* pName)
	{
		if (!Ptr)
		{
			throw std::runtime_error(static_cast<std::string>(pName) + "is nullptr");
		}

		return Ptr;
	}

	/**
	 * 汎用合成のみ、例外処理などは一切行いません
	 * 返り値無し
	 * 
	 * up        -> 合成先のポインタの参照 
	 * fpFactory -> ファクトリ関数
	 */
	template <typename T, typename U>
	inline void Compose(std::unique_ptr<T>& up, U&& fpFactory)
	{
		up = std::forward<U>(fpFactory)();
	}

	/**
	 * 線形補間
	 * 返り値は毎回の計算結果
	 *
	 * Base   -> 現在値
	 * Target -> 目標値
	 * fRate  -> 減衰の強度
	 */
	template <typename T>
	inline T Lerp(const T& Base, const T& Target, float fRate)
	{
		return Base + (Target - Base) * fRate;
	}

	/**
	 * 指数減衰
	 * 返り値無し
	 *
	 * Base   -> 現在値
	 * Target -> 目標値
	 * fCoef  -> 減衰の強度
	 */
	template <typename T>
	inline void ExponentialDecay(T& Base, T Target, float fCoef)
	{
		Base += (Target - Base) * fCoef;
	}

	/**
	 * オリジナル乱数生成
	 * 返り値は指定した型の乱数
	 */
	template <typename T>
	inline T GetRandomValue()
	{
		// 数値の取る範囲を設定
		const int nRange = 180;

		// 倍の範囲からランダムな整数を生成
		int nRandom{ rand() % (nRange * 2) };

		// もとの範囲を超えていた場合正負を反転
		if (nRandom > nRange)
		{
			nRandom %= nRange;
			nRandom *= -1;
		}

		// 整数を指定された型にキャスト
		return static_cast<T>(nRandom);
	}

	/**
	 * オリジナル乱数生成2
	 * 返り値は指定した型の乱数
	 */
	template <typename T> 
	T GetRandomValue(T Range)
	{
		// 数値の取る範囲を設定
		const int nRange = static_cast<int>(Range);

		// 倍の範囲からランダムな整数を生成
		int nRandom{ rand() % (nRange * 2) };

		// もとの範囲を超えていた場合正負を反転
		if (nRandom > nRange)
		{
			nRandom %= nRange;
			nRandom *= -1;
		}

		// 戻り値の型は引数依存
		return static_cast<T>(nRandom);
	}

	/**
	 * ダウンキャスト
	 * 返り値は指定した型のポインタ
	 * 
	 * pBase -> 変更したい型のポインタ
	 */
	template <typename T1, typename T2>
	T1* DownCast(T2* pBase)
	{
		// 引数のオブジェクトを指定されたクラスにダウンキャスト
		T1* pObj = dynamic_cast<T1*>(pBase);

		// ダウンキャスト失敗
		if (!pObj)
		{
			throw std::bad_cast();
		}

		return pObj;
	}

	//****************************************************
	// 関数テンプレートを定義
	//****************************************************

	/**
	 * 汎用生成、前提として 'Inirialzie()' と 'Release()' をメンバに持つクラス向け
	 * 返り値はインスタンスのポインタ
	 */
	template <typename T>
	T* Create();

	/**
	 * 汎用ユニークポインタ生成、前提として 'Inirialzie()' と 'Release()' をメンバに持つクラス向け
	 * テンプレート引数に指定された型のデフォルトコンストラクタで生成します
	 * 返り値はインスタンスのユニークポインタ
	 */
	template <typename T>
	up<T> CreateUniquePtr();

	/**
	 * 汎用解放、前提として 'Release()' をメンバに持つクラス向け
	 * 返り値無し
	 *
	 * p -> 解放したいインスタンスを参照
	 */
	template <typename T>
	void Release(T*& p);

	/**
	 * unique_ptr型の中身を、テンプレート引数に指定された型のデフォルトコンストラクタでmake_uniqueし交換する
	 * 返り値無し
	 *
	 * up -> メモリ解放対象、および新規インスタンスの格納先のunique_ptr
	 */
	template <typename T, typename U>
	void ChangeUniquePtr(up<T>& up);

#ifndef NONUSE_DX9
	/**
	 * オブジェクトの向きをYaw回転
	 * 返り値無し
	 *
	 * Base  -> 'GetRot()' と 'SetRot()' をメンバに持つオブジェクト限定
	 * fCoef -> 加算する回転量、速さや回転方向を指定
	 */
	template <typename T>
	void RotateYawAxis(T& Base, float fCoef);

	/**
	 * オブジェクトの目標向きをYaw回転
	 * 返り値無し
	 *
	 * Base  -> 'GetRotTarget()' と 'SetRotTarget()' をメンバに持つオブジェクト限定
	 * fCoef -> 加算する回転量、速さや回転方向を指定
	 */
	template <typename T>
	void RotateTargetYawAxis(T& Base, float fCoef);
#endif // NONUSE_DX9

	//****************************************************
	// プロトタイプ宣言
	//****************************************************

#ifndef NONUSE_DX9
	/**
	 * 頂点フォーマットの持つテクスチャマッピング情報を初期化します
	 * 返り値無し
	 *
	 * rVtx -> 頂点フォーマットのポインタ参照、必ずロックされた直後のポインタを渡してください
	 */
	void InitializeTextureMappingInVertex(VERTEX_2D*& rVtx);
	void InitializeTextureMappingInVertex(VERTEX_3D*& rVtx);

	/**
	 * ワールド行列の計算、引数でゼロ指定するとその要素を無視します
	 * 返り値無し
	 *
	 * mtxWorld -> ワールド行列
	 * Scale    -> 拡縮
	 * Rot      -> 向き
	 * Pos      -> 座標
	 */
	void UpdateMtxWorld(Mtx& mtxWorld, const Vec3& Scale, const Vec3& Rot, const Vec3& Pos);

	/**
	 * マウス位置を元にして、ワールド空間上のレイ情報を作成します
	 * 返り値は作成されたレイ情報
	 * 
	 * pDev -> デバイス
	 * hWnd -> ウィンドウハンドル
	 */
	RAY CalcRayInWorldFromMousePos(LPDIRECT3DDEVICE9 pDev, HWND& hWnd);

	/**
	 * 対象のメッシュに応じて、ワールド空間内のレイ情報をローカライズ
	 * 返り値は変換されたレイ情報
	 * 
	 * mtxWolrdFromMesh -> 対象のメッシュのワールド行列
	 * Ray              -> レイ情報
	 */
	RAY LocalizeRayForTargetMesh(const D3DXMATRIX& mtxWolrdFromMesh, const RAY& Ray);

	/**
	 * メッシュとレイの当たり判定
	 * 返り値は接触点の座標、接触が無ければ初期化情報
	 * 
	 * pMesh        -> 対象のメッシュ
	 * RayLocalized -> ローカライズされたレイ情報
	 */
	D3DXVECTOR3 RaycastForMesh(const LPD3DXBASEMESH& pMesh, const RAY& RayLocalized);

	/**
	 * 目標値へ角度を合わせる、補間系を使用する際など
	 * 返り値無し
	 *
	 * fAngle  -> 現在の角度
	 * fDest   -> 目標の角度
	 * fRadius -> 増減量
	 */
	void NormalizeAngleToDest(float& fAngle, float fDest);
	void NormalizeAngleToDest(float& fAngle, float fDest, float fRadius);

	/**
	 * 角度を範囲内に収める、360度回す際など
	 * 返り値無し
	 * 
	 * fAngle1・2 -> この値を360度内に変換
	 */
	void WrapAngleToRange(float& fAngle);
	void WrapAngleToRange(float& fAngle1, float& fAngle2);
#endif // NONUSE_DX9

	/**
	 * JSONファイルの展開
	 * 返り値は
	 * 
	 * FilePath -> JSONファイルのパス指定
	 */
	nlohmann::json OpenJsonFile(const std::string& FilePath);

	/**
	 * 例外送出を持ったJSONファイルの展開
	 * 返り値は
	 *
	 * FilePath -> JSONファイルのパス指定
	 */
	nlohmann::json OpenJsonFileMaybeThrow(const std::string& FilePath);

	/**
	 * UTF8のJSONデータをSJISに変換
	 * 返り値はSJISの文字列
	 *
	 * Json -> 要素指定されたデータ
	 */
	std::string JsonConvertToSJIS(const nlohmann::json& Json);

#ifndef NONUSE_DX9
	/**
	 * JSONデータをD3DXVECTOR2に変換
	 * 返り値はD3DXVECTOR2
	 *
	 * Json -> 要素指定されたデータ
	 */
	D3DXVECTOR2	JsonConvertToVec2(const nlohmann::json& Json);

	/**
	 * JSONデータをD3DXVECTOR3に変換
	 * 返り値はD3DXVECTOR3
	 *
	 * Json -> 要素指定されたデータ
	 */
	D3DXVECTOR3	JsonConvertToVec3(const nlohmann::json& Json);

	/**
	 * JSONデータをD3DXCOLORに変換
	 * 返り値はD3DXCOLOR
	 *
	 * Json -> 要素指定されたデータ
	 */
	D3DXCOLOR JsonConvertToXCol(const nlohmann::json& Json);
#endif // NONUSE_DX9

	//****************************************************
	// 名前空間MISを定義
	//****************************************************
	namespace MIS
	{
		/* ImGuiを使用する際の自分向けショートカットです */

		/**
		 * ImGuiのWindowを生成、必ず'ImGui::End()'とセットで使用すること
		 * 返り値はウィンドウ生成の可否
		 */
		bool MyImGuiShortcut_BeginWindow(const char* pName);
		bool MyImGuiShortcut_BeginWindow(const char* pName, const ImVec2& Rect);
		bool MyImGuiShortcut_BeginWindow(const char* pName, const ImVec2& Rect, const ImVec2& Pos);
		bool MyImGuiShortcut_BeginWindow(const char* pName, const ImVec2& Rect, const ImVec2& Pos, ImGuiWindowFlags Flags);

		/**
		 * ボタンのウィジェットを、該当行の残りの幅に均等に敷き詰めるためのサイズを算出します
		 * 必ず右詰めになります、これらの左に説明を記載する際は先行してテキストを配置すること
		 * 返り値はボタン1つのサイズ
		 */
		ImVec2 CalcButtonSpacing(unsigned short wNum);
		ImVec2 CalcButtonSpacing(unsigned short wNum, float fHeight);

#ifndef NONUSE_DX9
		/**
		 * Vector系構造体のドラッグ操作
		 * 返り値無し
		 */
		void MyImGuiShortcut_DragVector(const char* pLabel, Vec2& Vec2, float fSpeed);
		void MyImGuiShortcut_DragVector(const char* pLabel, Vec3& Vec3, float fSpeed);

		/**
		 * Vector系構造体のリセット操作
		 */
		void MyImGuiShortcut_ResetVector(const char* pLabel, Vec2& Vec2);
		void MyImGuiShortcut_ResetVector(const char* pLabel, Vec2& Vec2, float fInitValue);
		void MyImGuiShortcut_ResetVector(const char* pLabel, Vec3& Vec3);
		void MyImGuiShortcut_ResetVector(const char* pLabel, Vec3& Vec3, float fInitValue);
#endif // NONUSE_DX9
	}
}

// テンプレート実装ファイル
#include "API.useful.tpp"