// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BluetoothDevice.generated.h"

//===================================================
// BLE Service UUID / Device Name
// (外部から設定する場合は SetTargetDeviceName() を使用)
//===================================================
#define BLE_DEFAULT_DEVICE_NAME "DefaultDevice"

// Android Permission
#define ANDROID_FILE_LOCATION_PERMISSION  "android.permission.ACCESS_FINE_LOCATION"
#define ANDROID_BLUETOOTH_CONNECT_PERMISSION "android.permission.BLUETOOTH_CONNECT"
#define ANDROID_BLUETOOTH_SCAN_PERMISSION "android.permission.BLUETOOTH_SCAN"

class IBleManagerInterface;
class IBleDeviceInterface;

//===================================================
// BLE 接続状態
//===================================================
UENUM(BlueprintType)
enum class EBleConnectionState : uint8
{
	Disconnected   UMETA(DisplayName = "Disconnected"),
	Scanning       UMETA(DisplayName = "Scanning"),
	Connecting     UMETA(DisplayName = "Connecting"),
	Connected      UMETA(DisplayName = "Connected"),
	Disconnecting  UMETA(DisplayName = "Disconnecting"),
};

//===================================================
// BLE Operation Queue
//===================================================

/// <summary>
/// BLE操作の種類
/// </summary>
UENUM()
enum class EBleOperationType : uint8
{
	WriteCharacteristic,       // データ書き込み
	SubscribeCharacteristic,   // 通知購読
	ReadCharacteristic,        // データ読み取り
};

/// <summary>
/// Queueに積む一つのBLE操作を表す構造体
/// </summary>
struct FBleOperation
{
	EBleOperationType Type;
	FString ServiceUUID;
	FString CharacteristicUUID;
	TArray<uint8> Data;
	bool bWithResponse = false;

	static FBleOperation MakeWrite(const FString& Service, const FString& Characteristic, const TArray<uint8>& WriteData)
	{
		FBleOperation Op;
		Op.Type = EBleOperationType::WriteCharacteristic;
		Op.ServiceUUID = Service;
		Op.CharacteristicUUID = Characteristic;
		Op.Data = WriteData;
		return Op;
	}

	static FBleOperation MakeSubscribe(const FString& Service, const FString& Characteristic, bool bResponse = false)
	{
		FBleOperation Op;
		Op.Type = EBleOperationType::SubscribeCharacteristic;
		Op.ServiceUUID = Service;
		Op.CharacteristicUUID = Characteristic;
		Op.bWithResponse = bResponse;
		return Op;
	}

	static FBleOperation MakeRead(const FString& Service, const FString& Characteristic)
	{
		FBleOperation Op;
		Op.Type = EBleOperationType::ReadCharacteristic;
		Op.ServiceUUID = Service;
		Op.CharacteristicUUID = Characteristic;
		return Op;
	}
};

//===================================================
// Delegates（外部へのイベント通知）
//===================================================

/// <summary>接続成功通知</summary>
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBleConnectedDelegate, const FString&, DeviceName, const FString&, DeviceUUID);

/// <summary>接続失敗通知</summary>
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBleConnectFailedDelegate, const FString&, ErrorMessage);

/// <summary>切断通知</summary>
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBleDisconnectedDelegate);

/// <summary>Write完了通知（外部がどのWriteが終わったか判断できる）</summary>
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBleWriteCompletedDelegate, const FString&, ServiceUUID, const FString&, CharacteristicUUID);

/// <summary>データ受信通知</summary>
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBleDataReceivedDelegate, const FString&, ServiceUUID, const FString&, CharacteristicUUID, const TArray<uint8>&, Data);

/// <summary>全Queue操作完了通知</summary>
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBleQueueEmptyDelegate);

/**
 * BLE接続・切断・スキャンと、OperationQueueの管理のみを担う純粋な基盤クラス。
 * どのService/CharacteristicにRead/Write/Subscribeするかは外部コードが決定する。
 * 
 * 使い方:
 *   1. OnConnected に接続後の処理（EnqueueWrite/Read/Subscribe の呼び出し）をバインドする
 *   2. OnDataReceived に受信データの処理をバインドする
 *   3. Initialize() → StartScan() の順で呼ぶ
 */
UCLASS(BlueprintType, Category = "Bluetooth Device")
class TP_VRPROJECT_API UBluetoothDevice : public UObject
{
	GENERATED_BODY()

public:
	UBluetoothDevice();
	virtual ~UBluetoothDevice();

