// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Component/SwitchComponent.h" // スイッチのタイプを使用するためにインクルード
#include "GameEventData.generated.h"

// 複数のスイッチが満たすべき条件
UENUM(BlueprintType)
enum class ESwitchCondition : uint8
{
	AND UMETA(DisplayName = "AND (All Switches)"),
	OR UMETA(DisplayName = "OR (Any Switch)"),
};


// ターゲットアクターの挙動を定義するEnum
UENUM(BlueprintType)
enum class ETargetType : uint8
{
	EveryTime UMETA(DisplayName = "Every Time"),
	OneShot UMETA(DisplayName = "One-Shot (Once)"),
};

// スイッチと、そのタイプを関連付ける構造体
USTRUCT(BlueprintType)
struct FEventSwitch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	TSoftObjectPtr<AActor> SwitchActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	ESwitchType SwitchType;
};

// ターゲットアクターと、そのタイプを関連付ける構造体
USTRUCT(BlueprintType)
struct FEventTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	TSoftObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	ETargetType TargetType;
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
	TArray<FEventTarget> EventTargets;

	// イベントをトリガーするスイッチの配列と、そのタイプ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	TArray<FEventSwitch> EventSwitches;

	// スイッチが押されたと見なすための条件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Event")
	ESwitchCondition SwitchCondition;

};
