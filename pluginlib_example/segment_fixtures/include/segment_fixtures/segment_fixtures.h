/*
 * segment_fixtures.h
 *
 *  Created on: Nov 2, 2012
 *      Author: kidson
 */

#ifndef SEGMENT_FIXTURES_H_
#define SEGMENT_FIXTURES_H_

#include <segmentation_interface/segmentation.h>

namespace segment_fixtures
{
  class SegmentFixtures : public segmentation_interface::Segmentation
  {
    public:
      SegmentFixtures ();
      virtual ~SegmentFixtures ();

      void segment (std::vector<PointCloudConstPtr> segmented_clouds);
  };
}
#endif /* SEGMENT_FIXTURES_H_ */
