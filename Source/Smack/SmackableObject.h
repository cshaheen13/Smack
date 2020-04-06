// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SmackableObject.generated.h"

UCLASS()
class SMACK_API ASmackableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASmackableObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* Overlap;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	class UPhysicsConstraintComponent* PhysicsConstraint;

	void AddImpulse(float xDirection);

	/*UPROPERTY(VisibleAnywhere)
	class UPhysicsMaterial* SmackablePhysicsMaterial;*/


	/*UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);*/

};
