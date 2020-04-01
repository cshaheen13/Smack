// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SmackGameMode.h"
#include "SmackCharacter.h"
#include "SmackableObject.h"
#include "Engine/Engine.h"

ASmackGameMode::ASmackGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = ASmackCharacter::StaticClass();	
}

void ASmackGameMode::BeginPlay()
{
	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, TEXT("Actor Spawning"));

	FTransform SpawnLocation;
	GetWorld()->SpawnActor<ASmackableObject>(ASmackableObject::StaticClass(), SpawnLocation);
}
