// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// このインターフェースを実装するクラスは、インタラクションが可能であることを示す。
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TP_VRPROJECT_API IInteractableInterface
{
	GENERATED_BODY()

public:
	// Blueprintで実装可能な関数。プレイヤーからのインタラクション時に呼び出される。
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void OnInteract(AActor* InstigatorActor);
};