	/// <summary>
	/// BLEマネジャーの初期化・Bluetooth有効確認・権限要求を行う。
	/// StartScan()の前に必ず呼ぶこと。
	/// </summary>
	/// <returns>true: 初期化成功 / false: BLE非対応またはエラー</returns>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth")
	bool Initialize();

	/// <summary>
	/// デバイスのスキャンを開始する。
	/// TargetDeviceName と一致するデバイスが見つかったら自動で接続する。
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth")
	void StartScan();

	/// <summary>
	/// スキャンを停止する。
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth")
	void StopScan();

	/// <summary>
	/// デバイスとの接続を切断する。
	/// </summary>
	/// <returns>true: 切断要求送信成功</returns>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth")
	bool Disconnect();

	//===================================================
	// 設定
	//===================================================

	/// <summary>
	/// 接続対象のデバイス名を設定する（デフォルト: BLE_DEFAULT_DEVICE_NAME）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth")
	void SetTargetDeviceName(const FString& DeviceName);

	/// <summary>
	/// スキャン時にフィルタするサービスUUIDを追加する（省略可）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth")
	void AddScanServiceFilter(const FString& ServiceUUID);

	/// <summary>
	/// サービスフィルタをクリアする
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth")
	void ClearScanServiceFilter();

	//===================================================
	// Operation Queue（外部から操作を積む）
	//===================================================

	/// <summary>
	/// Write操作をQueueに追加する
	/// </summary>
	/// <param name="ServiceUUID">サービスUUID</param>
	/// <param name="CharacteristicUUID">キャラクタリスティックUUID</param>
	/// <param name="Data">書き込むデータ</param>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth|Queue")
	void EnqueueWrite(const FString& ServiceUUID, const FString& CharacteristicUUID, const TArray<uint8>& Data);

	/// <summary>
	/// Subscribe操作をQueueに追加する
	/// </summary>
	/// <param name="ServiceUUID">サービスUUID</param>
	/// <param name="CharacteristicUUID">キャラクタリスティックUUID</param>
	/// <param name="bWithResponse">レスポンスありかどうか</param>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth|Queue")
	void EnqueueSubscribe(const FString& ServiceUUID, const FString& CharacteristicUUID, bool bWithResponse = false);

	/// <summary>
	/// Read操作をQueueに追加する
	/// </summary>
	/// <param name="ServiceUUID">サービスUUID</param>
	/// <param name="CharacteristicUUID">キャラクタリスティックUUID</param>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth|Queue")
	void EnqueueRead(const FString& ServiceUUID, const FString& CharacteristicUUID);

	/// <summary>
	/// Queueに残っている操作をすべてクリアする
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Bluetooth|Queue")
	void ClearQueue();

	//===================================================
	// 状態取得
	//===================================================

	/// <summary>
	/// 現在の接続状態を返す
	/// </summary>
	UFUNCTION(BlueprintPure, Category = "Bluetooth")
	EBleConnectionState GetConnectionState() const;

	/// <summary>
	/// 接続中かどうか
	/// </summary>
	UFUNCTION(BlueprintPure, Category = "Bluetooth")
	bool IsConnected() const;

	/// <summary>
	/// 接続中のデバイス名を返す（未接続時は空文字）
	/// </summary>
	UFUNCTION(BlueprintPure, Category = "Bluetooth")
	FString GetConnectedDeviceName() const;

	/// <summary>
	/// 接続中のデバイスUUIDを返す（未接続時は空文字）
	/// </summary>
	UFUNCTION(BlueprintPure, Category = "Bluetooth")
	FString GetConnectedDeviceUUID() const;

	/// <summary>
	/// Queueに操作が残っているかどうか
	/// </summary>
	UFUNCTION(BlueprintPure, Category = "Bluetooth|Queue")
	bool HasPendingOperations() const;

	//===================================================
	// イベント（外部がバインドして使う）
	//===================================================

	/// <summary>接続成功時に発行される</summary>
	UPROPERTY(BlueprintAssignable, Category = "Bluetooth|Events")
	FOnBleConnectedDelegate OnConnected;

	/// <summary>接続失敗時に発行される</summary>
	UPROPERTY(BlueprintAssignable, Category = "Bluetooth|Events")
	FOnBleConnectFailedDelegate OnConnectFailed;

