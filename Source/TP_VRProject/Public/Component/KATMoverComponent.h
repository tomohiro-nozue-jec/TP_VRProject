// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "KATMoverComponent.generated.h"

// 前方宣言
class KATSDKWarpper;
class UCameraComponent;
struct KATTreadMillMemoryData;

/// <summary>
/// KATVRデバイスからの入力を処理し、キャラクターの移動と回転を制御するコンポーネント
/// </summary>
/// <remarks>
/// 主な機能:
/// - KATVRトレッドミルからの移動入力の処理
/// - キャラクターの回転制御
/// - デバイスのキャリブレーション
/// - ハプティックフィードバック（振動）
/// - LED制御
/// </remarks>
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TP_VRPROJECT_API UKATMoverComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UKATMoverComponent();
	
	virtual ~UKATMoverComponent();

protected:
	virtual void BeginPlay() override;
	
	/// <summary>
	/// ゲーム終了時に呼ばれる - デバイスのシャットダウン処理
	/// </summary>
	/// <param name="EndPlayReason">終了理由</param>
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	/// <summary>
	/// 毎フレーム呼ばれる - KATVRからの入力を処理
	/// </summary>
	/// <param name="DeltaTime">前フレームからの経過時間</param>
	/// <param name="TickType">Tickのタイプ</param>
	/// <param name="ThisTickFunction">このTickの実行情報</param>
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//=============================================================================
	// ユーティリティ
	//=============================================================================
	
	/// <summary>
	/// ワールド内の最初のKATMoverComponentを取得
	/// </summary>
	/// <param name="WorldContextObject">ワールドコンテキスト</param>
	/// <returns>見つかったKATMoverComponent、見つからない場合はnullptr</returns>
	UFUNCTION(BlueprintPure, Category = "KATVR|Utility", meta = (WorldContext = "WorldContextObject"))
	static UKATMoverComponent* FindKATMoverComponent(const UObject* WorldContextObject);
	
	/// <summary>
	/// 指定したアクターからKATMoverComponentを取得
	/// </summary>
	/// <param name="Actor">検索対象のアクター</param>
	/// <returns>見つかったKATMoverComponent、見つからない場合はnullptr</returns>
	UFUNCTION(BlueprintPure, Category = "KATVR|Utility")
	static UKATMoverComponent* GetKATMoverComponentFromActor(AActor* Actor);

	//=============================================================================
	// 移動制御
	//=============================================================================
	
	/// <summary>
	/// 移動を有効化する
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Movement")
	void StartMove();

	/// <summary>
	/// 移動を無効化する
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Movement")
	void StopMove();

	/// <summary>
	/// 現在の移動速度を取得（m/s）
	/// </summary>
	/// <returns>現在の移動速度</returns>
	UFUNCTION(BlueprintPure, Category = "KATVR|Movement")
	float GetCurrentSpeed() const { return CurrentSpeed; }

	/// <summary>
	/// 移動中かどうかを判定
	/// </summary>
	/// <returns>移動中の場合true</returns>
	UFUNCTION(BlueprintPure, Category = "KATVR|Movement")
	bool IsMoving() const { return CurrentSpeed > 0.01f; }

	//=============================================================================
	// キャリブレーション
	//=============================================================================
	
	/// <summary>
	/// KATVRデバイスのキャリブレーションを実行
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Calibration")
	void DoCalibration();

	/// <summary>
	/// 最後のキャリブレーションからの経過時間を取得
	/// </summary>
	/// <returns>経過時間（秒単位）。デバイスが初期化されていない場合は-1.0</returns>
	float GetLastCalibratedTimeEscaped();

	//=============================================================================
	// デバイス情報
	//=============================================================================
	
	/// <summary>
	/// 接続されているKATVRデバイスの数を取得
	/// </summary>
	/// <returns>デバイス数。デバイスが初期化されていない場合は0</returns>
	int32 GetDeviceCount();

	/// <summary>
	/// KATVRデバイスに強制接続を試みる
	/// </summary>
	void ForceConnectDevice();

	/// <summary>
	/// KATVRデバイスが接続されているか確認
	/// </summary>
	/// <returns>接続中の場合true、未接続の場合false</returns>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Device")
	bool IsDeviceConnected();

	//=============================================================================
	// ハプティックフィードバック（振動）
	//=============================================================================
	
	/// <summary>
	/// 一定の振幅で連続振動を開始
	/// </summary>
	/// <param name="Amplitude">振動の強さ（0.0～1.0）。0で停止</param>
	void VibrateConst(float Amplitude);

	/// <summary>
	/// 指定時間だけ振動
	/// </summary>
	/// <param name="Amplitude">振動の強さ（0.0～1.0）</param>
	/// <param name="Duration">振動時間（秒）</param>
	void VibrateInSeconds(float Amplitude, float Duration);

	/// <summary>
	/// 1回だけ短く振動
	/// </summary>
	/// <param name="Amplitude">振動の強さ（0.0～1.0）</param>
	void VibrateOnce(float Amplitude);

	/// <summary>
	/// 指定した時間と振幅で振動
	/// </summary>
	/// <param name="Duration">振動時間（秒）</param>
	/// <param name="Amplitude">振動の強さ（0.0～1.0）</param>
	void VibrateFor(float Duration, float Amplitude);

	//=============================================================================
	// LED制御
	//=============================================================================
	
	/// <summary>
	/// 一定の輝度でLEDを点灯し続ける
	/// </summary>
	/// <param name="Amplitude">輝度（0.0～1.0）。0で消灯</param>
	void LEDConst(float Amplitude);

	/// <summary>
	/// 指定時間だけLEDを点灯
	/// </summary>
	/// <param name="Amplitude">輝度（0.0～1.0）</param>
	/// <param name="Duration">点灯時間（秒）</param>
	void LEDInSeconds(float Amplitude, float Duration);

	/// <summary>
	/// LEDを1回だけ短く点灯
	/// </summary>
	/// <param name="Amplitude">輝度（0.0～1.0）</param>
	void LEDOnce(float Amplitude);

	/// <summary>
	/// 指定したパターンでLEDを点滅
	/// </summary>
	/// <param name="Duration">点滅時間（秒）</param>
	/// <param name="Frequency">点滅周波数（Hz）</param>
	/// <param name="Amplitude">輝度（0.0～1.0）</param>
	void LEDFor(float Duration, float Frequency, float Amplitude);

