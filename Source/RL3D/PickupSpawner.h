// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Pickup.h"
#include "PickupSpawner.generated.h"

UCLASS()
class RL3D_API APickupSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SpawnTimer = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UClass* PickupClass;

	UFUNCTION(BlueprintCallable)
		void SpawnPickup();


	float spawnTimeTracker;


};
