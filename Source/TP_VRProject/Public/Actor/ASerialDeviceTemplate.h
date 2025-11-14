// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASerialDeviceTemplate.generated.h"

class UWiredDevice;

UCLASS()
class TP_VRPROJECT_API AASerialDeviceTemplate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASerialDeviceTemplate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	/// <summary>
	/// コマンドを送信
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void SendCmd();

	void Connect();

	void Disconnect();

	void PauseThread();

	void ReStartThread();

protected:

	/// <summary>
	/// デバイスを初期化
	/// </summary>
	UFUNCTION(BlueprintNativeEvent)
	void InitializeDevice();

private:

	/// <summary>
	/// デバイスからデータを受け取ったときの処理
	/// </summary>
	UFUNCTION()
	void OnDataReceived(int Value);

	/// <summary>
	/// デバイス
	/// </summary>
	/// ここに必ずUPROPERTY()をつけること。UWiredDeviceはUObjectであるため、付けないとガベージコレクションで消される可能性がある。
	UPROPERTY()
	UWiredDevice* SerialDevice;

	/// <summary>
	/// 有線デバイスのID
	/// </summary>
	int WiredDeviceID;

	/// <summary>
	/// 有線デバイスのバージョン
	/// </summary>
	int WiredDeviceVer;

	/// <summary>
	/// RPMのコマンド
	/// </summary>
	unsigned char RPMCmd;
};
