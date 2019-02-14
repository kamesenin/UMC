/***
UMC
Created by Tomasz 'kamesenin' Witczak - kamesenin@gmail.com
**/
#pragma once
#include "IUMC.h"
DECLARE_LOG_CATEGORY_EXTERN( UMCLog, Log, All );

class FUMC : public IUMC
{
public:
	virtual void			StartupModule() override {};
	virtual void			ShutdownModule() override {};
};