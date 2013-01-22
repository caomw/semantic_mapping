/*
 * visualization.cpp
 *
 *  Created on: Nov 10, 2012
 *      Author: kidson
 */

#include "visualizer/visualization.h"

// ros
#include <sensor_msgs/PointCloud2.h>
#include <ros/ros.h>
#include <ros/console.h>

//pcl
#include <pcl17/point_types.h>
#include <pcl17/ros/conversions.h>
#include <pcl17/filters/voxel_grid.h>
#include <pcl17/filters/extract_indices.h>
//vtk
#include <boost/thread/thread.hpp>
#include <pcl17/common/common_headers.h>
#include <pcl17/features/normal_3d.h>
#include <pcl17/visualization/pcl_visualizer.h>
#include <pcl17/console/parse.h>

// opencv -> ROS -> opencv
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

// opencv
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

static boost::shared_ptr<pcl17::visualization::PCLVisualizer> viewer_;

Visualization::Visualization ():
  vox_grid_size_(0.0)
{
  if(!viewer_)    // instantiate viewer
  {
    viewer_.reset(new pcl17::visualization::PCLVisualizer ("3D Viewer"));
    viewer_->addCoordinateSystem (1.0);
    viewer_->initCameraParameters ();
    viewer_->setBackgroundColor (0, 0, 0);
  }
}

Visualization::~Visualization ()
{
  // TODO Auto-generated destructor stub
}

void Visualization::visualizeCloud (const sensor_msgs::PointCloud2& pointcloud_msg)
{
  PointCloudPtr cloud_ptr (new PointCloud);
  pcl17::fromROSMsg(pointcloud_msg,*cloud_ptr);

  visualizeCloud(cloud_ptr);
}

void Visualization::visualizeCloud (PointCloudConstPtr cloud_ptr)
{
  std::vector<PointCloudConstPtr> cloud_ptr_vec;
  cloud_ptr_vec.push_back(cloud_ptr);
  visualizeCloud(cloud_ptr_vec);
}

void Visualization::visualizeCloud (std::vector<PointCloudConstPtr>& cloud_ptr_vec)
{
  viewer_->removeAllPointClouds();
  for(std::vector<PointCloudConstPtr>::iterator itr=cloud_ptr_vec.begin(); itr != cloud_ptr_vec.end(); itr++)
  {
    int cloud_num = itr - cloud_ptr_vec.begin();
    std::stringstream cloud_name;
    cloud_name << "cloud "<< itr - cloud_ptr_vec.begin();

    // downsample cloud if needed
    PointCloudConstPtr downsampled_ptr;
    if(vox_grid_size_ > 0.0)
      downsampled_ptr = downsampleCloud(*itr);
    else
      downsampled_ptr = *itr;

    // different colours for different clouds
    pcl17::visualization::PointCloudColorHandlerCustom<PointType>
      single_color(downsampled_ptr, rand() % 255, rand() % 255, rand() % 255);

    //add the cloud
    viewer_->addPointCloud<PointType> (downsampled_ptr, single_color, cloud_name.str());
    viewer_->setPointCloudRenderingProperties (pcl17::visualization::PCL17_VISUALIZER_POINT_SIZE, 1,
      cloud_name.str());
  }
//  while (!viewer_->wasStopped ())
  this->spinOnce();
}

void Visualization::visualizeCloud (PointCloudConstPtr cloud_ptr, pcl17::PointIndicesConstPtr& cloud_indices_ptr)
{
  pcl17::ExtractIndices<PointType> extractor;
  PointCloudPtr output_cloud_ptr (new PointCloud);
  extractor.setIndices(cloud_indices_ptr);
  extractor.setInputCloud(cloud_ptr);
  extractor.filter(*output_cloud_ptr);
  visualizeCloud(output_cloud_ptr);
}

void Visualization::visualizeCloud (PointCloudConstPtr cloud_ptr, std::vector<pcl17::PointIndicesConstPtr>& cloud_indices_ptrs)
{
  std::vector<PointCloudConstPtr> clouds_to_visualize_ptrs;
  for(std::vector<pcl17::PointIndicesConstPtr>::const_iterator itr=cloud_indices_ptrs.begin(); itr!=cloud_indices_ptrs.end(); itr++)
  {
    pcl17::ExtractIndices<PointType> extractor;
    PointCloudPtr output_cloud_ptr (new PointCloud);
    extractor.setIndices(*itr);
    extractor.setInputCloud(cloud_ptr);
    extractor.filter(*output_cloud_ptr);
    clouds_to_visualize_ptrs.push_back(output_cloud_ptr);
  }
  visualizeCloud(clouds_to_visualize_ptrs);
}

void Visualization::visualizeCloudNormals (PointCloudConstPtr cloud_ptr, PointCloudNormalsConstPtr cloud_normals_ptr)
{
  viewer_->removeAllPointClouds();
  viewer_->addPointCloudNormals<PointType, PointNormal>(cloud_ptr, cloud_normals_ptr, 20);
  viewer_->setPointCloudRenderingProperties (pcl17::visualization::PCL17_VISUALIZER_POINT_SIZE, 1);
  this->spinOnce();
}

void Visualization::spinOnce()
{
  viewer_->spinOnce (100);
  //boost::this_thread::sleep (boost::posix_time::microseconds (100000));
}

void Visualization::visualizeImage(const sensor_msgs::Image& image_msg)
{
  sensor_msgs::ImagePtr msg_ptr (new sensor_msgs::Image(image_msg));
  cv_bridge::CvImagePtr cv_ptr;
  try
  {
    cv_ptr = cv_bridge::toCvCopy (msg_ptr, msg_ptr->encoding);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
  }
  cv::imshow("abc", cv_ptr->image);
  cv::waitKey(0);
}

PointCloudConstPtr Visualization::downsampleCloud (PointCloudConstPtr input)
{
  const double voxel_size = vox_grid_size_;
  PointCloudPtr cloud_filtered (new PointCloud);
  pcl17::VoxelGrid<PointType> downsampler;
  downsampler.setInputCloud (input);
  downsampler.setLeafSize (voxel_size, voxel_size, voxel_size);
  downsampler.filter (*cloud_filtered);
  return cloud_filtered;
}