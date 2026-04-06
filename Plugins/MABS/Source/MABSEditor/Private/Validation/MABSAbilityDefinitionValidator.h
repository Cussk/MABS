#pragma once

#include "EditorValidatorBase.h"
#include "MABSAbilityDefinitionValidator.generated.h"

UCLASS()
class UMABSAbilityDefinitionValidator final : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	UMABSAbilityDefinitionValidator();

protected:
	virtual bool CanValidateAsset_Implementation(
		const FAssetData& InAssetData,
		UObject* InAsset,
		FDataValidationContext& InContext) const override;

	virtual EDataValidationResult ValidateLoadedAsset_Implementation(
		const FAssetData& InAssetData,
		UObject* InAsset,
		FDataValidationContext& Context) override;
};
