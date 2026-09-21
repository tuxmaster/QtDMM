// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Bench multimeters QtDMM reads through sigrok-cli (protocol Sigrok, the
// ASCII decoder parses sigrok-cli's output lines, see portdevices/sigrok.cpp).
// The last field names the libsigrok driver; the settings page then builds
// the sigrok-cli driver string from it and the connection the user enters.
// The models are the ones libsigrok's scpi-dmm driver knows by *IDN?; more
// (OWON XDM, GW Instek GDM, HP 3457A, Agilent U12xx) can follow on request.
// Serial parameters are meaningless here (sigrok-cli talks to the meter),
// baud 0 marks that in the device table. Nothing verified on hardware.

#include "dmmdecoder.h"

static const bool registered = []() {
  DmmDecoder::addConfig({"Keysight", "34465A (sigrok) *", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 1000000, 0, 0, 0, "", "scpi-dmm"});
  DmmDecoder::addConfig({"Agilent", "34405A (sigrok) *", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 100000, 0, 0, 0, "", "scpi-dmm"});
  DmmDecoder::addConfig({"Agilent", "34410A (sigrok) *", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 1000000, 0, 0, 0, "", "scpi-dmm"});
  DmmDecoder::addConfig({"Agilent", "34460A (sigrok) *", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 1000000, 0, 0, 0, "", "scpi-dmm"});
  DmmDecoder::addConfig({"HP", "34401A (sigrok) *", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 1000000, 0, 0, 0, "", "scpi-dmm"});
  DmmDecoder::addConfig({"Siglent", "SDM3055 (sigrok) *", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 200000, 0, 0, 0, "", "scpi-dmm"});
  DmmDecoder::addConfig({"sigrok", "SCPI DMM (any scpi-dmm model) *", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 100000, 0, 0, 0, "", "scpi-dmm"});
  return true;
}();
