// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SmackCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "math.h"
#include "SmackableObject.h"

DEFINE_LOG_CATEGORY_STATIC(SideScrollerCharacter, Log, All);

//////////////////////////////////////////////////////////////////////////
// ASmackCharacter

ASmackCharacter::ASmackCharacter()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	//Set the vectors for the semicircle trace in front of the character
	for (int i = 0; i <= 180; i += 10)
	{
		FVector vector = FVector(FMath::Sin(FMath::DegreesToRadians(i)) * 150, 0, FMath::Cos(FMath::DegreesToRadians(i)) * 150);
		SemicircleTraceArray.Add(vector);
	}

	//for (auto& vec : SemicircleTraceArray)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Red, FString::Printf(TEXT("Trace Array Vector: %s"), *vec.ToCompactString()));
	//}

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

    // 	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
    // 	TextComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
    // 	TextComponent->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
    // 	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    // 	TextComponent->SetupAttachment(RootComponent);

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;
}

//////////////////////////////////////////////////////////////////////////
// Animation

void ASmackCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	// Are we moving or standing still?
	UPaperFlipbook* DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	if( GetSprite()->GetFlipbook() != DesiredAnimation 	)
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void ASmackCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SmackLineTrace(DeltaSeconds);

	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("SmackableObject Hit = %s"), IsSmackableHit));

	UpdateCharacter();
}


//////////////////////////////////////////////////////////////////////////
// Input

void ASmackCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASmackCharacter::MoveRight);
	PlayerInputComponent->BindAction("Smack", IE_Pressed, this, &ASmackCharacter::Smack);
	PlayerInputComponent->BindAction("Smack", IE_Released, this, &ASmackCharacter::StopSmack);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASmackCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASmackCharacter::TouchStopped);
}

void ASmackCharacter::MoveRight(float Value)
{
	/*UpdateChar();*/

	// Apply the input to the character motion
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
}

void ASmackCharacter::Smack()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Smack")));
	IsSmacking = true;
}

void ASmackCharacter::StopSmack()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("StopSmack")));
	IsSmacking = false;
}

void ASmackCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Jump on any touch
	Jump();
}

void ASmackCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Cease jumping once touch stopped
	StopJumping();
}

//Semicircle Line Trace in front of the character
void ASmackCharacter::SmackLineTrace(float deltaSeconds)
{
	count = 0;
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams(FName(TEXT("SmackableTrace")), true, this);

	FVector Start = GetActorLocation();
	FVector Front = GetActorForwardVector();

	for (auto& i : SemicircleTraceArray)
	{
		FVector End = FVector((Start.X + (i.X * Front.X)), (Start.Y + i.Y), (Start.Z + i.Z));
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, .01, 0, 3);

		IsSmackableHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_GameTraceChannel2, CollisionParams);
		//If line trace hits a smackable object
		if (IsSmackableHit)
		{
			count++;
			//UE_LOG(LogTemp, Warning, TEXT("Smack Hit, Count = %i"), count);

			//If the player input to smack the object is being pressed
			if (IsSmacking)
			{
				ASmackableObject * SmackableObject = Cast<ASmackableObject>(OutHit.GetActor());
				if (SmackableObject && !Initialized)
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Cast To: %s"), *OutHit.GetActor()->GetName()));
					SmackableObject->AddImpulse();
					Initialized = true;
					break;
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("No Cast")));
				}
			}
		}
	}

	//Entire for loop in a single frame never hit a smackable object, so reset Initialized
	if (count == 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Full Arc, Never Hit Smackable Object During Frame"));
		Initialized = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Smackable Object During Frame"));
	}

	///Sphere Trace Using SweepBySingleChannel
	//FHitResult OutHit;

	//// start and end locations
	//FVector SweepStart = GetActorLocation();
	//FVector SweepEnd = GetActorLocation();

	//// create a collision sphere
	//FCollisionShape MyColSphere = FCollisionShape::MakeSphere(100.0f);

	//// draw collision sphere
	//DrawDebugSphere(GetWorld(), GetActorLocation(), MyColSphere.GetSphereRadius(), 25, FColor::Purple, false, .01, 2);

	//// check if something got hit in the sweep
	//bool isHit = GetWorld()->SweepSingleByChannel(OutHit, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, MyColSphere);

	//if (isHit)
	//{
	//	if (GEngine)
	//	{
	//		// screen log information on what was hit
	//		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Result: %s"), OutHit.Actor->GetName()));
	//		// uncommnet to see more info on sweeped actor
	//		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("All Hit Information: %s"), *Hit.ToString()));
	//	}
	//}
}

void ASmackCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	const FVector PlayerVelocity = GetVelocity();	
	float TravelDirection = PlayerVelocity.X;
	// Set the rotation so that the character faces his direction of travel.
	if (Controller != nullptr)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}
