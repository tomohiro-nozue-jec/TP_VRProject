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
	void StartMove();

	/// <summary>
	/// 移動停止
	/// </summary>
	void StopMove();

	/// <summary>
	/// KATVRのキャリブレーションを実行
	/// </summary>


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
