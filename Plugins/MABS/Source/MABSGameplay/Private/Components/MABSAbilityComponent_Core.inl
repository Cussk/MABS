void UMABSAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMABSAbilityComponent, GrantedAbilities);
	DOREPLIFETIME(UMABSAbilityComponent, CooldownGroupStates);
	DOREPLIFETIME_CONDITION(UMABSAbilityComponent, ActivePeriodicEffects, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UMABSAbilityComponent, bReplicateDebugDataToOwningClient, COND_OwnerOnly);
}


void UMABSAbilityComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(UMABSAbilityComponent, ActivePeriodicEffects, bReplicateDebugDataToOwningClient);
}


TArray<FMABSAbilitySpec> UMABSAbilityComponent::GetGrantedAbilities() const
{
	return GrantedAbilities;
}


bool UMABSAbilityComponent::FindGrantedAbilitySpecByTag(const FGameplayTag AbilityTag, FMABSAbilitySpec& OutAbilitySpec) const
{
	if (const FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByTagInternal(AbilityTag))
	{
		OutAbilitySpec = *AbilitySpec;
		return true;
	}

	return false;
}


float UMABSAbilityComponent::GetCooldownRemainingByTag(const FGameplayTag AbilityTag) const
{
	const FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByTagInternal(AbilityTag);
	return AbilitySpec != nullptr ? GetCooldownRemainingForAbilitySpec(*AbilitySpec) : 0.0f;
}


bool UMABSAbilityComponent::IsAbilityOnCooldown(const FGameplayTag AbilityTag) const
{
	return GetCooldownRemainingByTag(AbilityTag) > 0.0f;
}


float UMABSAbilityComponent::GetCooldownGroupRemaining(const FGameplayTag CooldownGroupTag) const
{
	return GetCooldownGroupRemainingInternal(CooldownGroupTag);
}


bool UMABSAbilityComponent::IsCooldownGroupActive(const FGameplayTag CooldownGroupTag) const
{
	return GetCooldownGroupRemainingInternal(CooldownGroupTag) > 0.0f;
}


void UMABSAbilityComponent::SetDebugReplicationEnabled(const bool bEnabled)
{
	if (!CanMutateAbilityState())
	{
		return;
	}

	bReplicateDebugDataToOwningClient = bEnabled;
}


bool UMABSAbilityComponent::IsDebugReplicationEnabled() const
{
	return bReplicateDebugDataToOwningClient;
}


FMABSAbilitySpec* UMABSAbilityComponent::FindGrantedAbilitySpecByTagMutable(const FGameplayTag& AbilityTag)
{
	return GrantedAbilities.FindByPredicate(
		[&AbilityTag](const FMABSAbilitySpec& AbilitySpec)
		{
			return AbilitySpec.AbilityTag == AbilityTag;
		});
}


const FMABSAbilitySpec* UMABSAbilityComponent::FindGrantedAbilitySpecByTagInternal(const FGameplayTag& AbilityTag) const
{
	return GrantedAbilities.FindByPredicate(
		[&AbilityTag](const FMABSAbilitySpec& AbilitySpec)
		{
			return AbilitySpec.AbilityTag == AbilityTag;
		});
}


FMABSAbilitySpec* UMABSAbilityComponent::FindGrantedAbilitySpecByHandle(const FMABSAbilityHandle& AbilityHandle)
{
	return GrantedAbilities.FindByPredicate(
		[&AbilityHandle](const FMABSAbilitySpec& AbilitySpec)
		{
			return AbilitySpec.Handle == AbilityHandle;
		});
}


FMABSCooldownGroupState* UMABSAbilityComponent::FindCooldownGroupStateMutable(const FGameplayTag& CooldownGroupTag)
{
	return CooldownGroupStates.FindByPredicate(
		[&CooldownGroupTag](const FMABSCooldownGroupState& CooldownGroupState)
		{
			return CooldownGroupState.CooldownGroupTag == CooldownGroupTag;
		});
}


const FMABSCooldownGroupState* UMABSAbilityComponent::FindCooldownGroupStateInternal(const FGameplayTag& CooldownGroupTag) const
{
	return CooldownGroupStates.FindByPredicate(
		[&CooldownGroupTag](const FMABSCooldownGroupState& CooldownGroupState)
		{
			return CooldownGroupState.CooldownGroupTag == CooldownGroupTag;
		});
}


bool UMABSAbilityComponent::CanMutateAbilityState() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr && Owner->HasAuthority();
}


FMABSAbilityHandle UMABSAbilityComponent::MakeNextAbilityHandle()
{
	return FMABSAbilityHandle(NextAbilityHandleValue++);
}


int32 UMABSAbilityComponent::MakeNextPeriodicEffectRuntimeId()
{
	return NextPeriodicEffectRuntimeIdValue++;
}
