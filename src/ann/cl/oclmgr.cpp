#include "oclmgr.h"
#include "oclplatform.h"
#include "ocldevice.h"
#include "oclcontext.h"
#include "utils/log.h"

OclMgr::OclMgr()
{
    _computeDevice = new OclDevice(OclPlatform::queryPlatforms().front().queryDevices().front());
    _oclContext = new OclContext(*_computeDevice);
    sLog.log("OpenCL platform:");
    sLog.log("    Name: ", _computeDevice->getPlatform()->getName());
    sLog.log("    Vendor: ", _computeDevice->getPlatform()->getVendor());
    sLog.log("    OpenCL version: ", _computeDevice->getPlatform()->getOclMajorVersion(), ".", _computeDevice->getPlatform()->getOclMinorVersion());
    sLog.log("    Device:", (_computeDevice->isAvaliable() ? "" : " (unavaliable)"));
    sLog.log("        Name: ", _computeDevice->getName());
    sLog.log("        Vendor: ", _computeDevice->getVendor());
    sLog.log("        OpenCL version: ", _computeDevice->getOclMajorVersion(), ".", _computeDevice->getOclMinorVersion());
    sLog.log("        Adress bits: ", _computeDevice->getAdressSize(), (_computeDevice->isLittleEndian() ? " (little-endian)" : " (big-endian)"));
    sLog.log("        Avaliable compute units: ", _computeDevice->getAvaliableComputeUnits());
    sLog.log("        Clock frequency: ", _computeDevice->getClockFrequency());
    sLog.log("        Has image support: ", _computeDevice->hasImageSupport());
    sLog.log("        Max image width: ", _computeDevice->getImageMaxWidth());
    sLog.log("        Max image height: ", _computeDevice->getImageMaxHeight());
    sLog.log("        Max work items: ", _computeDevice->getMaxWorkItems());
    sLog.log("        Has compiler: ", _computeDevice->canCompile());
    sLog.log("");
}

OclMgr::~OclMgr()
{
    delete _oclContext;
    delete _computeDevice;
}
