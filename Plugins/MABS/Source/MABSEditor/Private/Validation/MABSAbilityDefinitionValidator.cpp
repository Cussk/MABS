#include "Validation/MABSAbilityDefinitionValidator.h"

#include "Actors/MABSProjectileBase.h"
#include "Data/MABSAbilityDefinition.h"
#include "Delivery/MABSDeliveryHandler.h"
#include "Delivery/MABSDirectDeliveryHandler.h"
#include "Delivery/MABSHitTraceDeliveryHandler.h"
#include "Delivery/MABSMeleeDeliveryHandler.h"
#include "Delivery/MABSProjectileDeliveryHandler.h"

#define LOCTEXT_NAMESPACE "MABSAbilityDefinitionValidator"

namespace
{
	FText GetAbilityAssetLabelText(const UMABSAbilityDefinition& Ability)
	{
		return FText::FromString(Ability.GetName());
	}

	FText GetDeliveryModeText(const EMABSDeliveryMode DeliveryMode)
	{
		return StaticEnum<EMABSDeliveryMode>()->GetDisplayNameTextByValue(static_cast<int64>(DeliveryMode));
	}

	FText GetTargetTypeText(const EMABSTargetType TargetType)
	{
		return StaticEnum<EMABSTargetType>()->GetDisplayNameTextByValue(static_cast<int64>(TargetType));
	}

	FText GetInstantEffectTypeText(const EMABSInstantEffectType EffectType)
	{
		return StaticEnum<EMABSInstantEffectType>()->GetDisplayNameTextByValue(static_cast<int64>(EffectType));
	}

	bool HasAuthoredGameplayEffect(const UMABSAbilityDefinition& Ability)
	{
		return Ability.InstantEffectType != EMABSInstantEffectType::None || Ability.PeriodicEffect.bEnabled;
	}

