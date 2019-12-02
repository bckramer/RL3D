// Fill out your copyright notice in the Description page of Project Settings.

#include "NEAT_Manager.h"

using namespace std;

using namespace NEAT;

#define NUM_RUNS	100


// Sets default values
ANEAT_Manager::ANEAT_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANEAT_Manager::BeginPlay()
{
	Super::BeginPlay();

	population = Begin(10);
}

// Called every frame
void ANEAT_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (initialized) {
		//agents[0]->MakeMoves();
	}

	TArray<AActor*> FoundAgents;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AgentClass, FoundAgents);

	if (FoundAgents.Num() <= 0) {
		EndEpoch(population, generation);
		generation++;
		StartEpoch(population, generation);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("New Epoch Started."));
	}
}

TArray<AFirstPersonAgent*> ANEAT_Manager::SpawnNewAgents()
{
	float spawnHeight = 300.0f;
	TArray<AFirstPersonAgent*> agents;
	float TraceLength = 100.0f;
	float MuzzleXOffset = 100.0f;

	FActorSpawnParameters ActorSpawnParams;
	UWorld* const World = GetWorld();
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	FVector SelectedLoc = Spawn1->GetActorLocation();
	FRotator SelectedRot = FRotator(0.0f, Spawn1->GetActorRotation().Yaw, 0.0f);
	SelectedLoc.Z = SelectedLoc.Z + spawnHeight;
	AFirstPersonAgent* Agent1 = World->SpawnActor<AFirstPersonAgent>(AgentClass, SelectedLoc, SelectedRot, ActorSpawnParams);
	Agent1->FP_MuzzleLocation->SetRelativeLocation(FVector(MuzzleXOffset, 0.0f, 0.0f));
	agents.Add(Agent1);

	SelectedLoc = Spawn2->GetActorLocation();
	SelectedLoc.Z = SelectedLoc.Z + spawnHeight;
	SelectedRot = FRotator(0.0f, Spawn2->GetActorRotation().Yaw, 0.0f);
	AFirstPersonAgent* Agent2 = World->SpawnActor<AFirstPersonAgent>(AgentClass, SelectedLoc, SelectedRot, ActorSpawnParams);
	Agent2->FP_MuzzleLocation->SetRelativeLocation(FVector(MuzzleXOffset, 0.0f, 0.0f));
	agents.Add(Agent2);

	SelectedLoc = Spawn3->GetActorLocation();
	SelectedLoc.Z = SelectedLoc.Z + spawnHeight;
	SelectedRot = FRotator(0.0f, Spawn3->GetActorRotation().Yaw, 0.0f);
	AFirstPersonAgent* Agent3 = World->SpawnActor<AFirstPersonAgent>(AgentClass, SelectedLoc, SelectedRot, ActorSpawnParams);
	Agent3->FP_MuzzleLocation->SetRelativeLocation(FVector(MuzzleXOffset, 0.0f, 0.0f));
	agents.Add(Agent3);

	SelectedLoc = Spawn4->GetActorLocation();
	SelectedLoc.Z = SelectedLoc.Z + spawnHeight;
	SelectedRot = FRotator(0.0f, Spawn4->GetActorRotation().Yaw, 0.0f);
	AFirstPersonAgent* Agent4 = World->SpawnActor<AFirstPersonAgent>(AgentClass, SelectedLoc, SelectedRot, ActorSpawnParams);
	Agent4->FP_MuzzleLocation->SetRelativeLocation(FVector(MuzzleXOffset, 0.0f, 0.0f));
	agents.Add(Agent4);

	SelectedLoc = Spawn5->GetActorLocation();
	SelectedLoc.Z = SelectedLoc.Z + spawnHeight;
	SelectedRot = FRotator(0.0f, Spawn5->GetActorRotation().Yaw, 0.0f);
	AFirstPersonAgent* Agent5 = World->SpawnActor<AFirstPersonAgent>(AgentClass, SelectedLoc, SelectedRot, ActorSpawnParams);
	Agent5->FP_MuzzleLocation->SetRelativeLocation(FVector(MuzzleXOffset, 0.0f, 0.0f));
	agents.Add(Agent5);

	SelectedLoc = Spawn6->GetActorLocation();
	SelectedLoc.Z = SelectedLoc.Z + spawnHeight;
	SelectedRot = FRotator(0.0f, Spawn6->GetActorRotation().Yaw, 0.0f);
	AFirstPersonAgent* Agent6 = World->SpawnActor<AFirstPersonAgent>(AgentClass, SelectedLoc, SelectedRot, ActorSpawnParams);
	Agent6->FP_MuzzleLocation->SetRelativeLocation(FVector(MuzzleXOffset, 0.0f, 0.0f));
	agents.Add(Agent6);

	return agents;
}

