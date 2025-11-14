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
};

/**
 * スレッド内でデバイスと接続し、データを受け渡します
 */
class ASERIALCOM_API DeviceCommunicator : public FRunnable
{
public:
	/// <summary>
	/// コンストラクタ, キューでデータを受け渡す
	/// </summary>
	/// <param name="ID">デバイスID</param>
	/// <param name="Version">デバイスバージョン</param>
	/// <param name="Name">スレッド名</param>
	/// <param name="InDataQueue">データキュー</param>
	DeviceCommunicator(int ID, int Version, const TCHAR* Name, TQueue<ASerialDataStruct::ASerialData, EQueueMode::Spsc>* InDataQueue);
	~DeviceCommunicator();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

#pragma region スレッド関連
public:

	/// <summary>
	/// 一時停止する
	/// </summary>
	void Pause();

	/// <summary>
	/// 再開する
	/// </summary>
	void ReStart();

	/// <summary>
	/// 更新頻度を変更
	/// </summary>
	/// <param name="NewFrequency">新しい更新頻度</param>
	void ChangeFrequency(float NewFrequency);

private:

	/// <summary>
	/// 安全に終了する
	/// </summary>
	void EnsureCompletion();

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
	/// 接続
	/// </summary>
	ExecutionResult Connect();

	/// <summary>
	/// 切断
	/// </summary>
	/// <returns>Succ: 成功, Fail: 失敗</returns>
	ExecutionResult Disconnect();

	/// <summary>
	/// 接続しているか
	/// </summary>
	/// <returns>true: はい, false: いいえ</returns>
	bool IsConnecting();

	/// <summary>
	/// コマンドを受ける
	/// </summary>
	void ReceiveCmd(uint8_t Cmd);

private:
	/// <summary>
	/// デバイスを生成
	/// </summary>
	void CreateDevice();

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
	TQueue<ASerialDataStruct::ASerialData, EQueueMode::Spsc>* DataQueue;

	/// <summary>
	/// プロセスの状態
	/// </summary>
	EProcessState ProcessState;

#pragma endregion
};