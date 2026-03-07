// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BluetoothDeviceTemplate.generated.h"

//===================================================
// BLE UUID 定義（このActorが使うデバイス固有の値）
// ここに書いているService UUIDやCharacteristic UUIDは仮の物です
// 電子科から提供してきたデバイスのServiceやCharacteristicに替えてください
//===================================================
// Service UUID
#define TEMPLATE_PAIR_SERVICE_UUID    TEXT("52f93fe3-013b-4256-8d87-fdcea414b533")
// Write Characteristic UUID
#define TEMPLATE_PAIR_CHARACTERISTIC_UUID       TEXT("abaae5f2-4829-121c-1595-0e80c6f0e78a")
// Read / Notify Characteristic UUID
#define TEMPLATE_RPM_CHARACTERISTIC_UUID        TEXT("682a0468-1482-63be-dc47-4296d65ad4b9")

class UBluetoothDevice;

/*
* UBluetoothDevice の使用方法を示すテンプレート Actor。
 *
 * ── 初期化シーケンス ──
 *   BeginPlay
 *     └─ BLE初期化 → スキャン開始
 *          └─ OnBleConnected（接続成功）
 *               └─ Enqueue: Write ペアリング要求
 *                    └─ OnBleWriteCompleted（ペアリング完了を確認）
 *                         └─ Enqueue: Subscribe RPM
 *                         └─ Enqueue: Subscribe Revolution
 *                              └─ OnBleQueueEmpty（初期化完了）
 *
 * ── 継続受信 ──
 *   OnBleDataReceived
 *     ├─ RPM Characteristic         → HandleRPMData()
 *     ├─ Revolution Characteristic  → HandleRevolutionData()
 *     └─ LED Color Characteristic   → HandleLEDColorData()
*/
UCLASS()
class TP_VRPROJECT_API ABluetoothDeviceTemplate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABluetoothDeviceTemplate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	//===================================================
	// UBluetoothDevice イベントハンドラ
	// （OnConnected で Enqueue、OnWriteCompleted で次の Enqueue という
	//   チェーン形式で初期化シーケンスを構成する）
	//===================================================

	/// <summary>
	/// 接続成功時に呼ばれる。
	/// ここでペアリング Write を Enqueue して初期化シーケンスを開始する。
	/// </summary>
	UFUNCTION()
	void OnBleConnected(const FString& DeviceName, const FString& DeviceUUID);

	/// <summary>
	/// 接続失敗時に呼ばれる。
	/// </summary>
	UFUNCTION()
	void OnBleConnectFailed(const FString& ErrorMessage);

	/// <summary>
	/// 切断完了時に呼ばれる。
	/// </summary>
	UFUNCTION()
	void OnBleDisconnected();

	/// <summary>
	/// Write完了時に呼ばれる。
	/// どの Write が終わったかを ServiceUUID / CharacteristicUUID で判断し、
	/// 次に必要な操作を Enqueue する。
	/// </summary>
	UFUNCTION()
	void OnBleWriteCompleted(const FString& ServiceUUID, const FString& CharacteristicUUID);

	/// <summary>
	/// データ受信時に呼ばれる。
	/// ServiceUUID / CharacteristicUUID で処理を振り分ける。
	/// </summary>
	UFUNCTION()
	void OnBleDataReceived(const FString& ServiceUUID, const FString& CharacteristicUUID, const TArray<uint8>& Data);

	/// <summary>
	/// Queue がすべて空になった時に呼ばれる。
	/// 初期化シーケンス完了の確認などに使う。
	/// </summary>
	UFUNCTION()
	void OnBleQueueEmpty();

	//===================================================
	// 受信データ処理（振り分け先）
	//===================================================

	/// <summary>
	/// バイト配列をビッグエンディアンで数値に変換するユーティリティ。
	/// Data[0] が上位バイト、Data[Size-1] が下位バイト。
	/// </summary>
	template<typename T>
	T BytesToInt(const uint8* Data, int32 Size) const;

	//===================================================
	// メンバ変数
	//===================================================

	/// <summary>
	/// BLE接続・Queue管理を担う UBluetoothDevice インスタンス
	/// </summary>
	UPROPERTY()
	UBluetoothDevice* BluetoothDevice;

	/// <summary>
	/// 初期化シーケンスが完了したかどうか
	/// </summary>
	bool bIsInitialized;
};
