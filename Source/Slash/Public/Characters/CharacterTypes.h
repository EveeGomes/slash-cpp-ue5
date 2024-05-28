#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two-Handed Weapon")
};

enum class EActionState : uint8
{
	// The character might be occupied doing any number of things (attacking or interacting with an object etc). These are states that would prevent the character from being able to engage in other actions, like attacking.
	EAS_Uoccupied UMETA(DisplayName = "Uoccupied"),
	EAS_Attacking UMETA(DisplayName = "Attacking")
};