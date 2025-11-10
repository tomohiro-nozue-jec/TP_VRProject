// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "KATMoverComponent.generated.h"

class KATSDKWarpper;
class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TP_VRPROJECT_API UKATMoverComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UKATMoverComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// <summary>
	/// 移動開始
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Movement")
	void StartMove();

	/// <summary>
	/// 移動停止
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Movement")
	void StopMove();

	/// <summary>
	/// KATVRのキャリブレーションを実行
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Calibration")
	void DoCalibration();

	/// <summary>
	/// デバイス数を取得
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Device")
	int32 GetDeviceCount();

	/// <summary>
	/// 最後のキャリブレーションからの経過時間を取得
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Calibration")
	float GetLastCalibratedTimeEscaped();

	/// <summary>
	/// デバイスに振動を送信（一定振幅）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Haptics")
	void VibrateConst(float Amplitude);

	/// <summary>
	/// デバイスに振動を送信（指定時間）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Haptics")
	void VibrateInSeconds(float Amplitude, float Duration);

	/// <summary>
	/// デバイスに振動を送信（1回）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Haptics")
	void VibrateOnce(float Amplitude);

	/// <summary>
	/// デバイスに振動を送信（時間と振幅指定）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Haptics")
	void VibrateFor(float Duration, float Amplitude);

	/// <summary>
	/// LEDを点灯（一定輝度）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|LED")
	void LEDConst(float Amplitude);

	/// <summary>
	/// LEDを点灯（指定時間）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|LED")
	void LEDInSeconds(float Amplitude, float Duration);

	/// <summary>
	/// LEDを点灯（1回）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|LED")
	void LEDOnce(float Amplitude);

	/// <summary>
	/// LEDを点灯（時間、周波数、振幅指定）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|LED")
	void LEDFor(float Duration, float Frequency, float Amplitude);

	/// <summary>
	/// デバイスに強制接続
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Device")
	void ForceConnectDevice();

	/// <summary>
	/// 接続されているかチェック
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR|Device")
	bool IsDeviceConnected();

private:

	/// <summary>
	/// KATVRからの入力処理
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HandleKATVRInput(float DeltaTime);

	/// <summary>
	/// 歩く処理
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HandleKATVRInputWalk(float DeltaTime);

	/// <summary>
	/// 回転処理
	/// </summary>
	void HandleKATVRRotator();

	/// <summary>
	/// キャラクターに反応する
	/// </summary>
	/// <param name="targetQuat"></param>
	/// <param name="duration"></param>
	void RotateCharacterByFQuat(FQuat targetQuat, float duration);

	/// <summary>
	/// KATVR
	/// </summary>
	KATSDKWarpper* KATDataHandler;
	
	/// <summary>
	/// カメラ
	/// </summary>
	UCameraComponent* VRCamera;

	/// <summary>
	/// 前回の回転角度
	/// </summary>
	FRotator PreRotator;

	/// <summary>
	/// 今回の回転角度
	/// </summary>
	FRotator CurrentRotator;

	/// <summary>
	/// 移動速度
	/// </summary>
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float NowSpeed = 4.0f;

	/// <summary>
	/// 移動できるか
	/// </summary>
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool CanMove = true;
};
