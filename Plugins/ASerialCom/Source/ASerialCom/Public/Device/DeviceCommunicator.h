// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

class UASerialLibControllerWin;

/**
 * 
 */
class ASERIALCOM_API DeviceCommunicator : public FRunnable
{
public:
	DeviceCommunicator(const TCHAR* Name);
	~DeviceCommunicator();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

#pragma region スレッド関連
public:

	/// <summary>
	/// 安全に終了する
	/// </summary>
	void EnsureCompletion();

	/// <summary>
	/// 一時停止する
	/// </summary>
	void Pause();

	/// <summary>
	/// 再開する
	/// </summary>
	void ReStart();

private:

	/// <summary>
	/// 今のスレッドを管理する
	/// </summary>
	FRunnableThread* Thread;

	/// <summary>
	/// 更新頻度(デフォルト：60)
	/// </summary>
	float UpdateFrequency;

	/// <summary>
	/// 動けるか
	/// </summary>
	bool bIsRunning;

	/// <summary>
	/// 一時停止フラグ
	/// </summary>
	bool bIsPause;

#pragma endregion

};
