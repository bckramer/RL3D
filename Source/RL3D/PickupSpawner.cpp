// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupSpawner.h"


// Sets default values
APickupSpawner::APickupSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APickupSpawner::BeginPlay()
{
	Super::BeginPlay();
	spawnTimeTracker = 0.0f;
}

// Called every frame
void APickupSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	spawnTimeTracker -= DeltaTime;
	if (spawnTimeTracker <= 0.0f) {
		SpawnPickup();
		spawnTimeTracker = SpawnTimer;
	}
}

void APickupSpawner::SpawnPickup() 
{
	if (PickupClass == NULL) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("ERROR: Pickup class not set!")));
	}
	else {
		TArray<AActor*> FoundPickups;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), PickupClass, FoundPickups);
		bool ShouldSpawnPickup = true;
		for (int i = 0; i < FoundPickups.Num(); i++) {
			FVector PickupLocation = FoundPickups[i]->GetActorLocation();
			if (FMath::IsNearlyEqual(PickupLocation.X, GetActorLocation().X, 5.0f) && FMath::IsNearlyEqual(PickupLocation.Y, GetActorLocation().Y, 5.0f)) {
				ShouldSpawnPickup = false;
				break;
			}
		}
		if (ShouldSpawnPickup) {
			UWorld* const World = GetWorld();
			APickup* NewPickup;
			FVector SpawnLocation = GetActorLocation();
			SpawnLocation.Z = SpawnLocation.Z + 10.0f;
			NewPickup = World->SpawnActor<APickup>(PickupClass, SpawnLocation, FRotator::ZeroRotator);
			if (FMath::RandRange(0, 1) == 0) {
				NewPickup->HealthPickup = true;
			}
			else {
				NewPickup->FiringPickup = true;
			}
 		}
	}
}

