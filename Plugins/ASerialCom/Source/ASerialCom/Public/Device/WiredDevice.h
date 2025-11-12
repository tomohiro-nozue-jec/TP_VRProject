// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <ASerialCore/ASerialPacket.h>
#include "WiredDevice.generated.h"

class DeviceCommunicator;

/**
 * 
 */
UCLASS(BlueprintType, Category = "ASerial")
class ASERIALCOM_API UWiredDevice : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UWiredDevice();
	~UWiredDevice();

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

	virtual bool IsTickableInEditor() const override;

	/// <summary>
	/// 連絡者を初期化
	/// </summary>
	/// <param name="DeviceID">デバイスID</param>
	///  <param name="DeviceVersion">デバイスバージョン</param>
	///  <param name="DeviceName">デバイス名</param>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	void InitCommunicator(int DeviceID, int DeviceVersion, const FString& DeviceName);

	/// <summary>
	/// スレッドを一時停止
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	void PauseThread();

	/// <summary>
	/// スレッドを再開
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	void ReStartThread();

	/// <summary>
	/// デバイスとの連絡頻度を変更
	/// </summary>
	/// <param name="NewFrequency">新しい更新頻度</param>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	void ChangeFrequency(float NewFrequency);

	/// <summary>
	/// 名前をゲット
	/// </summary>
	/// <returns>名前</returns>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	FString GetName();

	/// <summary>
	/// デバイスと接続
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	void Connect();

	/// <summary>
	/// デバイスとのリンクを切断
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	void Disconnect();

	/// <summary>
	/// 接続しているか
	/// </summary>
	/// <returns>true: はい, false: いいえ</returns>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	bool IsConnecting() const;

	/// <summary>
	/// データを要求
	/// </summary>
	/// <param name="Cmd">データのコマンド</param>
	UFUNCTION(BlueprintCallable, Category = "ASerial")
	void RequestData(uint8 Cmd);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecieveDataDelegate, int, Value);

	/// <summary>
	/// データが貰ったの通知
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "ASerial")
	FOnRecieveDataDelegate OnRecieveDataEvent;

private:

	/// <summary>
	/// 貰ったデータから必要な数値に変換
	/// </summary>
	/// <param name="Data">データ</param>
	/// <param name="Size">データサイズ</param>
	/// <returns>数値</returns>
	template<typename T>
	T ConvertData(const uint8_t* Data, int Size)
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
	/// 貰ったデータを加工する
	/// </summary>
	/// <param name="Data">データ</param>
	void HandleData(ASerialDataStruct::ASerialData Data);

	/// <summary>
	/// データが貰ったことを通知する
	/// </summary>
	/// <param name="Data">データ</param>
	void NotifyRecieveData(int Data);

	/// <summary>
	/// 連絡者
	/// </summary>
	DeviceCommunicator* Communicator;

	/// <summary>
	/// 名前
	/// </summary>
	FString Name;

	/// <summary>
	/// 貰ったデータのキュー
	/// </summary>
	TQueue<ASerialDataStruct::ASerialData, EQueueMode::Spsc> DataQueue;
};
