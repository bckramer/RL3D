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
	ObstacleCheck();
	UpdateFitness();
	/* Give Output */

	/* Output Given */
	if (initialized) {
		MakeMoves();
	}

	if (Dead) {
		Destroy();
	}

}

void AFirstPersonAgent::MakeMoves() 
{
	if (org != NULL) {
		NEAT::Network *net;
		double out[4]; //The four outputs
		double this_out; //The current output
		int count;
		// double errorsum;

		bool success;  //Check for successful activation
		int numnodes;  /* Used to figure out how many nodes
				  should be visited during activation */

		int net_depth = 1; //The max depth of the network to be activated
		int relax; //Activates until relaxation

		double in[4][3] = { { InitialHealth, EnemySensed, EnemyDestroyed   },
							 { Dead, ItemAcquired, ItemSensed		        },
							 { IncreasedFireRate, TookDamage, GaveDamage    },
							 { ObstacleLeft, ObstacleMiddle, ObstacleRight	} };
		net = org->net;
		numnodes = ((org->gnome)->nodes).size();

		for (count = 0; count <= 3; count++) {
			net->load_sensors(in[count]);

			//Relax net and get output
			success = net->activate();

			//use depth to ensure relaxation
			for (relax = 0; relax <= net_depth; relax++) {
				success = net->activate();
				this_out = (*(net->outputs.begin()))->activation;
			}



			out[count] = (*(net->outputs.begin()))->activation;

			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Output %d: %lf"), count, out[count]));

			net->flush();

		}

		if (!initialized) {
			initialized = true;
		}

		MoveX(out[0]);
		MoveY(out[1]);
		Roll(out[2]);
		Fire(true);
	}
	else {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Organism not set...")));
	}
}

void AFirstPersonAgent::UpdateFitness() 
{
	if (EnemySensed) {
		Fitness += 10.0f;
	}
	if (GaveDamage) {
		Fitness += 20.0f;
	}
	if (EnemyDestroyed) {
		Fitness += 100.0f;
	}
	if (ItemAcquired) {
		Fitness += 20.0f;
	}
	if (ItemSensed) {
		Fitness += 5.0f;
	}
	if (IncreasedFireRate) {
		Fitness += 10.0f;
	}
	if (TookDamage) {
		Fitness -= 20.0f;
	}
	float healthBonus = InitialHealth - NormalHealth;
	Fitness += healthBonus;


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


// 0 or 1
void AFirstPersonAgent::Fire(bool Shouldfire)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Fire called...")));
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
				AProjectile* projectile = World->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
				if (projectile != NULL) {
					projectile->SetOwner(this);
				}
			}
			CooldownTracker = 0.0f;
		}

	}
}

// -1.0f to 1.0f
void AFirstPersonAgent::MoveX(float Val) {
	FVector ActorLocation = GetActorLocation();
	SetActorLocation(FVector(ActorLocation.X + (Val * MovementSpeed), ActorLocation.Y, ActorLocation.Z));
	Fitness += (Val / 10.0f);
}

// -1.0f to 1.0f
void AFirstPersonAgent::MoveY(float Val) {
	FVector ActorLocation = GetActorLocation();
	SetActorLocation(FVector(ActorLocation.X, ActorLocation.Y + (Val * MovementSpeed), ActorLocation.Z));
	Fitness += (Val / 10.0f);
}

// -1.0f to 1.0f
void AFirstPersonAgent::Roll(float Val) {
	FRotator ActorRotation = GetActorRotation();
	SetActorRotation(FRotator(ActorRotation.Pitch, ActorRotation.Yaw, ActorRotation.Roll + (Val * RotationSpeed)));
	Fitness += (Val / 10.0f);
}

void AFirstPersonAgent::Respawn() {
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Respawning")));
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
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Health: %f"), InitialHealth));
			if (InitialHealth <= 0.0f) {
				if (OtherActor->GetOwner()->GetClass() == GetClass()) {
					Dead = true;
					AFirstPersonAgent* CastedActor = (AFirstPersonAgent*)OtherActor;
					CastedActor->GaveDamage = true;
					CastedActor->EnemyDestroyed = true;
					CastedActor->TotalEliminations += 1;
				}
			}
			else {
				//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, OtherActor->GetOwner()->GetClass());
				if (OtherActor->GetOwner()->GetClass() == GetClass()) {
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Is true!"));
					//Enemy->GaveDamage = true;
					AFirstPersonAgent* CastedActor = (AFirstPersonAgent*) OtherActor;
					CastedActor->GaveDamage = true;
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

void AFirstPersonAgent::ObstacleCheck() {
	FVector Location;
	FRotator Rotation;
	FHitResult Hit;

	FVector Start = GetActorLocation();

	// Check Left
	FVector End = End = Start + ((GetActorForwardVector() - GetActorRightVector()) * TraceLength);
	FCollisionQueryParams TraceParams;
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
	ObstacleLeft = Hit.Distance;
	if (Hit.bBlockingHit) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Distance: %f"), Hit.Distance));
	}
	DrawDebugLine(GetWorld(), Start, Hit.TraceEnd, FColor::Orange, false, 0.1f);

	// Check Middle
	End = Start + (GetActorForwardVector() * TraceLength * 2.0f);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
	ObstacleMiddle = Hit.Distance;
	DrawDebugLine(GetWorld(), Start, Hit.TraceEnd, FColor::Orange, false, 0.1f);

	// Check Right
	End = Start + ((GetActorForwardVector() + GetActorRightVector()) * TraceLength);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
	ObstacleRight = Hit.Distance;
	DrawDebugLine(GetWorld(), Start, Hit.TraceEnd, FColor::Orange, false, 0.1f);
}