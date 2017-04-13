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

# ifndef H_SV_G4_CREATE_DESTROY_TRACKING_ACTION_H
# define H_SV_G4_CREATE_DESTROY_TRACKING_ACTION_H

//  # include "g4extras/DetectorConstruction.hpp"

//# include <Geant4/G4UserTrackingAction.hh>
# include <G4UserTrackingAction.hh>
# include <G4Track.hh>
# include <G4TrackStatus.hh>
# include <root/TTree.h>
# include <unordered_map>
# include <unordered_set>

namespace sV {

/**@brief A combined dictionary for particle selectors and statistics.
 *
 * This class is designed to maintain both the particle selector instances and
 * create/destroy statistics instances. Has to be used with
 * `CreateDestroyStats_TA` subclass of `G4UserTrackingAction` for observation
 * of created particles.
 *
 * This singleton has to be filled in the same manner of every dictionary in
 * StromaV are filled --- with __constructor__((...)) preprocessor directives
 * upon shared object dynamic loading.
 * */
class ParticleStatisticsDictionary {
public:
    /// Aux class representing particle track selector type.
    class iTrackSelector {
    private:
        const std::string _name;
    protected:
        /// Has to return True if track matches.
        virtual bool _V_matches(const G4Track*) const = 0;
    public:
        iTrackSelector( const std::string & nm ) : _name(nm) {}
        virtual ~iTrackSelector() {}
        bool matches( const G4Track*t ) const { return _V_matches(t); }
        const std::string & name() { return _name; }
    };

    typedef iTrackSelector * (*SelectorConstructor)();

    /// Aux class representing target statistics type.
    class iTrackingStatistics {
    protected:
        virtual void _V_fill_track( const iTrackSelector &, const G4Track * ) = 0;
    public:
        void fill_track( const iTrackSelector &s, const G4Track * t)
            { _V_fill_track(s, t); }
        virtual ~iTrackingStatistics() {}
    };

    typedef iTrackingStatistics * (*StatisticsConstructor)();
private:
    static ParticleStatisticsDictionary * _self;
    std::unordered_map<std::string, SelectorConstructor> _selectors;
    std::unordered_map<std::string, StatisticsConstructor> _stats;
    std::unordered_set<iTrackSelector *> _allocatedSelectors;
    std::unordered_set<iTrackingStatistics *> _allocatedStatistics;

    ParticleStatisticsDictionary() {}
public:
    static ParticleStatisticsDictionary & self();

    void add_selector_ctr( const std::string &, SelectorConstructor );
    void add_statistics_ctr( const std::string &, StatisticsConstructor );

    iTrackSelector * new_selector( const std::string & );
    void free_selector( iTrackSelector * );

    iTrackingStatistics * new_statistics( const std::string & );
    void free_statistics( iTrackingStatistics * );

    /// Checks for all selector/statistics instances to be freed upon
    /// completion and deletes self instance. If there are some unfreed
    /// objects, frees it by itself printing warning.
    static void finalize();
};  // class ParticleStatisticsDictionary



/**@brief Tracking action gathering create/destroy kinematic information.
 *
 * This tracking action instances looks up for certain particle type on
 * creation and destroy collecting momentum and position stats.
 * */
class CreateDestroyStats_TA : public G4UserTrackingAction {
public:
    /// Hooks index type (this TrackingAction entry).
    typedef std::unordered_map<
                const ParticleStatisticsDictionary::iTrackSelector *,
                ParticleStatisticsDictionary::iTrackingStatistics *>
                                                            StatsObjectIndex;
private:
    StatsObjectIndex _ssOnCreate, _ssOnDestroy;
protected:
    void _fill_if_need( StatsObjectIndex &, const G4Track * );
public:
    CreateDestroyStats_TA ();
    ~CreateDestroyStats_TA ();

    virtual void  PreUserTrackingAction(const G4Track*) override;
    virtual void PostUserTrackingAction(const G4Track*) override;

    /// Inserts selector->stats pair. Note, that selector ptr may be
    /// non-unique.
    void choose_tracks_on_create( const ParticleStatisticsDictionary::iTrackSelector *,
                                  ParticleStatisticsDictionary::iTrackingStatistics * );
    /// Inserts selector->stats pair. Note, that selector ptr may be
    /// non-unique.
    void choose_tracks_on_destroy( const ParticleStatisticsDictionary::iTrackSelector *,
                                  ParticleStatisticsDictionary::iTrackingStatistics * );
};  // class UserTrackingAction

/**@brief A rather primitive statistic tracking class gathering information
 *        about on-born particle track vertex.
 * @class ParticleBornKinematics
 *
 * Internally, relies on global ROOT file to be opened.
 *
 * TODO: [re]setting routine may be implemented in slightly more efficient way.
 * */
class ParticleBornKinematics :
                public sV::ParticleStatisticsDictionary::iTrackingStatistics {
public:
    struct ParticleVertex {
        double posX,
               posY,
               posZ;
        double momentumX,
               momentumY,
               momentumZ;
        double totalEnergy;
        double kineticEnergy;
        //  G4TrackStatus trackStatus;
    };
protected:
    ParticleVertex _vertex;
    TTree * _aprimeInfo;
protected:
    void _V_fill_track( const sV::ParticleStatisticsDictionary::iTrackSelector &,
                        const G4Track * ) override;
public:
    ParticleBornKinematics();
};  // ParticleBornKinematics

}  // namespace sV

# endif  // H_SV_G4_CREATE_DESTROY_TRACKING_ACTION_H


