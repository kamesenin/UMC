/***
UMC
Created by Tomasz 'kamesenin' Witczak - kamesenin@gmail.com
**/
#pragma once
#include "ModuleManager.h"

class IUMC : public IModuleInterface 
{
public:
	static inline IUMC& Get() 
	{
		return FModuleManager::LoadModuleChecked< IUMC >("UMC");
	}

	static inline bool IsAvailable() 
	{
		return FModuleManager::Get().IsModuleLoaded("UMC");
	}
};