	void ValidateGameplayEffectAuthoring(UEditorValidatorBase& Validator, const UMABSAbilityDefinition& Ability)
	{
		const FText AbilityLabel = GetAbilityAssetLabelText(Ability);
		if (!Ability.AbilityTag.IsValid())
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("MissingAbilityTag", "Ability '{0}' requires a valid AbilityTag."),
					AbilityLabel));
		}

		if (!HasAuthoredGameplayEffect(Ability))
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("MissingGameplayEffect", "Ability '{0}' must author an instant or periodic gameplay effect."),
					AbilityLabel));
		}

		if (Ability.InstantEffectType != EMABSInstantEffectType::None && Ability.EffectMagnitude <= 0.0f)
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("InvalidInstantMagnitude", "Ability '{0}' uses instant effect '{1}' but EffectMagnitude must be greater than zero."),
					AbilityLabel,
					GetInstantEffectTypeText(Ability.InstantEffectType)));
		}

		if (Ability.PeriodicEffect.bEnabled)
		{
			if (Ability.PeriodicEffect.Duration <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidPeriodicDuration", "Ability '{0}' enables PeriodicEffect but Duration must be greater than zero."),
						AbilityLabel));
			}

			if (Ability.PeriodicEffect.TickInterval <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidPeriodicTickInterval", "Ability '{0}' enables PeriodicEffect but TickInterval must be greater than zero."),
						AbilityLabel));
			}

			if (Ability.PeriodicEffect.TickMagnitude <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidPeriodicTickMagnitude", "Ability '{0}' enables PeriodicEffect but TickMagnitude must be greater than zero."),
						AbilityLabel));
			}
		}

		if (Ability.AoE.bEnabled)
		{
			switch (Ability.AoE.Shape)
			{
			case EMABSAoEShape::Sphere:
				if (Ability.AoE.Radius <= 0.0f)
				{
					Validator.AssetFails(
						&Ability,
						FText::Format(
							LOCTEXT("InvalidSphereAoE", "Ability '{0}' enables sphere AoE but Radius must be greater than zero."),
							AbilityLabel));
				}
				break;

			case EMABSAoEShape::Box:
				if (Ability.AoE.BoxExtent.GetMax() <= 0.0f)
				{
					Validator.AssetFails(
						&Ability,
						FText::Format(
							LOCTEXT("InvalidBoxAoE", "Ability '{0}' enables box AoE but BoxExtent must have at least one positive component."),
							AbilityLabel));
				}
				break;

			case EMABSAoEShape::Capsule:
				if (Ability.AoE.CapsuleRadius <= 0.0f)
				{
					Validator.AssetFails(
						&Ability,
						FText::Format(
							LOCTEXT("InvalidCapsuleRadiusAoE", "Ability '{0}' enables capsule AoE but CapsuleRadius must be greater than zero."),
							AbilityLabel));
				}

				if (Ability.AoE.CapsuleHalfHeight <= 0.0f)
				{
					Validator.AssetFails(
						&Ability,
						FText::Format(
							LOCTEXT("InvalidCapsuleHalfHeightAoE", "Ability '{0}' enables capsule AoE but CapsuleHalfHeight must be greater than zero."),
							AbilityLabel));
				}
				break;
			}
		}

		if (Ability.Combo.NextComboAbilityTag.IsValid())
		{
			if (Ability.DeliveryMode != EMABSDeliveryMode::Melee)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("ComboRequiresMelee", "Ability '{0}' authors a combo follow-up but combo support requires Melee delivery."),
						AbilityLabel));
			}

			if (!Ability.Combo.IsEnabled())
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidComboWindow", "Ability '{0}' authors combo follow-up '{1}' but ComboWindowEnd must be greater than ComboWindowStart."),
						AbilityLabel,
						FText::FromString(Ability.Combo.NextComboAbilityTag.ToString())));
			}
		}

		if (Ability.TargetType == EMABSTargetType::None)
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("MissingTargetType", "Ability '{0}' requires a supported TargetType."),
					AbilityLabel));
		}

		if (Ability.TargetType == EMABSTargetType::Location)
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("LocationTargetUnsupported", "Ability '{0}' uses TargetType 'Location', but location targeting is not supported in MABS v1."),
					AbilityLabel));
		}
	}

	void ValidateDeliveryModeAuthoring(UEditorValidatorBase& Validator, const UMABSAbilityDefinition& Ability)
	{
		const FText AbilityLabel = GetAbilityAssetLabelText(Ability);
		switch (Ability.DeliveryMode)
		{
		case EMABSDeliveryMode::Direct:
			if (Ability.TargetType != EMABSTargetType::Self && Ability.TargetType != EMABSTargetType::Actor)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidDirectTargetType", "Ability '{0}' uses Direct delivery but TargetType '{1}' is not supported. Use Self or Actor."),
						AbilityLabel,
						GetTargetTypeText(Ability.TargetType)));
			}

			if (Ability.TargetType == EMABSTargetType::Actor && Ability.TargetTraceDistance <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidDirectTraceDistance", "Ability '{0}' uses Direct actor targeting but TargetTraceDistance must be greater than zero."),
						AbilityLabel));
			}

			if (Ability.TargetType == EMABSTargetType::Actor
				&& Ability.ActorTargetTraceMode == EMABSTargetTraceMode::Sphere
				&& Ability.TargetTraceRadius <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidDirectTraceRadius", "Ability '{0}' uses Direct sphere targeting but TargetTraceRadius must be greater than zero."),
						AbilityLabel));
			}
			break;

		case EMABSDeliveryMode::HitTrace:
			if (Ability.TargetType != EMABSTargetType::Actor)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidHitTraceTargetType", "Ability '{0}' uses HitTrace delivery but TargetType '{1}' is not supported. Use Actor."),
						AbilityLabel,
						GetTargetTypeText(Ability.TargetType)));
			}

			if (Ability.HitTraceDistance <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidHitTraceDistance", "Ability '{0}' uses HitTrace delivery but HitTraceDistance must be greater than zero."),
						AbilityLabel));
			}
			break;

		case EMABSDeliveryMode::Melee:
			if (Ability.TargetType != EMABSTargetType::Actor)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidMeleeTargetType", "Ability '{0}' uses Melee delivery but TargetType '{1}' is not supported. Use Actor."),
						AbilityLabel,
						GetTargetTypeText(Ability.TargetType)));
			}

			if (Ability.MeleeRange <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidMeleeRange", "Ability '{0}' uses Melee delivery but MeleeRange must be greater than zero."),
						AbilityLabel));
			}

			if (Ability.MeleeRadius <= 0.0f)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidMeleeRadius", "Ability '{0}' uses Melee delivery but MeleeRadius must be greater than zero."),
						AbilityLabel));
			}
			break;

		case EMABSDeliveryMode::Projectile:
			if (Ability.TargetType != EMABSTargetType::Actor)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidProjectileTargetType", "Ability '{0}' uses Projectile delivery but TargetType '{1}' is not supported. Use Actor."),
						AbilityLabel,
						GetTargetTypeText(Ability.TargetType)));
			}

			if (Ability.ProjectileActorClass == nullptr)
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("MissingProjectileActorClass", "Ability '{0}' uses Projectile delivery but ProjectileActorClass is unset."),
						AbilityLabel));
				break;
			}

			if (!Ability.ProjectileActorClass->IsChildOf(AMABSProjectileBase::StaticClass()))
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("InvalidProjectileActorClass", "Ability '{0}' uses Projectile delivery but ProjectileActorClass '{1}' must derive from AMABSProjectileBase."),
						AbilityLabel,
						FText::FromString(GetNameSafe(Ability.ProjectileActorClass.Get()))));
			}

			if (Ability.ProjectileActorClass->HasAnyClassFlags(CLASS_Abstract))
			{
				Validator.AssetFails(
					&Ability,
					FText::Format(
						LOCTEXT("AbstractProjectileActorClass", "Ability '{0}' uses Projectile delivery but ProjectileActorClass '{1}' is abstract."),
						AbilityLabel,
						FText::FromString(GetNameSafe(Ability.ProjectileActorClass.Get()))));
			}
			break;
		}
	}

	UClass* ResolveAuthoredDeliveryHandlerClass(const UMABSAbilityDefinition& Ability)
	{
		if (Ability.DeliveryHandlerClass.IsNull())
		{
			return nullptr;
		}

		if (UClass* const ResolvedClass = Ability.DeliveryHandlerClass.ResolveClass())
		{
			return ResolvedClass;
		}

		return Ability.DeliveryHandlerClass.TryLoadClass<UObject>();
	}

	void ValidateDeliveryHandlerCompatibility(
		UEditorValidatorBase& Validator,
		const UMABSAbilityDefinition& Ability,
		UClass& HandlerClass)
	{
		const FText AbilityLabel = GetAbilityAssetLabelText(Ability);
		struct FDeliveryHandlerBaseRule
		{
			EMABSDeliveryMode DeliveryMode;
			UClass* HandlerBaseClass;
			const TCHAR* HandlerBaseName;
		};

		const FDeliveryHandlerBaseRule HandlerRules[] =
		{
			{EMABSDeliveryMode::Direct, UMABSDirectDeliveryHandler::StaticClass(), TEXT("UMABSDirectDeliveryHandler")},
			{EMABSDeliveryMode::HitTrace, UMABSHitTraceDeliveryHandler::StaticClass(), TEXT("UMABSHitTraceDeliveryHandler")},
			{EMABSDeliveryMode::Melee, UMABSMeleeDeliveryHandler::StaticClass(), TEXT("UMABSMeleeDeliveryHandler")},
			{EMABSDeliveryMode::Projectile, UMABSProjectileDeliveryHandler::StaticClass(), TEXT("UMABSProjectileDeliveryHandler")}
		};

		const FDeliveryHandlerBaseRule* ExpectedRule = nullptr;
		for (const FDeliveryHandlerBaseRule& HandlerRule : HandlerRules)
		{
			if (HandlerRule.DeliveryMode == Ability.DeliveryMode)
			{
				ExpectedRule = &HandlerRule;
				break;
			}
		}

		for (const FDeliveryHandlerBaseRule& HandlerRule : HandlerRules)
		{
			if (!HandlerClass.IsChildOf(HandlerRule.HandlerBaseClass))
			{
				continue;
			}

			if (ExpectedRule != nullptr && HandlerRule.DeliveryMode == ExpectedRule->DeliveryMode)
			{
				return;
			}

			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("IncompatibleDeliveryHandler", "Ability '{0}' uses {1} delivery but DeliveryHandlerClass '{2}' derives from {3}."),
					AbilityLabel,
					GetDeliveryModeText(Ability.DeliveryMode),
					FText::FromString(GetNameSafe(&HandlerClass)),
					FText::FromString(HandlerRule.HandlerBaseName)));
			return;
		}
	}

	void ValidateDeliveryHandlerAuthoring(UEditorValidatorBase& Validator, const UMABSAbilityDefinition& Ability)
	{
		if (Ability.DeliveryHandlerClass.IsNull())
		{
			return;
		}

		const FText AbilityLabel = GetAbilityAssetLabelText(Ability);
		UClass* const HandlerClass = ResolveAuthoredDeliveryHandlerClass(Ability);
		if (HandlerClass == nullptr)
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("UnresolvedDeliveryHandlerClass", "Ability '{0}' sets DeliveryHandlerClass '{1}', but that class could not be loaded."),
					AbilityLabel,
					FText::FromString(Ability.DeliveryHandlerClass.ToString())));
			return;
		}

		if (!HandlerClass->IsChildOf(UMABSDeliveryHandler::StaticClass()))
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("InvalidDeliveryHandlerBase", "Ability '{0}' sets DeliveryHandlerClass '{1}', but it does not derive from UMABSDeliveryHandler."),
					AbilityLabel,
					FText::FromString(GetNameSafe(HandlerClass))));
			return;
		}

		if (HandlerClass->HasAnyClassFlags(CLASS_Abstract))
		{
			Validator.AssetFails(
				&Ability,
				FText::Format(
					LOCTEXT("AbstractDeliveryHandlerClass", "Ability '{0}' sets DeliveryHandlerClass '{1}', but that class is abstract."),
					AbilityLabel,
					FText::FromString(GetNameSafe(HandlerClass))));
			return;
		}

		ValidateDeliveryHandlerCompatibility(Validator, Ability, *HandlerClass);
	}
}

UMABSAbilityDefinitionValidator::UMABSAbilityDefinitionValidator()
{
	bIsEnabled = true;
}

bool UMABSAbilityDefinitionValidator::CanValidateAsset_Implementation(
	const FAssetData& InAssetData,
	UObject* InAsset,
	FDataValidationContext& InContext) const
{
	return Cast<UMABSAbilityDefinition>(InAsset) != nullptr;
}

EDataValidationResult UMABSAbilityDefinitionValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData,
	UObject* InAsset,
	FDataValidationContext& Context)
{
	UMABSAbilityDefinition* const Ability = Cast<UMABSAbilityDefinition>(InAsset);
	if (Ability == nullptr)
	{
		return EDataValidationResult::NotValidated;
	}

	ValidateGameplayEffectAuthoring(*this, *Ability);
	ValidateDeliveryHandlerAuthoring(*this, *Ability);
	ValidateDeliveryModeAuthoring(*this, *Ability);

	if (GetValidationResult() != EDataValidationResult::Invalid)
	{
		AssetPasses(Ability);
	}

	return GetValidationResult();
}

#undef LOCTEXT_NAMESPACE
