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

	UPROPERTY(EditAnywhere, Category = "Trigger")
	UGameEventData* TriggerData;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void BindToSwitchDelegates();
	UFUNCTION()
	void OnSwitchStateChanged(USwitchComponent* SwitchComponent, bool bIsOn);

	// ƒpƒYƒ‹‚ÌğŒ‚ğ•]‰¿‚·‚éŠÖ”
	bool EvaluateCondition();

};
