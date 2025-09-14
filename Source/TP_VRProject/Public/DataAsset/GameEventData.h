// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameEventData.generated.h"

// 複数のスイッチが満たすべき条件
UENUM(BlueprintType)
enum class ESwitchCondition : uint8
{
	AND UMETA(DisplayName = "AND (All Switches)"),
	OR UMETA(DisplayName = "OR (Any Switch)"),
};

/**
 * 
 */
UCLASS()
class TP_VRPROJECT_API UGameEventData : public UDataAsset
{
	GENERATED_BODY()

public :
	// イベントの対象となるアクターへのソフト参照。
// ドア、宝箱、ライト、壁など、種類を問わず設定可能。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	TArray<TSoftObjectPtr<AActor>> TargetActors;

	// イベントをトリガーするスイッチのアクターへのソフト参照の配列。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	TArray<TSoftObjectPtr<AActor>> TriggerSwitches;

	// スイッチが押されたと見なすための条件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	ESwitchCondition SwitchCondition;
};
