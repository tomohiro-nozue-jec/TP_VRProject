// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ConditionSourceComponent.generated.h"

// スイッチのタイプを定義するEnum
// BP_EventTriggerによってGameEventDataから設定される
UENUM(BlueprintType)
enum class ESourceBehavior : uint8
{
	Toggle UMETA(DisplayName = "Toggle"),
	LatchOn UMETA(DisplayName = "Latch On"),
};

// スイッチの状態が変化したときに呼び出されるデリゲート
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConditionStateChanged, UConditionSourceComponent*, ConditionSourceComponent, bool, bIsOn);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TP_VRPROJECT_API UConditionSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UConditionSourceComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// スイッチの状態を表す変数
	UPROPERTY(BlueprintReadWrite, Category = "Switch")
	bool bIsOn = false;

	// スイッチの挙動タイプを外部から設定できるようにする
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switch")
	ESourceBehavior SourceBehavior = ESourceBehavior::Toggle;

	// スイッチの状態が変化したときに呼び出されるイベント
	UPROPERTY(BlueprintAssignable, Category = "Switch")
	FOnConditionStateChanged OnConditionStateChanged;

	// スイッチの状態を切り替える関数
	UFUNCTION(BlueprintCallable, Category = "Switch")
	void ToggleSwitch();
};