	/// <summary>切断完了時に発行される</summary>
	UPROPERTY(BlueprintAssignable, Category = "Bluetooth|Events")
	FOnBleDisconnectedDelegate OnDisconnected;

	/// <summary>Write完了時に発行される</summary>
	UPROPERTY(BlueprintAssignable, Category = "Bluetooth|Events")
	FOnBleWriteCompletedDelegate OnWriteCompleted;

	/// <summary>データ受信時に発行される（Read/Subscribe通知）</summary>
	UPROPERTY(BlueprintAssignable, Category = "Bluetooth|Events")
	FOnBleDataReceivedDelegate OnDataReceived;

	/// <summary>Queueの全操作が完了した時に発行される</summary>
	UPROPERTY(BlueprintAssignable, Category = "Bluetooth|Events")
	FOnBleQueueEmptyDelegate OnQueueEmpty;

private:

	//===================================================
	// BLE 内部処理
	//===================================================
	
	/// <summary>
	/// Bluetooth周りの検査
	/// </summary>
	/// <returns></returns>
	bool CheckBluetooth();

	/// <summary>
	/// アンドロイドのbluetoothの権限を要求する
	/// </summary>
	void RequestAndroidPermission();

	/// <summary>
	/// 権限の要求結果
	/// </summary>
	/// <param name="Permissions"></param>
	/// <param name="GrantResults"></param>
	UFUNCTION()
	void OnPermissionResult(const TArray<FString>& Permissions, const TArray<bool>& GrantResults);
	
	/// <summary>
	/// 新しいデバイスが見つかった時
	/// </summary>
	/// <param name="Device">デバイス</param>
	UFUNCTION()
	void OnDeviceFound(TScriptInterface<IBleDeviceInterface> Device);
	
	/// <summary>
	/// コネクションが成功した時
	/// </summary>
	UFUNCTION()
	void OnConnectSucc();
	
	/// <summary>
	/// コネクションが失敗した時
	/// </summary>
	/// <param name="ErrorMessage">エラーメッセージ</param>
	UFUNCTION()
	void OnConnectError(FString ErrorMessage);

	/// <summary>
	/// 切断成功した時
	/// </summary>
	UFUNCTION()
	void OnDisconnectSucc();
	
	/// <summary>
	/// 切断失敗した時
	/// </summary>
	/// <param name="ErrorMessage">エラーメッセージ</param>
	UFUNCTION()
	void OnDisconnectError(FString ErrorMessage);

	/// <summary>Write完了コールバック（BLEプラグインから呼ばれる）</summary>
	UFUNCTION()
	void OnWriteData(FString ServiceUUID, FString CharacteristicUUID);

	/// <summary>データ受信コールバック（BLEプラグインから呼ばれる）</summary>
	UFUNCTION()
	void OnReceiveData(FString ServiceUUID, FString CharacteristicUUID, TArray<uint8>& Data);

	//===================================================
	// Operation Queue 内部処理
	//===================================================

	void EnqueueOperation(const FBleOperation& Operation);
	void ProcessNextOperation();
	void OnOperationCompleted();

	//===================================================
	// メンバ変数
	//===================================================

	UPROPERTY()
	TScriptInterface<IBleManagerInterface> BleManager;

	UPROPERTY()
	TScriptInterface<IBleDeviceInterface> CurrentDevice;
	
	/// <summary>
	/// スキャン時のサービスフィルタ
	/// </summary>
	TArray<FString> ScanServiceFilters;

	/// <summary>
	/// 接続対象のデバイス名
	/// </summary>
	FString TargetDeviceName;

	/// <summary>
	/// 接続中のデバイス名
	/// </summary>
	FString ConnectedDeviceName;

	/// <summary>
	/// 接続中のデバイスUUID
	/// </summary>
	FString ConnectedDeviceUUID;

	/// <summary>
	/// 現在の接続状態
	/// </summary>
	EBleConnectionState ConnectionState;

	/// <summary>
	/// BLE操作のQueue本体
	/// </summary>
	TQueue<FBleOperation> OperationQueue;

	/// <summary>
	/// 現在操作実行中フラグ
	/// </summary>
	bool bIsOperationInProgress;

	/// <summary>
	/// Queueに操作が積まれているかを追跡するカウンタ（TQueueはIsEmpty以外ない
	/// </summary>
	int32 PendingOperationCount;
};
