
#include <iostream>
#include <map>

#include <Eigen/Dense>

#include <kdl/tree.hpp>

#include <kdl_parser/kdl_parser.hpp>

#include <rtt_ros_tools/tools.h>
#include <kdl_urdf_tools/tools.h>
#include <controllerman_controllers/controllers/gravity_compensation.h>

using namespace controllerman_controllers::controllers;

GravityCompensation::GravityCompensation(std::string const& name) :
  TaskContext(name)
  // Properties
  ,robot_description_("")
  ,root_link_("")
  ,tip_link_("")
  ,gravity_(3,0.0)
  // Working variables
  ,n_dof_(0)
  ,kdl_tree_()
  ,kdl_chain_()
  ,id_solver_(NULL)
  ,ext_wrenches_()
  ,positions_()
  ,accelerations_()
  ,torques_()
{
  // Declare properties
  this->addProperty("robot_description",robot_description_)
    .doc("The WAM URDF xml string.");
  this->addProperty("gravity",gravity_)
    .doc("The gravity vector in the root link frame.");
  this->addProperty("root_link",root_link_)
    .doc("The root link for the controller.");
  this->addProperty("tip_link",tip_link_)
    .doc("The tip link for the controller.");

  // Configure data ports
  this->ports()->addEventPort("positions_in", positions_in_port_)
    .doc("Input port: nx1 vector of joint positions. (n joints)");
  this->ports()->addPort("torques_out", torques_out_port_)
    .doc("Output port: nx1 vector of joint torques. (n joints)");
  
  // Initialize properties from rosparam
  rtt_ros_tools::load_rosparam_and_refresh(this);
}

bool GravityCompensation::configureHook()
{
  // Initialize kinematics (KDL tree, KDL chain, and #DOF)
  urdf::Model urdf_model;
  if(!kdl_urdf_tools::initialize_kinematics_from_urdf(
        robot_description_, root_link_, tip_link_,
        n_dof_, kdl_chain_, kdl_tree_, urdf_model))
  {
    ROS_ERROR("Could not initialize robot kinematics!");
    return false;
  }

  // Create inverse dynamics chainsolver
  id_solver_.reset(
      new KDL::ChainIdSolver_RNE(
        kdl_chain_,
        KDL::Vector(gravity_[0],gravity_[1],gravity_[2])));

  // Resize working vectors
  positions_.resize(n_dof_);
  accelerations_.resize(n_dof_);
  torques_.resize(n_dof_);
  ext_wrenches_.resize(kdl_chain_.getNrOfSegments());

  // Zero out torque data
  torques_.data.setZero();
  accelerations_.data.setZero();

  // Prepare ports for realtime processing
  torques_out_port_.setDataSample(torques_);

  return true;
}

bool GravityCompensation::startHook()
{
  return true;
}

void GravityCompensation::updateHook()
{
  // Read in the current joint positions & velocities
  positions_in_port_.readNewest( positions_ );

  // Compute inverse dynamics
  // This computes the torques on each joint of the arm as a function of
  // the arm's joint-space position, velocities, accelerations, external
  // forces/torques and gravity.
  if(id_solver_->CartToJnt(
        positions_.q,
        positions_.qdot,
        accelerations_,
        ext_wrenches_,
        torques_) != 0)
  {
    ROS_ERROR("Could not compute joint torques!");
  }
 
  // Send joint positions
  torques_out_port_.write( torques_ );
}

void GravityCompensation::stopHook()
{
}

void GravityCompensation::cleanupHook()
{
}
