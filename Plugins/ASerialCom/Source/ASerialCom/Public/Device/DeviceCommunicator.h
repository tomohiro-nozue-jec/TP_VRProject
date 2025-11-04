// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "../ASerialLibControllerWin.h"
#include "Containers/Queue.h"

enum class EProcessState
{
	Idle,           // 待機
	Sending,        // コマンドを送る
	WaitingResponse,// 返信を待つ
	Receiving       // 受け取る
};

/**
 * スレッド内でデバイスと接続し、データを受け渡します
 */
class ASERIALCOM_API DeviceCommunicator : public FRunnable
{
public:
	DeviceCommunicator(int ID, int Version, const TCHAR* Name);
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

#pragma region デバイス関連
public:

	/// <summary>
	/// 切断
	/// </summary>
	/// <returns>Succ: 成功, Fail: 失敗</returns>
	ExecutionResult Disconnect();

	/// <summary>
	/// コマンドを受ける
	/// </summary>
	void ReceiveCmd(int Cmd);

private:
	/// <summary>
	/// デバイスを生成
	/// </summary>
	void CreateDevice();

	/// <summary>
	/// 接続
	/// </summary>
	ExecutionResult Connect();

	/// <summary>
	/// 接続したか
	/// </summary>
	/// <returns>true: はい, false: いいえ</returns>
	bool IsConnected();

	/// <summary>
	/// コマンド処理
	/// </summary>
	void HandleCmd();

	/// <summary>
	/// コマンドをデバイスに送信する
	/// </summary>
	/// <param name="Command">コマンド</param>
	/// <returns>true: 成功, false: 失敗</returns>
	bool SendCommand(uint8_t Command);

	/// <summary>
	/// デバイスからの応答を待つ
	/// </summary>
	/// <param name="Data">データ</param>
	/// <returns>true: 成功, false: 失敗</returns>
	bool RecieveData(ASerialDataStruct::ASerialData& oData);

	/// <summary>
	/// 貰ったデータから必要な数値に変換
	/// </summary>
	/// <param name="Data">データ</param>
	/// <param name="Size">データサイズ</param>
	/// <returns>数値</returns>
	template<typename T>
	T TransformDataToInt(const uint8_t* Data, int Size)
	{
		//Data[0]が上位バイト, Data[1]が下位バイト
		T Result = 0;
		for (int i = 0; i < Size; ++i)
		{
			Result |= (Data[i] << (8 * (Size - 1 - i)));
		}
		return Result;
	}

	/// <summary>
	/// デバイス
	/// </summary>
	UASerialLibControllerWin* Device;

	/// <summary>
	/// 有線デバイスのID
	/// </summary>
	int DeviceID;

	/// <summary>
	/// 有線デバイスのバージョン
	/// </summary>
	int DeviceVer;

	/// <summary>
	/// 実行待ちコマンドのキュー
	/// </summary>
	TQueue<uint8_t, EQueueMode::Spsc> CommandQueue;

	/// <summary>
	/// 貰ったデータのキュー
	/// </summary>
	TQueue<ASerialDataStruct::ASerialData, EQueueMode::Spsc> DataQueue;

	/// <summary>
	/// プロセスの状態
	/// </summary>
	EProcessState ProcessState;

#pragma endregion
};