private:
	//=============================================================================
	// 内部処理関数
	//=============================================================================
	
	/// <summary>
	/// KATVRデバイスを初期化
	/// </summary>
	/// <returns>初期化に成功した場合true</returns>
	bool InitializeDevice();

	/// <summary>
	/// KATVRデバイスをシャットダウン
	/// </summary>
	void ShutdownDevice();

	/// <summary>
	/// KATVRからの全入力を処理するメイン関数
	/// </summary>
	/// <param name="DeltaTime">前フレームからの経過時間</param>
	void HandleKATVRInput(float DeltaTime);

	/// <summary>
	/// KATVRからの移動入力を処理
	/// </summary>
	/// <param name="DeltaTime">前フレームからの経過時間</param>
	/// <param name="data">KATVRから取得したデータ</param>
	void HandleKATVRInputWalk(float DeltaTime, const KATTreadMillMemoryData& data);

	/// <summary>
	/// KATVRからの回転入力を処理
	/// </summary>
	/// <param name="data">KATVRから取得したデータ</param>
	void HandleKATVRRotator(const KATTreadMillMemoryData& data);

	/// <summary>
	/// 指定したクォータニオンに基づいてキャラクターを回転
	/// </summary>
	/// <param name="targetQuat">目標回転（クォータニオン）</param>
	/// <param name="duration">未使用（将来の補間機能用に予約）</param>
	void RotateCharacterByFQuat(FQuat targetQuat, float duration);

	//=============================================================================
	// メンバー変数
	//=============================================================================
	
	/// <summary>
	/// KATVRデバイスとの通信を管理するクラス
	/// </summary>
	KATSDKWarpper* KATDataHandler;
	
	/// <summary>
	/// VRカメラコンポーネントへの参照
	/// </summary>
	UPROPERTY()
	TObjectPtr<UCameraComponent> VRCamera;

	/// <summary>
	/// 前フレームの回転角度
	/// </summary>
	FRotator PreRotator;

	/// <summary>
	/// 現在フレームの回転角度
	/// </summary>
	FRotator CurrentRotator;

	/// <summary>
	/// 現在の移動速度（m/s）
	/// </summary>
	float CurrentSpeed;

	//=============================================================================
	// 調整可能なパラメータ（Blueprintから編集可能）
	//=============================================================================
	
	/// <summary>
	/// 移動速度の倍率（m/s）
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1000.0"))
	float MoveSpeed = 5.0f;

	/// <summary>
	/// 移動できるか
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings", meta = (AllowPrivateAccess = "true"))
	bool CanMove = true;

	/// <summary>
	/// デバッグメッセージを画面に表示するか
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowDebugMessages = false;
};
