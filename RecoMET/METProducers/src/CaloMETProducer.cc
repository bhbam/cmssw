// -*- C++ -*-
//
// Package:    METProducers
// Class:      CaloMETProducer
//
//

//____________________________________________________________________________||
#include "RecoMET/METProducers/interface/CaloMETProducer.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/METReco/interface/METFwd.h"
#include "DataFormats/METReco/interface/CaloMETFwd.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/CommonMETData.h"

#include "RecoMET/METAlgorithms/interface/METAlgo.h"
#include "RecoMET/METAlgorithms/interface/SignAlgoResolutions.h"
#include "RecoMET/METAlgorithms/interface/CaloSpecificAlgo.h"
#include "RecoMET/METAlgorithms/interface/SignCaloSpecificAlgo.h"

#include <cstring>

//____________________________________________________________________________||
namespace cms {

  //____________________________________________________________________________||
  CaloMETProducer::CaloMETProducer(const edm::ParameterSet& iConfig)
      : inputToken_(consumes<edm::View<reco::Candidate> >(iConfig.getParameter<edm::InputTag>("src"))),
        calculateSignificance_(iConfig.getParameter<bool>("calculateSignificance")),
        resolutions_(nullptr),
        globalThreshold_(iConfig.getParameter<double>("globalThreshold")) {
    noHF_ = iConfig.getParameter<bool>("noHF");

    std::string alias = iConfig.getParameter<std::string>("alias");
    produces<reco::CaloMETCollection>().setBranchAlias(alias);

    if (calculateSignificance_)
      resolutions_ = new metsig::SignAlgoResolutions(iConfig);
  }

  //____________________________________________________________________________||
  CaloMETProducer::~CaloMETProducer() {
    if (resolutions_)
      delete resolutions_;
  }

  //____________________________________________________________________________||
  void CaloMETProducer::produce(edm::Event& event, const edm::EventSetup& setup) {
    edm::Handle<edm::View<reco::Candidate> > input;
    event.getByToken(inputToken_, input);

    METAlgo algo;
    CommonMETData commonMETdata = algo.run(*input.product(), globalThreshold_);

    CaloSpecificAlgo calospecalgo;
    reco::CaloMET calomet = calospecalgo.addInfo(input, commonMETdata, noHF_, globalThreshold_);

    /*
    if( calculateSignificance_ )
      {
	SignCaloSpecificAlgo signcalospecalgo;
	signcalospecalgo.calculateBaseCaloMET(input, commonMETdata, *resolutions_, noHF_, globalThreshold_);
	calomet.SetMetSignificance(signcalospecalgo.getSignificance());
	calomet.setSignificanceMatrix(signcalospecalgo.getSignificanceMatrix());
      }
*/
    auto calometcoll = std::make_unique<reco::CaloMETCollection>();
    calometcoll->push_back(calomet);
    event.put(std::move(calometcoll));
  }

  //____________________________________________________________________________||
  void CaloMETProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("src", edm::InputTag("towerMaker"));
    desc.add<bool>("calculateSignificance", false);
    desc.add<double>("globalThreshold", 0.3);
    desc.add<bool>("noHF", false);
    desc.add<std::string>("alias", "");
    metsig::SignAlgoResolutions::fillPSetDescription(desc);
    descriptions.addWithDefaultLabel(desc);
  }

  //____________________________________________________________________________||
  DEFINE_FWK_MODULE(CaloMETProducer);
}  // namespace cms

//____________________________________________________________________________||
