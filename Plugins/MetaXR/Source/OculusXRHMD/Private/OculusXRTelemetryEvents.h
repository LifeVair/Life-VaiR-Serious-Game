/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "OculusXRTelemetry.h"

namespace OculusXRTelemetry::Events
{
	using FEditorConsent = TMarker<191965622>;
	constexpr const char* ConsentOriginKey = "Origin";
} // namespace OculusXRTelemetry::Events
