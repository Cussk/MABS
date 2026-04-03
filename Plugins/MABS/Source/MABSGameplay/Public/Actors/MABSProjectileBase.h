// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSProjectileBase.generated.h"

class UMABSAbilityComponent;
class UMABSAbilityDefinition;
class UProjectileMovementComponent;
class USphereComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(Blueprintable)
class MABSGAMEPLAY_API AMABSProjectileBase : public AActor
{
	GENERATED_BODY()

public:

	AMABSProjectileBase();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeProjectile(
		UMABSAbilityComponent* InOwningAbilityComponent,
		AActor* InSourceActor,
		UMABSAbilityDefinition* InSourceAbilityDefinition,
		FGameplayTag InSourceAbilityTag,
		FMABSAbilityHandle InSourceAbilityHandle);

	UFUNCTION(BlueprintPure, Category="MABS|Projectile")
	USphereComponent* GetCollisionComponent() const;

	UFUNCTION(BlueprintPure, Category="MABS|Projectile")
	UProjectileMovementComponent* GetProjectileMovementComponent() const;

	UFUNCTION(BlueprintPure, Category="MABS|Projectile")
	AActor* GetSourceActor() const;

	UFUNCTION(BlueprintPure, Category="MABS|Projectile")
	UMABSAbilityDefinition* GetSourceAbilityDefinition() const;

	UFUNCTION(BlueprintPure, Category="MABS|Projectile")
	FGameplayTag GetSourceAbilityTag() const;

	UFUNCTION(BlueprintPure, Category="MABS|Projectile")
	FMABSAbilityHandle GetSourceAbilityHandle() const;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MABS|Projectile")
	TObjectPtr<USphereComponent> CollisionComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MABS|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MABS|Projectile", meta=(ClampMin="0.0"))
	float CollisionRadius = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MABS|Projectile")
	bool bDestroyOnRejectedImpact = true;

private:

	UFUNCTION()
	void OnRep_SourceAbilityDefinition();

	UFUNCTION()
	void HandleCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void HandleCollisionOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void HandleImpact(AActor* OtherActor, const FHitResult& HitResult);

	UMABSAbilityComponent* ResolveOwningAbilityComponent() const;

	void ActivateTravelPresentation();

	UPROPERTY(Transient, Replicated)
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_SourceAbilityDefinition)
	TObjectPtr<UMABSAbilityDefinition> SourceAbilityDefinition = nullptr;

	TWeakObjectPtr<UMABSAbilityComponent> OwningAbilityComponent;

	UPROPERTY(Transient, Replicated)
	FGameplayTag SourceAbilityTag;

	UPROPERTY(Transient, Replicated)
	FMABSAbilityHandle SourceAbilityHandle;

	bool bImpactHandled = false;

	bool bTravelPresentationActivated = false;
};
