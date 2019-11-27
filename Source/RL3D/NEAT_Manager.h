// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoint.h"
#include "FirstPersonAgent.h"
#include "Engine/World.h"

/* Global Headers */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <cstring>
#include <cstdlib>

/* NEAT headers*/
#include "NEAT.1.2.1/neat.h"
#include "NEAT.1.2.1/network.h"
#include "NEAT.1.2.1/population.h"
#include "NEAT.1.2.1/organism.h"
#include "NEAT.1.2.1/genome.h"
#include "NEAT.1.2.1/species.h"
#include "NEAT_Manager.generated.h"



UCLASS()
class RL3D_API ANEAT_Manager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANEAT_Manager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	NEAT::Population* Begin(int gens);

	int Epoch(NEAT::Population *pop, int generation, char *filename, int &winnernum, int &winnergenes, int &winnernodes);

	bool Evaluate(NEAT::Organism *org);

	bool initialized = false;

	UFUNCTION(BlueprintCallable)
		TArray<AFirstPersonAgent*> SpawnNewAgents();

	TArray<AFirstPersonAgent*> agents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UClass* AgentClass;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ASpawnPoint* Spawn1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ASpawnPoint* Spawn2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ASpawnPoint* Spawn3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ASpawnPoint* Spawn4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ASpawnPoint* Spawn5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ASpawnPoint* Spawn6;
};
