#ifndef BOUNDED_EXPLORE_LAYER_H_
#define BOUNDED_EXPLORE_LAYER_H_

#include <ros/ros.h>
#include <costmap_2d/layer.h>
#include <costmap_2d/GenericPluginConfig.h>
#include <dynamic_reconfigure/server.h>

#include <geometry_msgs/Polygon.h>
#include <std_srvs/Empty.h>
#include <frontier_exploration/Frontier.h>
#include <frontier_exploration/UpdateBoundaryPolygon.h>
#include <frontier_exploration/GetNextFrontier.h>
#include <frontier_exploration/BlacklistPoint.h>

namespace frontier_exploration
{

/**
 * @brief costmap_2d layer plugin that holds the state for a bounded frontier exploration task.
 * Manages the boundary polygon, superimposes the polygon on the overall exploration costmap,
 * and processes costmap to find next frontier to explore.
 */
class BoundedExploreLayer : public costmap_2d::Layer, public costmap_2d::Costmap2D
{
public:
    BoundedExploreLayer();
    ~BoundedExploreLayer();

    /**
     * @brief Loads default values and initialize exploration costmap.
     */
    virtual void onInitialize();

    /**
     * @brief Calculate bounds of costmap window to update
     */
    virtual void updateBounds(double origin_x, double origin_y, double origin_yaw, double* polygon_min_x, double* polygon_min_y, double* polygon_max_x,
                              double* polygon_max_y);

    /**
     * @brief Update requested costmap window
     */
    virtual void updateCosts(costmap_2d::Costmap2D& master_grid, int min_i, int min_j, int max_i, int max_j);
    bool isDiscretized()
    {
        return true;
    }

    /**
     * @brief Match dimensions and origin of parent costmap
     */
    virtual void matchSize();

    /**
     * @brief Reset exploration progress
     */
    virtual void reset();
    
    /**
     * @brief Future define for visualization_msgs::Marker::DELETEALL. Constant is not defined in ROS Indigo, but functionality is implemented
     */
    static const int DELETEALL = 3;

protected:

    /**
     * @brief ROS Service wrapper for updateBoundaryPolygon
     * @param req Service request
     * @param res Service response
     * @return True on service success, false otherwise
     */
    bool updateBoundaryPolygonService(frontier_exploration::UpdateBoundaryPolygon::Request &req, frontier_exploration::UpdateBoundaryPolygon::Response &res);

    /**
     * @brief Load polygon boundary to draw on map with each update
     * @param polygon_stamped polygon boundary
     * @return True if polygon successfully loaded, false otherwise
     */
    bool updateBoundaryPolygon(geometry_msgs::PolygonStamped polygon_stamped);

    /**
     * @brief ROS Service wrapper for getNextFrontier
     * @param req Service request
     * @param res Service response
     * @return True on service success, false otherwise
     */
    bool getNextFrontierService(frontier_exploration::GetNextFrontier::Request &req, frontier_exploration::GetNextFrontier::Response &res);

    /**
     * @brief Search the costmap for next reachable frontier to explore
     * @param start_pose Pose from which to start search
     * @param next_frontier Pose of found frontier
     * @return True if a reachable frontier was found, false otherwise
     */
    bool getNextFrontier(geometry_msgs::PoseStamped start_pose, geometry_msgs::PoseStamped &next_frontier);

    /**
     * @brief ROS Service wrapper for adding a point to the frontier blacklist
     * @param req Service request
     * @param res Service response
     * @return Always true
     */
    bool blacklistPointService(frontier_exploration::BlacklistPoint::Request &req, frontier_exploration::BlacklistPoint::Response &res);

    /**
     * @brief ROS Service wrapper for clearing the frontier blacklist
     * @param req Service request
     * @param res Service response
     * @return Always true
     */
    bool clearBlacklistService(std_srvs::Empty::Request &req, std_srvs::Empty::Response &resp);

private:

    /**
     * @brief Update the map with exploration boundary data
     * @param master_grid Reference to master costmap
     */
    void mapUpdateKeepObstacles(costmap_2d::Costmap2D& master_grid, int min_i, int min_j, int max_i, int max_j);

    void reconfigureCB(costmap_2d::GenericPluginConfig &config, uint32_t level);

    dynamic_reconfigure::Server<costmap_2d::GenericPluginConfig> *dsrv_;
    ros::ServiceServer polygonService_;
    ros::ServiceServer frontierService_;
    ros::ServiceServer blacklistPointService_;
    ros::ServiceServer clearBlacklistService_;
    geometry_msgs::Polygon polygon_;
    tf::TransformListener tf_listener_;

    ros::Publisher frontier_cloud_pub;
    ros::Publisher blacklist_marker_pub_;

    bool configured_, marked_;
    
    std::list<geometry_msgs::Point> blacklist_;
    
    std::string global_frame_;
    std::string frontier_travel_point_;
    double preferred_travel_point_distance_;
    bool resize_to_boundary_;
    int min_frontier_size_;
    double blacklist_radius_;
    
};

}
#endif
