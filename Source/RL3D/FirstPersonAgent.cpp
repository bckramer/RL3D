// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonAgent.h"


// Sets default values
AFirstPersonAgent::AFirstPersonAgent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = CollisionComp;
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(CollisionComp);
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;


	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);
}

// Called when the game starts or when spawned
void AFirstPersonAgent::BeginPlay()
{
	NormalFiringCooldown = FiringCooldown;
	NormalHealth = InitialHealth;
	Super::BeginPlay();
	if (PawnSensingComp) {
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AFirstPersonAgent::OnSeePawn);
		//PawnSensingComp->OnHearNoise.AddDynamic(this, &ABasicAutonomousShip::OnHearNoise);
	}
	CollisionComp->OnComponentHit.RemoveDynamic(this, &AFirstPersonAgent::OnHit);
	CollisionComp->OnComponentHit.AddDynamic(this, &AFirstPersonAgent::OnHit);
	
}

void AFirstPersonAgent::OnSeePawn(APawn* Pawn)
{
	if (Pawn->GetClass() == PickupClass) {
		ItemSensed = true;
	}
	else {
		SensedPawn = Pawn;
	}

}

// Called every frame
void AFirstPersonAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CooldownTracker += DeltaTime;
	if (invincibilityTimer > 0.0f) {
		invincibilityTimer -= DeltaTime;
	}
	else {
		invincible = false;
	}
	if (resetTimer > ResetTime) {
		ResetValues();
	}
	resetTimer = resetTimer + DeltaTime;
	if (SensedPawn != NULL) {
		EnemySensed = true;
	}
	/* Give Output */

	/* Output Given */


	if (Dead) {
		Respawn();
	}

}

void AFirstPersonAgent::ResetValues() {
	TookDamage = false;
	GaveDamage = false;
	EnemyDestroyed = false;

	SensedPawn = NULL;
	EnemySensed = false;
	ItemSensed = false;
	resetTimer = 0.0f;



}

void AFirstPersonAgent::Fire(bool Shouldfire)
{
	if (CooldownTracker > FiringCooldown) {
		if (ProjectileClass != NULL)
		{
			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
		
				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		
				//spawn the projectile at the muzzle
				World->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
			CooldownTracker = 0.0f;
		}

	}
}

void AFirstPersonAgent::MoveX(float Val) {
	FVector ActorLocation = GetActorLocation();
	SetActorLocation(FVector(ActorLocation.X + (Val * MovementSpeed), ActorLocation.Y, ActorLocation.Z));
}

void AFirstPersonAgent::MoveY(float Val) {
	FVector ActorLocation = GetActorLocation();
	SetActorLocation(FVector(ActorLocation.X, ActorLocation.Y + (Val * MovementSpeed), ActorLocation.Z));
}

void AFirstPersonAgent::Roll(float Val) {
	FRotator ActorRotation = GetActorRotation();
	SetActorRotation(FRotator(ActorRotation.Pitch, ActorRotation.Yaw, ActorRotation.Roll + (Val * RotationSpeed)));
}

void AFirstPersonAgent::Respawn() {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Respawning")));
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnPointClass, SpawnPoints);
	int length = SpawnPoints.Num();
	if (length > 0) {
		int randIndex = FMath::RandRange(0, length - 1);
		AActor* SelectedSpawnPoint = SpawnPoints[randIndex];
		FVector SelectedLoc = SelectedSpawnPoint->GetActorLocation();
		SetActorLocation(FVector(SelectedLoc.X, SelectedLoc.Y, SelectedLoc.Z + ZRespawnOffset));
	}
	Dead = false;
	InitialHealth = NormalHealth;
	FiringCooldown = NormalFiringCooldown;

}

void AFirstPersonAgent::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		if (OtherActor->GetClass() == ProjectileClass && !invincible) {
			InitialHealth -= 10.0f;
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Health: %f"), InitialHealth));
			if (InitialHealth <= 0.0f) {
				Dead = true;
				if (Enemy != NULL) {
					Enemy->EnemyDestroyed = true;
					Enemy->TotalEliminations += 1;
				}
				else {
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("ERROR: Enemy not set!")));
				}
			}
			else {
				if (Enemy != NULL) {
					Enemy->GaveDamage = true;
					invincible = true;
					invincibilityTimer = 0.1f;
				}
				else {
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("ERROR: Enemy not set!")));
				}
			}
			TookDamage = true;
		}
		else if (OtherActor->GetClass() == PickupClass) {
			APickup* Pickup = Cast<APickup>(OtherActor);
			if (Pickup->HealthPickup) {
				InitialHealth = InitialHealth + Pickup->HealthBonus;
			}
			if (Pickup->FiringPickup) {
				FiringCooldown = FiringCooldown / Pickup->FiringRateModifier;
				IncreasedFireRate = true;
			}
			ItemAcquired = true;
		}
	}
}

