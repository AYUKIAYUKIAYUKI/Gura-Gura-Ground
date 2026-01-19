//============================================================================
// 
// デバフ状態管理クラス [debuff_behavior.h]
// Author : 元地弘汰
// 
//============================================================================

#pragma once



//デバフ周りの数値を管理するクラス群
class Debuff_Behavior
{
public:
	Debuff_Behavior() {}
	virtual void Initialize() = 0;
	//設定した値を返す関数
	float GetDecayValue() { return m_DecayValue; }
	float GetInertiaValue() { return m_InertiaValue; }
	bool GetTimer() {
		--m_Timer;
		return m_Timer > 0;
	}
	//時間だけ戻す
	virtual void TimerReset() { m_Timer = 0; }
protected:
	float m_DecayValue;		//減速の倍率
	float m_InertiaValue;	//慣性の倍率
	int m_Timer;
};

//潰されデバフ
class Stamp_DB : public Debuff_Behavior
{
public:
	Stamp_DB() { Initialize(); }
	void Initialize()override {
		m_Timer = 180;
		m_DecayValue = 0.3f;
		m_InertiaValue = 1.0f;
	}
	void TimerReset()override { m_Timer = 180; }
protected:

};

//鳥纏いデバフ
class Bird_DB : public Debuff_Behavior
{
public:
	Bird_DB() { Initialize(); }
	void Initialize()override {
		m_Timer = 240;
		m_DecayValue = 0.5f;
		m_InertiaValue = 1.2f;
	}
	void TimerReset()override { m_Timer = 240; }
protected:

};

//オイルデバフ
class Oil_DB : public Debuff_Behavior
{
public:
	Oil_DB() { Initialize(); }
	void Initialize()override {
		m_Timer = 60;
		m_DecayValue = 0.8f;
		m_InertiaValue = 8.5f;
	}
	void TimerReset()override { m_Timer = 60; }
protected:

};