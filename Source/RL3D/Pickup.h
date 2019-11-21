// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "Pickup.generated.h"

UCLASS()
class RL3D_API APickup : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UBoxComponent* CollisionComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UClass* AgentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool HealthPickup = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool FiringPickup = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FiringRateModifier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float HealthBonus = 20.0f;

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	float destroyTimer = 0.01f;
	bool startDestroy = false;
	
	
};
