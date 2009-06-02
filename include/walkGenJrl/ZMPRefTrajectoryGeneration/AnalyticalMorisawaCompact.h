/*! \file AnalyticalMorisawaCompact.h
   \brief Compact form of the analytical solution to generate the ZMP and the CoM.

   This object generate the reference value for the
   ZMP based on a polynomail representation
   of the ZMP following 
   "Experimentation of Humanoid Walking Allowing Immediate
   Modification of Foot Place Based on Analytical Solution"
   Morisawa, Harada, Kajita, Nakaoka, Fujiwara, Kanehiro, Hirukawa, 
   ICRA 2007, 3989--39994


   Copyright (c) 2007, 
   Francios Keith, Olivier Stasse,
   
   JRL-Japan, CNRS/AIST

   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without modification, 
   are permitted provided that the following conditions are met:
   
   * Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
   * Neither the name of the CNRS/AIST nor the names of its contributors 
   may be used to endorse or promote products derived from this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
   AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
   OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _ANALYTICAL_MORISAWA_FULL_H_
#define _ANALYTICAL_MORISAWA_FULL_H_

#include <dynamicsJRLJapan/HumanoidSpecificities.h>

#include <walkGenJrl/walkGenJrl_API.h>
#include <walkGenJrl/Clock.h>
#include <walkGenJrl/Mathematics/PolynomeFoot.h>
#include <walkGenJrl/Mathematics/ConvexHull.h>
#include <walkGenJrl/Mathematics/AnalyticalZMPCOGTrajectory.h>
#include <walkGenJrl/PreviewControl/PreviewControl.h>
#include <walkGenJrl/ZMPRefTrajectoryGeneration/AnalyticalMorisawaAbstract.h>
#include <walkGenJrl/ZMPRefTrajectoryGeneration/FilteringAnalyticalTrajectoryByPreviewControl.h>
#include <walkGenJrl/FootTrajectoryGeneration/LeftAndRightFootTrajectoryGenerationMultiple.h>


namespace PatternGeneratorJRL
{
  /*!     @ingroup analyticalformulation
    Structure to specify the parameter of a trajectory. */
  typedef struct
  {
    /* ! Initial value of the CoM for the spline representation. */
    double InitialCoM;

    /*! Initial value of the CoM speed for the spline representation */
    double InitialCoMSpeed;

    /*! Final value of the CoM for the spline representation */
    double FinalCoMPos;

    /*! ZMP profil for the chosen axis. */
    std::vector<double> * ZMPProfil; 

    /*! Height ZMP profil.*/
    std::vector<double> * ZMPZ;

    /*! Height COM profil. */
    std::vector<double> * CoMZ;
    
  } CompactTrajectoryInstanceParameters;

  /*!     @ingroup analyticalformulation
    This structure is used to compute the resultant fluctuation
   to move from one set of ZMP-COM trajectories to an other. */
  typedef struct
  {
    double CoMInit, CoMNew;
    double CoMSpeedInit, CoMSpeedNew;
    double ZMPInit, ZMPNew;
    double ZMPSpeedInit, ZMPSpeedNew;
  } FluctuationParameters;

  /*! \brief Class to compute analytically in a compact form the trajectories of both the ZMP and the CoM.
    @ingroup analyticalformulation
   */
  class WALK_GEN_JRL_EXPORT AnalyticalMorisawaCompact: public AnalyticalMorisawaAbstract
    {
      
    public:

      /*! \name Constants to handle errors 
	when changing foot steps.
	 @{ */
      const static int ERROR_WRONG_FOOT_TYPE = -1;
      const static int ERROR_TOO_LATE_FOR_MODIFICATION = -2;
      /*! @} */

      /*! \name Constants to handle reference frame when changing the feet on-line
	 @{ */
      const static unsigned int ABSOLUTE_FRAME = 0;
      const static unsigned int RELATIVE_FRAME = 1;
      /*! @} */
      /*! Constructor */
      AnalyticalMorisawaCompact(SimplePluginManager * lSPM);
      
      /*! Destructor */
      virtual ~AnalyticalMorisawaCompact();

      /*! \name Methods inherited from ZMPRefTrajectoryGeneration and reimplemented
	@{ */

      /*! Returns the CoM and ZMP trajectory for some relative foot positions. 
	Generate ZMP discreatization from a vector of foot position.
	ASSUME A COMPLETE MOTION FROM END TO START, and GENERATE EVERY VALUE.
	
	@param[out] ZMPPositions: Returns the ZMP reference values for the overall motion.
	Those are absolute position in the world reference frame. The origin is the initial
	position of the robot. The relative foot position specified are added.
	
	@param[out] CoMPositions: Returns the CoM reference values for the overall motion.
	Those are absolute position in the world reference frame. The origin is the initial
	position of the robot. The relative foot position specified are added.

	@param[in] RelativeFootPositions: The set of 
	relative steps to be performed by the robot.
		
	@param[out] LeftFootAbsolutePositions: Returns the absolute position of the left foot.
	According to the macro FULL_POLYNOME the trajectory will follow a third order
	polynom or a fifth order. By experience it is wise to put a third order. 
	A null acceleration might cause problem for the compensation of the Z-axis momentum.
	
	@param[out] RightFootAbsolutePositions: Returns the absolute position of the right foot.
	
	
	@param[in] Xmax: The maximal distance of a hand along the X axis in the waist coordinates.
	
	@param[in] lStartingCOMPosition: The initial position of the CoM.

	@param[in] lStartingZMPPosition: The initial position of the ZMP.

	@param[in] InitLeftFootAbsolutePosition: The initial position of the left foot.
	
	@param[in] InitRightFootAbsolutePosition: The initial position of the right foot.
      */
      void GetZMPDiscretization(deque<ZMPPosition> & ZMPPositions,
				deque<COMPosition> & CoMPositions,
				deque<RelativeFootPosition> &RelativeFootPositions,
				deque<FootAbsolutePosition> &LeftFootAbsolutePositions,
				deque<FootAbsolutePosition> &RightFootAbsolutePositions,
				double Xmax,
				COMPosition & lStartingCOMPosition,
				MAL_S3_VECTOR(,double) &lStartingZMPPosition,
				FootAbsolutePosition & InitLeftFootAbsolutePosition,
				FootAbsolutePosition & InitRightFootAbsolutePosition);
      
      /*! \brief Methods for on-line generation. 
	The queues will be updated as follows:
	- The first values necessary to start walking will be inserted.
	- The initial positions of the feet will be taken into account
	according to InitLeftFootAbsolutePosition and InitRightFootAbsolutePosition.
	- The RelativeFootPositions stack will be taken into account,
	in this case only three steps will be removed from the stack,
	- The starting COM Position.
	Returns the number of steps which has been completely put inside 
	the queue of ZMP, and foot positions.

	@param[out] FinalZMPPositions: The queue of ZMP reference positions.
	@param[out] CoMPositions: The queue of COM reference positions.
	@param[out] FinalLeftFootAbsolutePositions: The queue of left foot absolute positions.
	@param[out] FinalRightFootAbsolutePositions: The queue of right foot absolute positions.
	@param[in] InitLeftFootAbsolutePosition: The initial position of the left foot.
	@param[in] InitRightFootAbsolutePosition: The initial position of the right foot.
	@param[in] RelativeFootPositions: The set of relative positions to be taken into account.
	@param[in] lStartingCOMPosition: The initial position of the CoM given as a 3D vector.
	@param[in] lStartingZMPPosition: The initial position of the ZMP given as a 3D vector.
      */
      int InitOnLine(deque<ZMPPosition> & FinalZMPPositions,
		     deque<COMPosition> & CoMPositions,
		     deque<FootAbsolutePosition> & FinalLeftFootAbsolutePositions,
		     deque<FootAbsolutePosition> & FinalRightFootAbsolutePositions,
		     FootAbsolutePosition & InitLeftFootAbsolutePosition,
		     FootAbsolutePosition & InitRightFootAbsolutePosition,
		     deque<RelativeFootPosition> &RelativeFootPositions,
		     COMPosition &lStartingCOMPosition,
		     MAL_S3_VECTOR(&,double) lStartingZMPPosition
		     );
      
      /* ! \brief Methods to update the stack on-line by inserting a new foot position. 
        The foot is put right at the end of the stack. This method is supposed to be called
        when the first foot in the stack is finished.
       
        @param[in] NewRelativeFootPosition: The new foot to put in the stack.
        @param[out] FinalZMPPositions: The stack of final ZMP positions to be updated
        with the new relative foot position.
        @param[out] FinalLeftFootAbsolutePositions: The stack of final absolute left foot positions
	according to the new relative foot position.
	@param[out] FinalRightFootAbsolutePositions: The stack of final absolute right foot positions
	according to the new relative foot position.
	@param[in] EndSequence: Inherited from abstract interface and unused.

       */
      void OnLineAddFoot(RelativeFootPosition & NewRelativeFootPosition,
			 deque<ZMPPosition> & FinalZMPPositions,		
			 deque<COMPosition> & CoMPositions,			     
			 deque<FootAbsolutePosition> &FinalLeftFootAbsolutePositions,
			 deque<FootAbsolutePosition> &FinalRightFootAbsolutePositions,
			 bool EndSequence);
      
      /* ! \brief Method to update the stacks on-line */
      void OnLine(double time,
		  deque<ZMPPosition> & FinalZMPPositions,		
		  deque<COMPosition> & CoMPositions,			     
		  deque<FootAbsolutePosition> &FinalLeftFootAbsolutePositions,
		  deque<FootAbsolutePosition> &FinalRightFootAbsolutePositions);

      /* ! \brief Method to change on line the landing position of a foot.
	 @return If the method failed it returns -1, 0 otherwise.
      */
      int OnLineFootChange(double time,
			   FootAbsolutePosition &aFootPosition,				
			   deque<ZMPPosition> & FinalZMPPositions,			     
			   deque<COMPosition> & CoMPositions,
			   deque<FootAbsolutePosition> &FinalLeftFootAbsolutePositions,
			   deque<FootAbsolutePosition> &FinalRightFootAbsolutePositions,
			   StepStackHandler *aStepStackHandler=0);

      /* ! \brief Method to change on line the landing position of several feet.
	 @return If the method failed it returns -1, 0 otherwise.
      */
      int OnLineFootChanges(double time,
			    deque<FootAbsolutePosition> &FeetPosition,				
			    deque<ZMPPosition> & FinalZMPPositions,			     
			    deque<COMPosition> & CoMPositions,
			    deque<FootAbsolutePosition> &FinalLeftFootAbsolutePositions,
			    deque<FootAbsolutePosition> &FinalRightFootAbsolutePositions,
			    StepStackHandler *aStepStackHandler=0);

      /*! \brief Method to stop walking.
	@param[out] ZMPPositions: The queue of ZMP reference positions.
	@param[out] FinalCOMPositions: The queue of COM reference positions.
	@param[out] LeftFootAbsolutePositions: The queue of left foot absolute positions.
	@param[out] RightFootAbsolutePositions: The queue of right foot absolute positions.
      */
      void EndPhaseOfTheWalking(deque<ZMPPosition> &ZMPPositions,
				deque<COMPosition> &FinalCOMPositions,
				deque<FootAbsolutePosition> &LeftFootAbsolutePositions,
				deque<FootAbsolutePosition> &RightFootAbsolutePositions);


      
      /*! \brief Return the time at which it is optimal to regenerate a step in online mode. 
       */
      int ReturnOptimalTimeToRegenerateAStep();
      
      /*! @} */
      
      /*! \name Methods specifics to our current implementation.
       @{ */
      /*! \name Methods for the full linear system 
	@{ */

      /*! \brief Building the Z matrix to be inverted. 
	@param[in] lCoMZ: Profile of the CoM's height trajectory for each interval.
	@param[in] lZMPZ: Profile of the ZMP's height trajectory for each interval.
       */
      void BuildingTheZMatrix(std::vector<double> &lCoMZ, std::vector<double> &lZMPZ );

      /*! \brief Building the Z matrix to be inverted. */
      void BuildingTheZMatrix();

      /*! \brief Building the w vector. 
	It is currently assume that all ZMP's speed will be
	set to zero, as well as the final COM's speed.
	The sequence of ZMPSequence is the final value of the
	ZMP. As a special case, the first interval being set
	as a single support phase
	@param[in] InitialCoMPos: Initial Position of the CoM.
	@param[in] InitialComSpeed: Initial Speed of the CoM,
	@param[in] ZMPPosSequence: Set of position of the ZMP for the end of each interval.
	@param[in] FinalCoMPos: Final position of the CoM.
	@param[in] aAZCT: The analytical trajectory which store the coefficients we need
	to compute appropriatly the coefficients of \f[ w \f].

       */
      void ComputeW(double InitialCoMPos,
		    double InitialComSpeed,
		    std::vector<double> &ZMPPosSequence,
		    double FinalCoMPos,
		    AnalyticalZMPCOGTrajectory &aAZCT);

      /*! \brief Transfert the computed weights to an Analytical ZMP COG
	Trajectory able to generate the appropriate intermediates values.
      @param aAZCT: The object to be filled with the appropriate intermediate values.
      @param lCoMZ: The height trajectory of the CoM.
      @param lZMPZ: The height trajectory of the ZMP.
      @param lZMPInit: The initial value of the ZMP.
      @param lZMPEnd : The final value of the ZMP.
      @param InitializeaAZCT: This boolean is ignored as aAZCT should
      be already initialized.
      */
      void TransfertTheCoefficientsToTrajectories(AnalyticalZMPCOGTrajectory &aAZCT,
						  std::vector<double> &lCoMZ,
						  std::vector<double> &lZMPZ,
						  double &lZMPInit,
						  double &lZMPEnd,
						  bool InitializeaAZCT);
      /*! @} */

      /*! \brief Initialize automatically Polynomial degrees, and temporal intervals. 
	@return True if succeedeed, false otherwise.
       */
      bool InitializeBasicVariables();


      /*! \brief Compute the polynomial weights. */
      void ComputePolynomialWeights();

      /*! \brief Compute the polynomial weights. */
      void ComputePolynomialWeights2();

      /*! \brief Compute a trajectory with the given parameters.
	This method assumes that a Z matrix has already been computed. */
      void ComputeTrajectory(CompactTrajectoryInstanceParameters &aCTIP,
			     AnalyticalZMPCOGTrajectory &aAZCT);

      /*! \brief Reset internal variables to compute a new
	problem */
      void ResetTheResolutionOfThePolynomial();

      /*! \brief For the current time t, we will change the foot position 
	(NewPosX, NewPosY) during time interval IndexStep and IndexStep+1, using 
	the AnalyticalZMPCOGTrajectory objects and their parameters. 
	IndexStep has to be a double support phase, because it determines
	the landing position.
	
	@param[in] t : The current time.
	@param[in] IndexStep: The index of the interval where the modification will start.
	The modification of the foot position is done over 2 intervals.
	@param[in] NewFootAbsPos: The new foot position in absolute coordinates.
	@param[out] aAZCTX: The analytical trajectory along the X axis which will be modified
	(if possible) to comply with the new position.
	@param[out] aCTIPX: Embed the initial and final conditions
	to generate the ZMP and CoM trajectories along the X axis.
	@param[out] aAZCTY: The analytical trajectory along the Y axis which will be modified
	(if possible) to comply with the new position.
	@param[out] aCTIPY: Embed the initial and final conditions
	to generate the ZMP and CoM trajectories along the Y axis.
	@param[in] TemporalShift : If true, this authorize the method to shift the time for the modified interval.
	@param[in] aStepStackHandler: Access to the stack of steps.
	@return : Returns an error index if the operation was not feasible. You should use
	string error message to get the corresponding error message.

      */
      int ChangeFootLandingPosition(double t,
				    vector<unsigned int> & IndexStep,
				    vector<FootAbsolutePosition> & NewFootAbsPos,
				    AnalyticalZMPCOGTrajectory &aAZCTX,
				    CompactTrajectoryInstanceParameters &aCTIPX,
				    AnalyticalZMPCOGTrajectory &aAZCTY,
				    CompactTrajectoryInstanceParameters &aCTIPY,
				    bool TemporalShift,
				    StepStackHandler *aStepStackHandler=0);

      /*! \brief For the current time t, we will change the foot position 
	(NewPosX, NewPosY) during time interval IndexStep and IndexStep+1.
	IndexStep has to be a double support phase, because it determines
	the landing position.
	
	@param[in] t : The current time.
	@param[in] IndexStep: The index of the interval where the modification will start.
	The modification of the foot position is done over 2 intervals.
	@param[in] NewFootAbsPos: The new foot position in the absolute frame coordinates.
	@return : Returns an error index if the operation was not feasible. You should use
	string error message to get the corresponding error message.

      */
      int ChangeFootLandingPosition(double t,
				    vector<unsigned int> & IndexStep,
 				    vector<FootAbsolutePosition> & NewFootAbsPos);
      
      
      /*! @} */
      
      /*! Put an error messages string in ErrorMessage,
       according to ErrorIndex. */
      void StringErrorMessage(int ErrorIndex, string & ErrorMessage);

      /*! \brief This method filter out the orthogonal trajectory to minimize the 
	fluctuation involved by the time shift. 
      */
      void FilterOutOrthogonalDirection(AnalyticalZMPCOGTrajectory & aAZCT,
					CompactTrajectoryInstanceParameters &aCTIP,
					deque<double> & ZMPTrajectory,
					deque<double> & CoGTrajectory);




      /*! \name Feet Trajectory Generator methods  
       @{ */
      /*! Set the feet trajectory generator */
      void SetFeetTrajectoryGenerator(LeftAndRightFootTrajectoryGenerationMultiple * aFeetTrajectoryGenerator);
      
      /*! Get the feet trajectory generator */
      LeftAndRightFootTrajectoryGenerationMultiple * GetFeetTrajectoryGenerator();
      
      /*! @} */
      
      /*! Simple plugin interfaces 
	@{
       */
      /*! Register methods. */
      void RegisterMethods();

      /*! Call methods according to the arguments. */
      void CallMethod(std::string & Method, std::istringstream &strm);
      
      /*! @} */
    protected:
      
	      
      /*! \name Internal Methods to compute the full linear
	system. 
	@{
      */

      /*! \brief Building the Z1 Matrix */
      void ComputeZ1(unsigned int &lindex);

      /*! \brief Building the Zj Matrix 
       @param intervalindex: Index of the interval, 
       @param colindex: Index of the column inside the matrix,
       @param rowindex: Index of the row inside the matrix. */
      void ComputeZj(unsigned int intervalindex, 
		     unsigned int &colindex, 
		     unsigned int &rowindex);
      
      /*! \brief Building the Zm Matrix */
      void ComputeZm(unsigned int intervalindex, 
		     unsigned int &colindex, 
		     unsigned int &rowindex);

      /*! \brief Considering the current time given by LocalTime,
	it identifies by IndexStartingInterval which interval is concerned by LocalTime.
	A new duration of the IndexStartingInterval is proposed in NewTj
	to be m_Tj[StartingIndexInterval]-LocalTime - sum[m_Tj[0..StartingIndexInterval-1]].
	LocalTime should be given in the local reference time.
      */
      int TimeChange(double LocalTime,
		     unsigned int IndexStep,
		     unsigned int &IndexStartingInterval,
		     double &FinalTime,
		     double &NewTj);

      /*! \brief Recomputing all the m_DeltaTj according to NewTime,
	and the index of the first interval. */
      void NewTimeIntervals(unsigned IndexStartingInterval,
			    double NewTime);
      
      /*! \brief Recompute the trajectories based on the current time (LocalTime),
	the new landing position and the time interval (IndexStep) when the 
	modification should take place.
       */
      void ConstraintsChange(double LocalTime,
			     FluctuationParameters FPX,
			     FluctuationParameters FPY,
			     CompactTrajectoryInstanceParameters &aCTIPX,
			     CompactTrajectoryInstanceParameters &aCTIPY,
			     unsigned int IndexStartingInterval,
			     StepStackHandler *aStepStackHandler=0);
	
      /*! \brief Compute the time to compensate for the ZMP fluctuation. */
      double TimeCompensationForZMPFluctuation(FluctuationParameters &aFluctuationParameters,
					       double DeltaTInit);

      /*! @} */

      /*! \name Internal Methods to generate steps and create the associated 
	problem. 
	
	@{
      */  

      /*! \brief Build and solve the linear problem associated with a set of relative footholds.
	@param[in] lStartingCOMPosition: Specify the initial condition of the CoM \f$(x,y,z)\f$ for the 
	resolution of the linear problem. The matrix is defined as:
	\f[
	\left(
	\begin{matrix}
	x & \dot{x} & \ddot{x} \\
	y & \dot{y} & \ddot{y} \\
	z & \dot{z} & \ddot{z} \\
	\end{matrix}
	\right)
	\f]
	@param[in] LeftFootInitialPosition: The initial position of the left foot in the <b>absolute</b>
	reference frame.
	@param[in] RightFootInitialPosition: The initial position of the right foot in the <b>absolute</b>
	reference frame.
	the absolute initial feet positions and the queue of relative foot positions. This is not
	a set of trajectories at each 5 ms, but the support foot absolute positions.
	Thus the size of this queue should be the same than the relative foot positions.

	@param[in] IgnoreFirstRelativeFoot: Boolean to ignore the first relative foot.
	@param[in] DoNotPrepareLastFoot:  Boolean to not perform for the ending sequence.

	
      */
      int BuildAndSolveCOMZMPForASetOfSteps(MAL_S3x3_MATRIX(& ,double) lStartingCOMPosition,
					    FootAbsolutePosition &LeftFootInitialPosition,
					    FootAbsolutePosition &RightFootInitialPosition,
					    bool IgnoreFirstRelativeFoot,
					    bool DoNotPrepareLastFoot);

      /*! Change the profil of the ZMP profil according to the index of the interval.
	\param IndexStep : Index of the interval to be changed. The index can be higher than
	the size of the preview window (+1). In this case, the step available in m_AbsPosition are
	used.
	\param aNewFootAbsPos : The absolute position of the step to be changed.
	\param aCTIPX : The trajectory parameters along the X axis.
	\param aCTIPY : The trajectory parameters along the Y axis.
      */
      void ChangeZMPProfil(vector<unsigned int> & IndexStep,
			   vector<FootAbsolutePosition> &aNewFootAbsPos,
			   CompactTrajectoryInstanceParameters &aCTIPX,
			   CompactTrajectoryInstanceParameters &aCTIPY);
      /*! @} */

      /*! \brief LU decomposition of the Z matrix. */
      MAL_MATRIX(,double) m_AF;

      /*! \brief Pivots of the Z matrix LU decomposition. */
      MAL_VECTOR(,int) m_IPIV;

      /*! \brief Boolean on the need to reset to the
	precomputed Z matrix LU decomposition */
      bool m_NeedToReset;

      /*! \brief Pointer to the preview control object used to 
	filter out the orthogonal direction . */
      PreviewControl *  m_PreviewControl;

      /*! \name Object to handle trajectories. 
	@{
	*/
      /*! \brief Analytical sagital trajectories */
      AnalyticalZMPCOGTrajectory *m_AnalyticalZMPCoGTrajectoryX;
      
      /*! \brief Analytical sagital trajectories */
      AnalyticalZMPCOGTrajectory *m_AnalyticalZMPCoGTrajectoryY;

      /*! \brief Foot Trajectory Generator */
      LeftAndRightFootTrajectoryGenerationMultiple * m_FeetTrajectoryGenerator;
      /*! @} */

      /*! @} */
      

      /*! \brief Stores the relative foot positions currently in the buffer */
      deque<RelativeFootPosition> m_RelativeFootPositions;

      /*! \brief Stores the absolute support foot positions currently in the buffer */ 
      deque<FootAbsolutePosition> m_AbsoluteSupportFootPositions;

      /*! \brief Store the currently realized support foot position. 
	\warning This field makes sense only direct ON-LINE mode.
       */
      FootAbsolutePosition m_AbsoluteCurrentSupportFootPosition;

      /*! \name Stores the current ZMP profil, initial and final conditions for the trajectories.
       @{ */

      /*! Along the X-axis. */
      CompactTrajectoryInstanceParameters m_CTIPX;

      /*! Along the Y-axis. */
      CompactTrajectoryInstanceParameters m_CTIPY;

      /*! \brief Upper time limit over which the stacks are not updated anymore
       when calling OnLine(). */
      double m_UpperTimeLimitToUpdateStacks;
      /*! @} */

      /*! Clocks for code measurement in OnLine()*/
      Clock m_Clock1, m_Clock2, m_Clock3, m_Clock4;

      /*! Boolean value to check if there is a new step
	in the stack. */
      bool m_NewStepInTheStackOfAbsolutePosition;

      /*! Height of the initial CoM position. */
      double m_InitialPoseCoMHeight;
    
      /*! On-line change step mode */
      unsigned int m_OnLineChangeStepMode;

      /*! Filtering the axis through a preview control. */
      FilteringAnalyticalTrajectoryByPreviewControl * m_FilterXaxisByPC, * m_FilterYaxisByPC;
      
      /*! Activate or desactivate the filtering. */
      bool m_FilteringActivate;

    public:
      /*! \name Methods related to the Preview Control object used
	by this class. @{ */
      /*! Allows to set the preview control object. */
      int SetPreviewControl(PreviewControl * aPreviewControl);

      /*! Get the preview control object. */
      PreviewControl * GetPreviewControl();
      /*! @} */
      

    };
};
#endif
