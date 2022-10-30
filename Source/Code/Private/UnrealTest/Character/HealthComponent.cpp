// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealTest/Character/HealthComponent.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	MaxHealth = 100.f;
	ResetCurrentHealth();
}

void UHealthComponent::ResetCurrentHealth()
{
	CurrentHealth = MaxHealth;
}

void UHealthComponent::ApplyDamage(float damage)
{
	if (CanReceiveDamage() && damage > 0.f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - damage, 0.f, MaxHealth);
		OnRep_CurrentHealth();

		if(GetOwnerRole() < ENetRole::ROLE_SimulatedProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s took %f damage!"), *GetOwner()->GetName(), damage);
		}

		if (OnDamagedEvent.IsBound()) 
		{
			OnDamagedEvent.Broadcast();
		}

		if (OnHealthChangedEvent.IsBound()) 
		{
			OnHealthChangedEvent.Broadcast(CurrentHealth, MaxHealth);
		}

		if (CurrentHealth <= 0.f)
		{			
			Destroy();
		}
	}
}

bool UHealthComponent::CanReceiveDamage()
{
	return CurrentHealth > 0.f;
}

void UHealthComponent::Destroy()
{
	UE_LOG(LogTemp, Warning, TEXT("%s was killed!"), *GetOwner()->GetName());
	if (OnHealthEmptyEvent.IsBound())
	{
		OnHealthEmptyEvent.Broadcast();
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

void UHealthComponent::OnHealthUpdated()
{
	AActor* owner = GetOwner();
	if (owner != nullptr) 
	{
		if (owner->HasAuthority())
		{
			FString messageStr = FString::Printf(TEXT("[Server][%s] Health: %f"), *owner->GetName(), CurrentHealth);
			GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Blue, messageStr);
		}
		else
		{
			FString messageStr = FString::Printf(TEXT("[Client][Self] Health: %f"), CurrentHealth);
			GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Yellow, messageStr);
		}
	}	
}

void UHealthComponent::OnRep_CurrentHealth()
{
	OnHealthUpdated();
}

