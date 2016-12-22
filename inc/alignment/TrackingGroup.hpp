/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_STROMA_V_ALIGNMENT_TRACKING_VOLUMES_GROUP_H
# define H_STROMA_V_ALIGNMENT_TRACKING_VOLUMES_GROUP_H

# include "TrackingVolume.tcc"

# ifdef ALIGNMENT_ROUTINES

# include "Placement.hpp"

# include <boost/property_tree/ptree_fwd.hpp>

namespace sV {
namespace alignment {

/**@class TrackingGroup
 * @brief Auxilliary template class providing interface for track points
 * positions calculation.
 *
 * Aggregates a container of receptive volumes, positioned
 * and rotated accordingly to particular detectors layout. The goal is to
 * provide unified interface for further track lines building. This class also
 * incapsulates 2D -> 3D conversion for TrackReceptiveVolume<2> instances
 * providing ready sets of points in 3D space for track reconstruction.
 *
 * This class doesn't own the transformation instances, neither the volumes
 * itself (meaning that their lifetime must be tracked outside).
 *
 * The responsibility of this class is only to only keep all the geometrical
 * information which is necessary for further geometrical operations in
 * detector alignment / track reconstruction.
 */
class TrackingGroup {
public:
    typedef AbstractTrackReceptiveVolume Volume;
    typedef DetectorPlacement Placement;
protected:
    /// Placements already contains reference to detector, but
    /// due to inheritance hierarchy one need to dynamic_cast<>
    /// them. This map caches those casts.
    std::unordered_map<const Placement *, const Volume *> _pl2vol;
    /// Returns particular placement 
    std::unordered_map<const Volume *, const Placement *> _vol2pl;
    /// Stores order of insertion.
    std::list<const Volume *> _volumes;

    const std::string _name;
public:
    TrackingGroup( const std::string & name_ );
    virtual ~TrackingGroup() {}

    /// Returns reference to track receptive volume instance (const).
    const Volume & volume( const Placement * pl ) const;

    /// Returns placement instance referenced by particular volume.
    const Placement & placement( const Volume * vol ) const;

    /// Inserts a tracking volume into the group.
    virtual void include( const Placement & );

    /// Returns tracking group name.
    const char * name() const { return _name.c_str(); }

    /// Sets persistency (dispatches persistency change among all the
    /// tracking volumes).
    virtual void persistency( size_t n );

    /// Returns columes container.
    const DECLTYPE(_volumes) & volumes() const { return _volumes; }
};  // class TrackingGroup

/**@brief Constructors dictionary for tracking groups.
 */
class TrackingGroupFactory {
public:
    typedef boost::property_tree::basic_ptree<std::string,std::string> PTree;
    typedef TrackingGroup * (*GroupConstructor)( const std::string &, const PTree & );
private:
    static std::unordered_map<std::string, GroupConstructor> * _grpCtrs;
    //static std::unordered_map<std::string, TrackingGroup *> * _createdGroups;  //XXX: use alignment::app
public:
    /// Factory method; registers new track reconstruction algoritm in
    /// ctrs dict.
    static void register_constructor( const std::string &, GroupConstructor );
    /// Multi-constructor, factory. Clean-up should be performed
    /// with delete_drawable_tracking_group().
    static TrackingGroup * new_drawable_tracking_group(
            const std::string & name,
            const PTree & parameters );
    /// Deletes particular tracking group previously constructed
    /// with delete_drawable_tracking_group().
    static void delete_drawable_tracking_group( TrackingGroup * );

    //static std::unordered_map<std::string, TrackingGroup *> & groups();  //XXX: use alignment::app
};  // class TrackingGroupFactory

}  // namespace alignment
}  // namespace sV

# define StromaV_REGISTER_TRACKING_ALGORITHM( strID, name )                  \
static ::sV::alignment::TrackingGroup *                                 \
__construct_track_reconstruction_algorithm_ ## name                         \
( const std::string & trGName,                                              \
  const ::sV::alignment::TrackingGroupFactory::PTree & pt ) {           \
    return new name ( trGName, pt ); }                                      \
static void _ctr_register_track_reconstruction_ ## name () __attribute__(( __constructor__(156) )); \
static void _ctr_register_track_reconstruction_ ## name () {                \
    ::sV::alignment::TrackingGroupFactory::register_constructor(        \
                strID, __construct_track_reconstruction_algorithm_ ## name ); }

# endif  // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_ALIGNMENT_TRACKING_VOLUMES_GROUP_H

