// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataAsset/GameEventData.h"
#include "EventTrigger.generated.h"

UCLASS()
class TP_VRPROJECT_API AEventTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEventTrigger();

protected:
	virtual void BeginPlay() override;

protected:

	// トリガーするイベントのデータアセット
	UPROPERTY(EditAnywhere, Category = "Trigger")
	UGameEventData* TriggerData;

private:
	// 各ターゲットアクターの起動状態を管理するマップ
	UPROPERTY()
	TMap<AActor*, bool> TargetActivationStates;

	// スイッチの状態変化にバインドする関数
	void BindToSwitchDelegates();

	// スイッチの状態が変化したときに呼び出される関数
	UFUNCTION()
	void OnSwitchStateChanged(USwitchComponent* SwitchComponent, bool bIsOn);

	// 条件を評価して、必要ならイベントをトリガーする関数
	bool EvaluateCondition();

};
