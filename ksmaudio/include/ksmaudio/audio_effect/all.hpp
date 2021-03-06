#pragma once
#include "audio_effect.hpp"

#include "dsp/retrigger_dsp.hpp"
#include "params/retrigger_params.hpp"

#include "dsp/gate_dsp.hpp"
#include "params/gate_params.hpp"

#include "dsp/flanger_dsp.hpp"
#include "params/flanger_params.hpp"

#include "dsp/bitcrusher_dsp.hpp"
#include "params/bitcrusher_params.hpp"

#include "dsp/wobble_dsp.hpp"
#include "params/wobble_params.hpp"

namespace ksmaudio
{
	using Retrigger = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::RetriggerParams, AudioEffect::RetriggerDSP, AudioEffect::RetriggerDSPParams>;

	using Gate = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::GateParams, AudioEffect::GateDSP, AudioEffect::GateDSPParams>;

	using Flanger = AudioEffect::BasicAudioEffect<AudioEffect::FlangerParams, AudioEffect::FlangerDSP, AudioEffect::FlangerDSPParams>;

	using Bitcrusher = AudioEffect::BasicAudioEffect<AudioEffect::BitcrusherParams, AudioEffect::BitcrusherDSP, AudioEffect::BitcrusherDSPParams>;

	using Wobble = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::WobbleParams, AudioEffect::WobbleDSP, AudioEffect::WobbleDSPParams>;
}
