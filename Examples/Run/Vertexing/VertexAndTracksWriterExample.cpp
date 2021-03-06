// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/Framework/Sequencer.hpp"
#include "ACTFW/Generators/FlattenEvent.hpp"
#include "ACTFW/Generators/ParticleSelector.hpp"
#include "ACTFW/Generators/Pythia8ProcessGenerator.hpp"
#include "ACTFW/Io/Csv/CsvParticleWriter.hpp"
#include "ACTFW/Io/Root/RootParticleWriter.hpp"
#include "ACTFW/Io/Root/RootVertexAndTracksWriter.hpp"
#include "ACTFW/Options/CommonOptions.hpp"
#include "ACTFW/Options/Pythia8Options.hpp"
#include "ACTFW/TruthTracking/TrackSelector.hpp"
#include "ACTFW/TruthTracking/TruthVerticesToTracks.hpp"
#include "ACTFW/Utilities/Paths.hpp"
#include <Acts/EventData/TrackParameters.hpp>

#include <memory>

#include <boost/program_options.hpp>

using namespace FW;
using namespace Acts::UnitLiterals;

/// Main vertex finder example executable
///
/// @param argc The argument count
/// @param argv The argument list
int main(int argc, char* argv[]) {
  // setup and parse options
  auto desc = Options::makeDefaultOptions();
  Options::addSequencerOptions(desc);
  Options::addRandomNumbersOptions(desc);
  Options::addPythia8Options(desc);
  Options::addOutputOptions(desc);
  auto vm = Options::parse(desc, argc, argv);
  if (vm.empty()) {
    return EXIT_FAILURE;
  }

  // basic setup
  auto logLevel = Options::readLogLevel(vm);
  auto rnd =
      std::make_shared<RandomNumbers>(Options::readRandomNumbersConfig(vm));
  Sequencer sequencer(Options::readSequencerConfig(vm));

  // Set up event generator
  EventGenerator::Config evgen = Options::readPythia8Options(vm, logLevel);
  evgen.output = "event";
  evgen.randomNumbers = rnd;
  sequencer.addReader(std::make_shared<EventGenerator>(evgen, logLevel));

  ParticleSelector::Config selectParticles;
  selectParticles.inputEvent = evgen.output;
  selectParticles.outputEvent = "event_selected";
  selectParticles.absEtaMax = 2.5;
  selectParticles.rhoMax = 4_mm;
  selectParticles.ptMin = 400_MeV;
  selectParticles.removeNeutral = true;
  sequencer.addAlgorithm(
      std::make_shared<ParticleSelector>(selectParticles, logLevel));

  TruthVerticesToTracksAlgorithm::Config trkConvConfig;
  trkConvConfig.input = selectParticles.outputEvent;
  trkConvConfig.output = "tracks";
  trkConvConfig.doSmearing = true;
  trkConvConfig.randomNumberSvc = rnd;
  trkConvConfig.bField = {0_T, 0_T, 1_T};
  sequencer.addAlgorithm(std::make_shared<TruthVerticesToTracksAlgorithm>(
      trkConvConfig, logLevel));

  // Set up track selector
  TrackSelector::Config selectorConfig;
  selectorConfig.input = trkConvConfig.output;
  selectorConfig.output = "tracks_selected";
  selectorConfig.absEtaMax = 2.5;
  selectorConfig.rhoMax = 4_mm;
  selectorConfig.ptMin = 400_MeV;
  selectorConfig.keepNeutral = false;
  sequencer.addAlgorithm(
      std::make_shared<TrackSelector>(selectorConfig, logLevel));

  auto outputDir = ensureWritableDirectory(vm["output-dir"].as<std::string>());

  RootVertexAndTracksWriter::Config writerCfg;
  writerCfg.collection = selectorConfig.output;
  writerCfg.filePath = joinPaths(outputDir, selectorConfig.output + ".root");
  sequencer.addWriter(
      std::make_shared<RootVertexAndTracksWriter>(writerCfg, logLevel));

  return sequencer.run();
}
