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
	Lifetime -= DeltaTime;
	if (Dead || Lifetime < 0.0f) {
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

		double in[4][1] = {  { ObstacleLeft },
							 { ObstacleMiddle },
							 { ObstacleRight },
							 { EnemySensed } };
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

			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Output %d: %lf"), count, out[count]));

			net->flush();

		}

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Obstacle Left: %f"), ObstacleLeft));

		if (!initialized) {
			initialized = true;
		}
		float xMovement = out[0] - 0.5f;
		float yMovement = out[1] - 0.5f;
		float roll = out[2] - 0.5f;
		bool shouldFire = (out[3] > 0.5f) ? false : true;

		MoveX(xMovement);
		MoveY(yMovement);
		Roll(roll);
		Fire(shouldFire);
	}
	else {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Organism not set...")));
	}
}

void AFirstPersonAgent::UpdateFitness() 
{
	if (EnemySensed) {
		org->fitness += 500.0f;
	}
	if (GaveDamage) {
		org->fitness += 50000.0f;
	}
	if (EnemyDestroyed) {
		org->fitness += 100000.0f;
	}
	if (ItemAcquired) {
		org->fitness += 2000.0f;
	}
	if (ItemSensed) {
		org->fitness += 500.0f;
	}
	if (IncreasedFireRate) {
		org->fitness += 1000.0f;
	}
	//if (TookDamage) {
	//	org->fitness -= 20000.0f;
	//}
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


void AFirstPersonAgent::Fire(bool ShouldFire)
{
	if (ShouldFire) {
		if (CooldownTracker > FiringCooldown) {
			if (ProjectileClass != NULL)
			{
				UWorld* const World = GetWorld();
				if (World != NULL)
				{
					const FRotator SpawnRotation = GetActorRotation();
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					FVector tempLocation = GetActorLocation() + (GetActorForwardVector() * 100);
					//const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
					const FVector SpawnLocation = tempLocation;

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
}

// -1.0f to 1.0f
void AFirstPersonAgent::MoveX(float Val) {
	FVector ActorLocation = GetActorLocation();
	SetActorLocation(FVector(ActorLocation.X + (Val * MovementSpeed), ActorLocation.Y, ActorLocation.Z));
	org->fitness += FMath::Abs((GetActorLocation().X - ActorLocation.X) / MovementModifier);
}

// -1.0f to 1.0f
void AFirstPersonAgent::MoveY(float Val) {
	FVector ActorLocation = GetActorLocation();
	SetActorLocation(FVector(ActorLocation.X, ActorLocation.Y + (Val * MovementSpeed), ActorLocation.Z));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Health: %f"), FMath::Abs((GetActorLocation().Y - ActorLocation.Y))));
	org->fitness += FMath::Abs((GetActorLocation().Y - ActorLocation.Y) / MovementModifier);
}

// -1.0f to 1.0f
void AFirstPersonAgent::Roll(float Val) {
	FRotator ActorRotation = GetActorRotation();
	SetActorRotation(FRotator(ActorRotation.Pitch, ActorRotation.Yaw, ActorRotation.Roll + (Val * RotationSpeed)));
	org->fitness += FMath::Abs((GetActorRotation().Roll - ActorRotation.Roll));
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
		if (OtherActor->GetClass() == ProjectileClass && !invincible && OtherActor->GetOwner() != this) {
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