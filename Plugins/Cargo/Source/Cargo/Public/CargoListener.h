// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "cargo-io/public/CancellationToken.h"
#include "CoreMinimal.h"

class FCargoListener : public FRunnable
{
public:
	FCargoListener();
	virtual ~FCargoListener() override;

	virtual auto Run() -> uint32 override;
	virtual auto Stop() -> void override;

private:
	FRunnableThread* ListenerThread = nullptr;
	CancellationTokenPtr ServerToken = std::make_shared<CancellationToken>();
};