bool ANEAT_Manager::Evaluate(Organism *org)
{
	Network *net;
	double out[4]; //The four outputs
	double this_out; //The current output
	int count;
	// double errorsum;

	bool success;  //Check for successful activation
	int numnodes;  /* Used to figure out how many nodes
			  should be visited during activation */

	int net_depth = 1; //The max depth of the network to be activated
	int relax; //Activates until relaxation

	double in[7][1] = { {0.0},
						{1.0},
						{0.0},
						{1.0},
						{1.0},
						{1.0},
						{1.0} };

	net = org->net;
	numnodes = ((org->gnome)->nodes).size();

	for (count = 0; count <= 6; count++) {
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

	return false;
}

int ANEAT_Manager::StartEpoch(Population *pop, int generation) 
{
	

	vector<Organism*>::iterator curorg;
	agents = SpawnNewAgents();
	
	int curAgent = 0;
	for (curorg = (population->organisms).begin(); curorg != (population->organisms).end(); ++curorg)
	{
		agents[curAgent]->org = *curorg;
		agents[curAgent]->org->fitness = 0.01;
		agents[curAgent]->MakeMoves();
		curAgent++;
	}
	initialized = true;

	int timeout = 0;
	//while (timeout < 1000) {
	//	agents[0]->MakeMoves();
	//	timeout++;
	//}

	return 0;
}

void ANEAT_Manager::EndEpoch(Population *pop, int generation) {
	vector<Species*>::iterator curspecies;
	for (curspecies = (population->species).begin(); curspecies != (population->species).end(); ++curspecies) {

		//This experiment control routine issues commands to collect ave
		//and max fitness, as opposed to having the snapshot do it, 
		//because this allows flexibility in terms of what time
		//to observe fitnesses at

		(*curspecies)->compute_average_fitness();
		(*curspecies)->compute_max_fitness();
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Computing...")));
	}
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Organisms: %lf"), (double) population->organisms[0]->fitness));
	population->epoch(generation);
}

void ANEAT_Manager::NewEpoch(Population *pop, int generation, char *filename, int &winnernum, int &winnergenes, int &winnernodes) {

}

Population* ANEAT_Manager::Begin(int gens) 
{
	population = 0;
	Genome *start_genome;
	char curword[20];
	int id;
	int gen;

	int evals[NUM_RUNS];  //Hold records for each run
	int genes[NUM_RUNS];
	int nodes[NUM_RUNS];

	//For averaging
	int totalevals = 0;
	int totalgenes = 0;
	int totalnodes = 0;
	int expcount;
//	int samples;  //For averaging

	memset(evals, 0, NUM_RUNS * sizeof(int));
	memset(genes, 0, NUM_RUNS * sizeof(int));
	memset(nodes, 0, NUM_RUNS * sizeof(int));

	cout << "START XOR TEST" << endl;

	cout << "Reading in the start genome" << endl;

	ifstream iFile("C:\\Users\\Ben\\Documents\\Unreal Projects\\RL3D\\Source\\RL3D\\start_genes", ios::in);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Starting test")));

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Reading in start genome")));
	//Read in the start Genome
	iFile >> curword;
	iFile >> id;

	cout << "Reading in Genome id " << id << endl;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Reading in Genome id")));

	start_genome = new Genome(id, iFile);
	iFile.close();

	// Content

	// Below was changed, Switch back to < NUM_RUNS
	for (expcount = 0; expcount < 1; expcount++) {
		//Spawn the Population
		cout << "Spawning Population off Genome2" << endl;

		int population_size = 6;
		population = new Population(start_genome, population_size);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Started  %d")), population->species.size());
		//cout << "Verifying Spawned Pop" << endl;
		//pop->verify();
		generation = 1;
		for (gen = 1; gen <= 1; gen++) {
			cout << "Epoch " << gen << endl;

			//This is how to make a custom filename
			//ostringstream fnamebuf = ostringstream();
			//(*fnamebuf) << "gen_" << gen << ends;  //needs end marker

#ifndef NO_SCREEN_OUT
			//cout << "name of fname: " << fnamebuf->str() << endl;
#endif

			char temp[50];
			sprintf_s(temp, "gen_%d", gen);

			//Check for success
			if (StartEpoch(population, gen)) {
				//	if (xor_epoch(pop,gen,fnamebuf->str(),winnernum,winnergenes,winnernodes)) {
				//Collect Stats on end of experiment
				//evals[expcount] = NEAT::pop_size*(gen - 1) + winnernum;
				//genes[expcount] = winnergenes;
				//nodes[expcount] = winnernodes;
				//gen = gens;

			}

		}

		//if (expcount < NUM_RUNS - 1) delete pop;

	}

	//Average and print stats
	//cout << "Nodes: " << endl;
	//for (expcount = 0; expcount < NUM_RUNS; expcount++) {
	//	cout << nodes[expcount] << endl;
	//	totalnodes += nodes[expcount];
	//}
	//
	//cout << "Genes: " << endl;
	//for (expcount = 0; expcount < NUM_RUNS; expcount++) {
	//	cout << genes[expcount] << endl;
	//	totalgenes += genes[expcount];
	//}
	//
	//cout << "Evals " << endl;
	//samples = 0;
	//for (expcount = 0; expcount < NUM_RUNS; expcount++) {
	//	cout << evals[expcount] << endl;
	//	if (evals[expcount] > 0)
	//	{
	//		totalevals += evals[expcount];
	//		samples++;
	//	}
	//}
	//
	//cout << "Failures: " << (NUM_RUNS - samples) << " out of " << NUM_RUNS << " runs" << endl;
	//cout << "Average Nodes: " << (samples > 0 ? (double)totalnodes / samples : 0) << endl;
	//cout << "Average Genes: " << (samples > 0 ? (double)totalgenes / samples : 0) << endl;
	//cout << "Average Evals: " << (samples > 0 ? (double)totalevals / samples : 0) << endl;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Started."));

	return population;
}

