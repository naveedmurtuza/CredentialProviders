#pragma once

#include <stdlib.h>
#include <string>
#include <Windows.h>
#include "IMac.h"
#include "IClock.h"
#include "IExtractPin.h"
#include "TotpGenerator.h"

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)
