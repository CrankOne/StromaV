/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//# pragma GCC diagnostic push
//# pragma GCC diagnostic ignored "-Wdeprecated-register"  // "register" is depr-d
# include <G4RunManager.hh>
# include <G4LogicalVolumeStore.hh>
# include <G4TransportationManager.hh>
# include <G4GDMLParser.hh>
//# pragma GCC diagnostic pop

# include <TFile.h>

# include <goo_exception.hpp>

# include "app_g4simulation.hpp"
//# include "actions/TrackingAction.hpp"  // deprecated?
# include "g4extras/CreateDestroyStats_TA.hpp"
# include "actions/EventAction.hpp"
# include "g4extras/PhysList.hpp"
# include "g4extras/eHandler.hpp"
# include "g4extras/PGA.hpp"
# include "ext.gdml/dicts/sensDets.hpp"

# ifdef RPC_PROTOCOLS
// ... TODO: bucket dispatchers
# endif  // RPC_PROTOCOLS

# ifdef G4_MDL_VIS
# include <G4VisManager.hh>
# include <G4VisExecutive.hh>
# endif

# ifdef G4_MDL_GUI
#   include <G4UImanager.hh>
#   include <G4UIExecutive.hh>
# endif

# ifdef AFNA64_DPhMC_FOUND
#   include "evGen/parameters.h"
# endif

//# include "g4extras/aprimeHooks.hpp"

namespace svmc {

Application::Application( Config * cfg) :
                                AbstractApplication(cfg),
                                Geant4Application(cfg),
                                RootApplication(cfg)
                                # ifdef RPC_PROTOCOLS
                                , PBEventApp(cfg)
                                # endif  // StromaV_RPC_PROTOCOL
                                {}

Application::~Application( ) {
    //_fileRef.close();
}

void
Application::_V_concrete_app_append_common_cfg() /*{{{*/ {
    # if 0
    {
        sV::po::options_description g4CDTA;
        g4CDTA.add_options()
            ("g4.trackingActions.createStatistics",
                sV::po::value<std::vector<std::string> >(),
                "Array of comma-delimeted pairs: "
                    "<selector-name,statistics-name>.")
            ("g4.trackingActions.destroyStatistics",
                sV::po::value<std::vector<std::string> >(),
                "Array of comma-delimeted pairs: "
                    "<selector-name,statistics-name>.")
                // TODO: listing option
            ;
        res.push_back( g4CDTA );
    }
    # endif
} /* }}} */

void
Application::_initialize_tracking_action() {
    # if 0
    // TODO: what have we do in case of multiple tracking actions?
    if( co().count("g4.trackingActions.createStatistics")
     || co().count("g4.trackingActions.destroyStatistics") ) {
        // FIXME: less code duplication
        auto cdsTAPtr = new sV::CreateDestroyStats_TA();
        G4RunManager::GetRunManager()->SetUserAction( cdsTAPtr );

        {
            const auto & v = co()["g4.trackingActions.createStatistics"]
                                                .as<std::vector<std::string> >();
            for( auto sp : v ) {
                auto dPos = sp.find(':');
                auto p = std::make_pair(
                        sV::ParticleStatisticsDictionary::self().new_selector( sp.substr(0, dPos) ),
                        sV::ParticleStatisticsDictionary::self().new_statistics( sp.substr(dPos+1) )
                    );
                cdsTAPtr->choose_tracks_on_create( p.first, p.second );
                sV_log3( "Added on-create selector:statistics "
                         "pair %s -> %p:%p.\n", sp.c_str(), p.first, p.second );
            }
        }

        {
            const auto & v = co()["g4.trackingActions.destroyStatistics"]
                                                .as<std::vector<std::string> >();
            for( auto sp : v ) {
                auto dPos = sp.find(':');
                auto p = std::make_pair(
                        sV::ParticleStatisticsDictionary::self().new_selector( sp.substr(0, dPos) ),
                        sV::ParticleStatisticsDictionary::self().new_statistics( sp.substr(dPos+1) )
                    );
                cdsTAPtr->choose_tracks_on_destroy( p.first, p.second );
                sV_log3( "Added on-destroy selector:statistics "
                         "pair %s -> %p:%p.\n", sp.c_str(), p.first, p.second );
            }
        }

        sV_log2("CreateDestroyStats_TA has been registered.\n");
    }
    # endif
}

void
Application::_initialize_event_action() {
    # if 0
    //  TODO
    //  Further to be implemented using dedicated dictionary class.
    //  Bucket's destructor will be called from EventAction destructor.
    # ifdef RPC_PROTOCOLS
    _fileRef.open(goo::app<sV::AbstractApplication>().cfg_option<std::string>
        ("b-dispatcher.outFile"), std::ios::out | std::ios::binary |
                                  std::ios::app );
    # if 0
    sV::PlainStreamBucketDispatcher* bucketDispatcher =
        new sV::PlainStreamBucketDispatcher(
            _fileRef,
            (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
            ("b-dispatcher.maxBucketSize.KB"),
            (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
            ("b-dispatcher.maxBucketSize.events")
        );
    # endif
    sV::DummyCompressor * compressor = new sV::DummyCompressor;
    sV::ComprBucketDispatcher * bucketDispatcher =
        new sV::ComprBucketDispatcher(
                compressor,
                _fileRef,
                (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
                ("b-dispatcher.maxBucketSize.KB"),
                (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
                ("b-dispatcher.maxBucketSize.events"),
                (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
                ("b-dispatcher.BufSize.KB")
            );
    G4RunManager::GetRunManager()
                    ->SetUserAction(new EventAction( bucketDispatcher));
    sV_log2("User EventAction has been initialized\n");
    # endif  // RPC_PROTOCOLS
    # endif
}
void
Application::_V_concrete_app_configure() /*{{{*/ {
    // ...
} /*}}}*/

void
Application::_build_up_run() {
    Parent::_build_up_run();
    //G4RunManager::GetRunManager()->Initialize();
    // ^^^ xxx? Called after _build_up_run(), by _run_session()
    _initialize_tracking_action();
    _initialize_event_action();
}

int
Application::_V_run() /*{{{*/ {
    if( do_immediate_exit() ) return EXIT_SUCCESS;
    return _run_session();
}  /*}}}*/

} // namespace svmc

