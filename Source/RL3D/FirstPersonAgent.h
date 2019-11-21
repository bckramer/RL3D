// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Projectile.h"
#include "Perception/PawnSensingComponent.h"
#include "Engine/World.h"
#include "Pickup.h"
#include "Runtime/Core/Public/Misc/App.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Camera/CameraComponent.h"
#include "FirstPersonAgent.generated.h"

UCLASS()
class RL3D_API AFirstPersonAgent : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFirstPersonAgent(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UBoxComponent* CollisionComp;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		float FiringCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		AFirstPersonAgent* Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		UClass* SpawnPointClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		UClass* PickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector GunOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		APawn* SensedPawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ResetTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int TotalEliminations = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ZRespawnOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MovementSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RotationSpeed = 1.0f;

	/* State Information */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float InitialHealth = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool EnemySensed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool EnemyDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool Dead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool PawnSensed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ItemAcquired = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ItemSensed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IncreasedFireRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool TookDamage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool GaveDamage = false;



	float NormalFiringCooldown = 0.0f;
	float NormalHealth = 0.0f;
	float resetTimer = 0.0f;
	float CooldownTracker = 0.0f;

	float invincible = false;
	float invincibilityTimer = 0.0f;

	UFUNCTION(BlueprintCallable)
		void Respawn();

	/** Fires a projectile. */
	UFUNCTION(BlueprintCallable)
		void Fire(bool ShouldFire);

	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable)
		void MoveX(float Val);

	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable)
		void MoveY(float Val);

	UFUNCTION(BlueprintCallable)
		void Roll(float Val);

	UFUNCTION(BlueprintCallable)
		void OnSeePawn(APawn* Pawn);
	
	UFUNCTION(BlueprintCallable)
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
		void ResetValues();

};
