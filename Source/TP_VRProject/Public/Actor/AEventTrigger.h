// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataAsset/GameEventData.h"
#include "Component/SwitchComponent.h"
#include "AEventTrigger.generated.h"

UCLASS()
class TP_VRPROJECT_API AAEventTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAEventTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// トリガーするイベントのデータアセット
	UPROPERTY(EditAnywhere, Category = "Trigger")
	UGameEventData* TriggerData;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// スイッチの状態変化にバインドする関数
	void BindToSwitchDelegates();

	// スイッチの状態が変化したときに呼び出される関数
	UFUNCTION()
	void OnSwitchStateChanged(USwitchComponent* SwitchComponent, bool bIsOn);

	// 条件を評価して、必要ならイベントをトリガーする関数
	bool EvaluateCondition();

};
