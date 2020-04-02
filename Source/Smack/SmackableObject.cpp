// Fill out your copyright notice in the Description page of Project Settings.


#include "SmackableObject.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ASmackableObject::ASmackableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("<Mesh>"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Game/2DSideScroller/Meshes/Shape_Cylinder.Shape_Cylinder'"));
	UStaticMesh* Asset = MeshAsset.Object;
	MeshComponent->SetStaticMesh(Asset);

	//SmackablePhysicsMaterial = CreateDefaultSubobject<UPhysicsMaterial>(TEXT("<SmackablePhysicsMaterial>"));

	FVector spawnLocation = FVector(0, -25, 500);
	FRotator spawnRotation = FRotator(0, 0, 90);
	FVector scale = FVector(0.5, 0.5, .5);

	MeshComponent->SetWorldLocationAndRotation(spawnLocation, spawnRotation);
	MeshComponent->SetRelativeScale3D(scale);
	MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetConstraintMode(EDOFMode::XZPlane);
	MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	MeshComponent->SetCollisionObjectType(ECC_WorldStatic);

	/*Overlap = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CAPSULE"));
	Overlap->InitCapsuleSize(25, 25);
	Overlap->SetCollisionProfileName(TEXT("Overlap Trigger"));
	Overlap->SetupAttachment(RootComponent);*/
	//Overlap->OnComponentBeginOverlap.AddDynamic(this, &ASmackableObject::BeginOverlap);
	//Overlap->OnComponentBeginOverlap.AddDynamic(this, &ASmackableObject::EndOverlap);

}

// Called when the game starts or when spawned
void ASmackableObject::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASmackableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UKismetSystemLibrary::DrawDebugCapsule(GetWorld(), GetActorLocation(), 25, 25, FRotator(0, 0, 0), FLinearColor::Red, 0.25f, 3.f);
}

