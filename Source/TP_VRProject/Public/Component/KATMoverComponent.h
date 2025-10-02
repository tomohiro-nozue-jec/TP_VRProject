#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "KATMoverComponent.generated.h"

class UCameraComponent;
class KATHub; // KATSDKWarpperの代わりにKATHubを使用

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TP_VRPROJECT_API UKATMoverComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UKATMoverComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// <summary>
	/// 移動開始
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR")
	void StartMove();

	/// <summary>
	/// 移動停止
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR")
	void StopMove();

	/// <summary>
	/// KATVRのキャリブレーションを実行
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "KATVR")
	void Calibrate();


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
	/// KATVR
	/// </summary>
	KATHub* KATDataHandler; // 型をKATHub*に変更

	/// <summary>
	/// カメラ
	/// </summary>
	UPROPERTY()
	TObjectPtr<UCameraComponent> VRCamera;

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