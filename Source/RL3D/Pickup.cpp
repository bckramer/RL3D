// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickup.h"


// Sets default values
APickup::APickup()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = CollisionComp;
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	CollisionComp->OnComponentHit.RemoveDynamic(this, &APickup::OnHit);
	CollisionComp->OnComponentHit.AddDynamic(this, &APickup::OnHit);
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (startDestroy) {
		destroyTimer -= DeltaTime;
		if (destroyTimer <= 0.0f) {
			Destroy();
		}
	}
}

void APickup::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AgentClass == NULL) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("ERROR: AgentClass not set!")));
	}
	else if (OtherActor->GetClass() == AgentClass && (OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		startDestroy = true;
	}
}
