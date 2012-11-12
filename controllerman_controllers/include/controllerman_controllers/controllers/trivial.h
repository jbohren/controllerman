#ifndef __CONTROLLERMAN_CONTROLLERS_CONTROLLERS_TRIVIAL_H
#define __CONTROLLERMAN_CONTROLLERS_CONTROLLERS_TRIVIAL_H

#include <iostream>

#include <kdl/jntarrayvel.hpp>
#include <kdl/tree.hpp>
#include <kdl/chain.hpp>

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

namespace controllerman_controllers {
  namespace controllers {
    class Trivial : public RTT::TaskContext
    {
      // RTT Properties
      std::string robot_description_;
      std::string root_link_;
      std::string tip_link_;

      // RTT Ports
      RTT::InputPort<KDL::JntArrayVel> positions_in_port_;
      RTT::OutputPort<KDL::JntArray> torques_out_port_;

    public:
      Trivial(std::string const& name);


      bool configureHook();
      bool startHook();
      void updateHook();
      void stopHook();
      void cleanupHook();

    private:
      // Working variables
      unsigned int n_dof_;
      KDL::JntArrayVel positions_;
      KDL::JntArray torques_;
    };
  }
}


#endif // ifndef __CONTROLLERMAN_CONTROLLERS_CONTROLLERS_TRIVIAL_H
