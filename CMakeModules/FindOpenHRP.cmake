##############################################################################
#
# Copyright JRL, CNRS/AIST, 2008
# 
# Description:
# Try to find OpenHRP/Make.Rules
# capabilities.
# Once run this will define: 
#
# OpenHRP_FOUND
# OpenHRP_DIR
#
# Authors:
# Olivier Stasse
#
#############################################################################
SET(HAVE_OPENHRP 0)
SET(OPENHRP_VERSION_2 0)
SET(OPENHRP_VERSION_3 0)
IF(NOT UNIX)
  MESSAGE("FindOpenHRP.cmake: only available for Unix.")
  SET(OPENHRP_FOUND FALSE)
ELSE(NOT UNIX)
  
  FIND_PATH(OPENHRP_HOME NAMES Make.vars
    PATHS $ENV{OPENHRPHOME}
    $ENV{HOME}/src/OpenHRP )
    		
  # MESSAGE(STATUS "OpenHRP: ${OPENHRP_HOME}")    
  ## --------------------------------
     
  IF(OPENHRP_HOME)
    SET(OPENHRP_FOUND TRUE)
    SET(HAVE_OPENHRP 1)
  ELSE(OPENHRP_HOME)
    SET(OPENHRP_FOUND FALSE)
  ENDIF(OPENHRP_HOME)

  # Try to find the version of OpenHRP
  FIND_PATH(OPENHRP3_HOME NAMES Make.vars.ubuntu.8.04
    PATHS $ENV{OPENHRP_HOME}
    $ENV{HOME}/src/OpenHRP )

  IF(OPENHRP3_HOME)
    SET(OPENHRP_VERSION_3 1)
    SET(OPENHRP_HRP2_MODEL_DIRECTORY ${OPENHRP_HOME}/Controller/IOserver/robot/HRP2JRL/model/)
    SET(OPENHRP_HRP2_CONFIG_DIRECTORY ${OPENHRP_HOME}/Controller/IOserver/robot/HRP2JRL/etc/)
  ELSE(OPENHRP3_HOME)
    SET(OPENHRP_VERSION_2 1)
    SET(OPENHRP_HRP2_MODEL_DIRECTORY ${OPENHRP_HOME}/etc/HRP2JRL/)
    SET(OPENHRP_HRP2_CONFIG_DIRECTORY ${OPENHRP_HOME}/Controller/IOserver/robot/HRP2JRL/bin/)
  ENDIF(OPENHRP3_HOME)

  IF (OPENHRP_FOUND)
    MESSAGE(STATUS "OpenHRP: Version 2: ${OPENHRP_VERSION_2} 3:${OPENHRP_VERSION_3}  Home : ${OPENHRP_HOME}" )
  ELSE(OPENHRP_FOUND)
    MESSAGE(STATUS "OpenHRP not found" )
  ENDIF(OPENHRP_FOUND)

  IF(OPENHRP_FOUND)
    IF(OMNIORB4_FOUND)
      SET(OPENHRP_CXX_FLAGS "-DCOMMERCIAL -Wall -Wunused ")
      SET(OPENHRP_LDD_FLAGS "")
    ENDIF(OMNIORB4_FOUND)
  ENDIF(OPENHRP_FOUND)
  
  SET(ROBOT HRP2JRL)
  SET(${openhrp_final_plugin_path} "${OPENHRP_HOME}/Controller/IOserver/robot/${ROBOT}/bin")
  
  # MESSAGE(STATUS "OpenHRP: ${OPENHRP_HOME}")
ENDIF(NOT UNIX)
