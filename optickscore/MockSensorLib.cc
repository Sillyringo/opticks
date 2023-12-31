#include "SRand.hh"
#include "SLOG.hh"
#include "NPY.hpp"
#include "SensorLib.hh"
#include "MockSensorLib.hh"
#include "MockSensorAngularEfficiencyTable.hh"

SensorLib* MockSensorLib::Make(unsigned num_cat, unsigned num_sensor)  // static
{
    MockSensorLib msl(num_cat, num_sensor); 
    return msl.getSensorLib(); 
}

MockSensorLib::MockSensorLib(unsigned num_cat, unsigned num_sensor)
    :
    m_num_cat(num_cat),
    m_sensorlib(new SensorLib)
{
    initSensorData(num_sensor); 
    initAngularEfficiency( 180, 360 );  // height:theta, width:phi
}

/**
MockSensorLib::initSensorData
------------------------------

* sensors are randomly assigned a sensor category in range 0:m_num_cat-1
* overall efficiencies for all sensors are set to 0.5

**/

void MockSensorLib::initSensorData(unsigned num_sensor)
{
    LOG(info) << " num_sensor " << num_sensor ;  
    m_sensorlib->initSensorData(num_sensor); 
    for(unsigned i=0 ; i < num_sensor ; i++)
    {
        unsigned sensorIndex = 1+i ;  // 1-based 

        float efficiency_1 = 0.5f ;    
        float efficiency_2 = 1.0f ;    
        //int   sensor_cat = m_num_cat > 0 ? i % m_num_cat : -1 ; 
        int   sensor_cat = m_num_cat > 0 ? SRand::pick_random_category(m_num_cat) : -1 ; 

        unsigned sensor_identifier = 1000000 + sensorIndex - 1  ; 

        m_sensorlib->setSensorData( sensorIndex, efficiency_1, efficiency_2, sensor_cat, sensor_identifier );
    }   
}

/**
MockSensorLib::initAngularEfficiency
-------------------------------------

The angular efficiency table is created by MockSensorAngularEfficiencyTable

**/

void MockSensorLib::initAngularEfficiency(unsigned num_theta_steps, unsigned num_phi_steps)
{
    MockSensorAngularEfficiencyTable tab( m_num_cat, num_theta_steps, num_phi_steps );  // cat, height, width
    NPY<float>* angular_efficiency = tab.getArray(); 
    LOG(info) << "angular_efficiency " << angular_efficiency->getShapeString() ; 
    m_sensorlib->setSensorAngularEfficiency( angular_efficiency ); 
}

SensorLib* MockSensorLib::getSensorLib() const 
{
   return m_sensorlib ; 
}


