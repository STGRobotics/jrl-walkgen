/*
 * Copyright 2011
 *
 * Andrei Herdt
 * Francois Keith
 * Olivier Stasse
 *
 * JRL, CNRS/AIST
 *
 * This file is part of walkGenJrl.
 * walkGenJrl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * walkGenJrl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with walkGenJrl.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Research carried out within the scope of the 
 *  Joint Japanese-French Robotics Laboratory (JRL)
 */
/* This object generate all the values for the foot trajectories, */
#include <iostream>
#include <fstream>

#include <privatepgtypes.h>

#include "OnLineFootTrajectoryGeneration.h"


using namespace PatternGeneratorJRL;

OnLineFootTrajectoryGeneration::OnLineFootTrajectoryGeneration(SimplePluginManager *lSPM,
    CjrlFoot *aFoot)
: FootTrajectoryGenerationStandard(lSPM,aFoot)
{

}

OnLineFootTrajectoryGeneration::~OnLineFootTrajectoryGeneration()
{

}


void
OnLineFootTrajectoryGeneration::UpdateFootPosition(deque<FootAbsolutePosition> &SupportFootAbsolutePositions,
    deque<FootAbsolutePosition> &NoneSupportFootAbsolutePositions,
    int StartIndex, int k,
    double LocalInterpolationStartTime,
    double UnlockedSwingPeriod,
    int StepType, int /* LeftOrRight */)
{

  // Local time
  double InterpolationTime = (double)k*m_SamplingPeriod;
  int CurrentAbsoluteIndex = k+StartIndex;
  double EndOfLiftOff = (m_TSingle-UnlockedSwingPeriod)*0.5;
  double StartLanding = EndOfLiftOff + UnlockedSwingPeriod;

  // The foot support does not move.
  SupportFootAbsolutePositions[CurrentAbsoluteIndex] = 
      SupportFootAbsolutePositions[StartIndex-1];
  SupportFootAbsolutePositions[CurrentAbsoluteIndex].stepType = (-1)*StepType;
  NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].stepType = StepType;

  if (LocalInterpolationStartTime +InterpolationTime <= EndOfLiftOff || LocalInterpolationStartTime +InterpolationTime >= StartLanding)
    {
      // Do not modify x, y and theta while liftoff.
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].x = 
          NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex-1].x;
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].y = 
          NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex-1].y;
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].theta = 
          NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex-1].theta;
    }
  else if (LocalInterpolationStartTime < EndOfLiftOff && LocalInterpolationStartTime +InterpolationTime > EndOfLiftOff)
    {
      // DO MODIFY x, y and theta the remaining time.
      // x, dx
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].x = 
          m_PolynomeX->Compute(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].dx = 
          m_PolynomeX->ComputeDerivative(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
      if(m_PolynomeX->Degree() > 4)
        NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].ddx =
            m_PolynomeX->ComputeSecDerivative(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
      //y, dy
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].y = 
          m_PolynomeY->Compute(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].dy = 
          m_PolynomeY->ComputeDerivative(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
      if(m_PolynomeY->Degree() > 4)
        NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].ddy =
            m_PolynomeY->ComputeSecDerivative(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
      //theta, dtheta
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].theta = 
          m_PolynomeTheta->Compute(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].dtheta = 
          m_PolynomeTheta->ComputeDerivative(LocalInterpolationStartTime + InterpolationTime - EndOfLiftOff);
    }
  else 
    {
      // DO MODIFY x, y and theta all the time.
      // x, dx
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].x = 
          m_PolynomeX->Compute(InterpolationTime);
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].dx = 
          m_PolynomeX->ComputeDerivative(InterpolationTime);
      if(m_PolynomeX->Degree() > 4)
         NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].ddx =
             m_PolynomeX->ComputeSecDerivative(InterpolationTime);
      //y, dy
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].y = 
          m_PolynomeY->Compute(InterpolationTime);
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].dy = 
          m_PolynomeY->ComputeDerivative(InterpolationTime);
      if(m_PolynomeY->Degree() > 4)
         NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].ddy =
             m_PolynomeY->ComputeSecDerivative(InterpolationTime);
      //theta, dtheta
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].theta = 
          m_PolynomeTheta->Compute( InterpolationTime );
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].dtheta = 
          m_PolynomeTheta->ComputeDerivative(InterpolationTime);
    }

  NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].z = 
      m_PolynomeZ->Compute(LocalInterpolationStartTime+InterpolationTime);
  NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].dz = 
      m_PolynomeZ->Compute(LocalInterpolationStartTime+InterpolationTime);

  bool ProtectionNeeded=false;

  // Treat Omega with the following strategy:
  // First treat the lift-off.
  if (LocalInterpolationStartTime+InterpolationTime<EndOfLiftOff)
    {
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].omega = 
          m_PolynomeOmega->Compute(InterpolationTime);
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].domega = 
          m_PolynomeOmega->Compute(InterpolationTime);

      ProtectionNeeded=true;
    }
  // Prepare for the landing.
  else if (LocalInterpolationStartTime+InterpolationTime<StartLanding)
    {
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].omega = 
          m_Omega - m_PolynomeOmega2->Compute(LocalInterpolationStartTime+InterpolationTime-EndOfLiftOff)-
          NoneSupportFootAbsolutePositions[StartIndex-1].omega2;
    }
  // Realize the landing.
  else 
    {
      NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].omega = 
          m_PolynomeOmega->Compute(LocalInterpolationStartTime+InterpolationTime - StartLanding) +
          NoneSupportFootAbsolutePositions[StartIndex-1].omega - m_Omega;
      ProtectionNeeded=true;
    }
  double dFX=0,dFY=0,dFZ=0;
  double lOmega = 0.0;
  lOmega = NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].omega*M_PI/180.0;
  double lTheta = 0.0;
  lTheta = NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].theta*M_PI/180.0;

  double c = cos(lTheta);
  double s = sin(lTheta);

  {
    // Make sure the foot is not going inside the floor.
    double dX=0,Z1=0,Z2=0,X1=0,X2=0;
    double B=m_FootB,H=m_FootH,F=m_FootF; 

    if (lOmega<0)
      {
        X1 = B*cos(-lOmega);
        X2 = H*sin(-lOmega);
        Z1 = H*cos(-lOmega);
        Z2 = B*sin(-lOmega);
        dX = -(B - X1 + X2);
        dFZ = Z1 + Z2 - H;
      }
    else
      {
        X1 = F*cos(lOmega);
        X2 = H*sin(lOmega);
        Z1 = H*cos(lOmega);
        Z2 = F*sin(lOmega);
        dX = (F - X1 + X2);
        dFZ = Z1 + Z2 - H;
      }
    dFX = c*dX;
    dFY = s*dX;
  }


  MAL_S3_VECTOR(Foot_Shift,double);

  // Modification of the foot position.
  NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].x += dFX ;
  NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].y += dFY ;
  NoneSupportFootAbsolutePositions[CurrentAbsoluteIndex].z += dFZ ;

}


void
OnLineFootTrajectoryGeneration::check_solution(double & X, double & Y,
    const support_state_t & CurrentSupport, double CurrentTime)
{
  double Sign;
  if(CurrentSupport.Foot == LEFT)
    Sign = 1.0;
  else
    Sign = -1.0;
  if(CurrentSupport.NbStepsLeft > 0)
    {
      if(fabs(X)+fabs(Y)-0.00001<0.0)
        {
          //cout<<"Previewed foot x-position zero at time: "<<CurrentTime<<endl;
        }
      else if (CurrentSupport.TimeLimit-CurrentTime-QP_T_/2.0 > 0.0)
        {//The landing position is yet determined by the solver because the robot finds himself still in the single support phase
          //do nothing
        }
    }
  else
    {//The solver isn't responsible for the feet positions anymore
      //The robot is supposed to stop always with the feet aligned in the lateral plane.
      X = CurrentSupport.x + Sign*sin(CurrentSupport.yaw)*FeetDistanceDS_;
      Y = CurrentSupport.y - Sign*cos(CurrentSupport.yaw)*FeetDistanceDS_;
    }
}


void
OnLineFootTrajectoryGeneration::interpolate_feet_positions(double Time,
    const support_state_t & CurrentSupport,
    double FPx, double FPy,
    const deque<double> & PreviewedSupportAngles_deq,
    deque<FootAbsolutePosition> &FinalLeftFootTraj_deq,
    deque<FootAbsolutePosition> &FinalRightFootTraj_deq)
{

  unsigned int CurrentIndex = FinalLeftFootTraj_deq.size()-1;
  FinalLeftFootTraj_deq.resize((unsigned int)(QP_T_/m_SamplingPeriod)+CurrentIndex+1);
  FinalRightFootTraj_deq.resize((unsigned int)(QP_T_/m_SamplingPeriod)+CurrentIndex+1);
  check_solution( FPx, FPy, CurrentSupport, Time);

  double LocalInterpolationTime = Time-(CurrentSupport.TimeLimit-(m_TDouble+m_TSingle));

  double StepHeight = 0.05;
  int StepType = 1;

  if(CurrentSupport.Phase == SS && Time+3.0/2.0*QP_T_ < CurrentSupport.TimeLimit)
    {
      //determine coefficients of interpolation polynom
      double ModulationSupportCoefficient = 0.9;
      double UnlockedSwingPeriod = m_TSingle * ModulationSupportCoefficient;
      double EndOfLiftOff = (m_TSingle-UnlockedSwingPeriod)*0.5;
      double InterpolationTimePassed = 0.0;
      if(LocalInterpolationTime>EndOfLiftOff)
        InterpolationTimePassed = LocalInterpolationTime-EndOfLiftOff;

      FootAbsolutePosition LastSwingFootPosition;

      if(CurrentSupport.Foot == LEFT)
        {
          LastSwingFootPosition = FinalRightFootTraj_deq[CurrentIndex];
        }
      else
        {
          LastSwingFootPosition = FinalLeftFootTraj_deq[CurrentIndex];
        }

      //Set parameters for current polynomial
      SetParameters(FootTrajectoryGenerationStandard::X_AXIS,
          UnlockedSwingPeriod-InterpolationTimePassed,FPx,
          LastSwingFootPosition.x, LastSwingFootPosition.dx, LastSwingFootPosition.ddx);
      SetParameters(FootTrajectoryGenerationStandard::Y_AXIS,
          UnlockedSwingPeriod-InterpolationTimePassed,FPy,
          LastSwingFootPosition.y, LastSwingFootPosition.dy, LastSwingFootPosition.ddy);

      if(CurrentSupport.StateChanged==true)
        SetParameters(FootTrajectoryGenerationStandard::Z_AXIS, m_TSingle,StepHeight);

      SetParametersWithInitPosInitSpeed(FootTrajectoryGenerationStandard::THETA_AXIS,
          UnlockedSwingPeriod-InterpolationTimePassed,
          PreviewedSupportAngles_deq[0]*180.0/M_PI,
          LastSwingFootPosition.theta,
          LastSwingFootPosition.dtheta);
      SetParametersWithInitPosInitSpeed(FootTrajectoryGenerationStandard::OMEGA_AXIS,
          UnlockedSwingPeriod-InterpolationTimePassed,0.0*180.0/M_PI,
          LastSwingFootPosition.omega,
          LastSwingFootPosition.domega);
      SetParametersWithInitPosInitSpeed(FootTrajectoryGenerationStandard::OMEGA2_AXIS,
          UnlockedSwingPeriod-InterpolationTimePassed,2*0.0*180.0/M_PI,
          LastSwingFootPosition.omega2,
          LastSwingFootPosition.domega2);

      for(int k = 1; k<=(int)(QP_T_/m_SamplingPeriod);k++)
        {
          if (CurrentSupport.Foot == LEFT)
            {
              UpdateFootPosition(FinalLeftFootTraj_deq,
                  FinalRightFootTraj_deq,
                  CurrentIndex,k,
                  LocalInterpolationTime,
                  UnlockedSwingPeriod,
                  StepType, -1);
            }
          else
            {
              UpdateFootPosition(FinalRightFootTraj_deq,
                  FinalLeftFootTraj_deq,
                  CurrentIndex,k,
                  LocalInterpolationTime,
                  UnlockedSwingPeriod,
                  StepType, 1);
            }
          FinalLeftFootTraj_deq[CurrentIndex+k].time =
              FinalRightFootTraj_deq[CurrentIndex+k].time = Time+k*m_SamplingPeriod;
        }
    }
  else if (CurrentSupport.Phase == DS || Time+3.0/2.0*QP_T_ > CurrentSupport.TimeLimit)
    {
      for(int k = 0; k<=(int)(QP_T_/m_SamplingPeriod);k++)
        {
          FinalRightFootTraj_deq[CurrentIndex+k]=FinalRightFootTraj_deq[CurrentIndex+k-1];
          FinalLeftFootTraj_deq[CurrentIndex+k]=FinalLeftFootTraj_deq[CurrentIndex+k-1];
          FinalLeftFootTraj_deq[CurrentIndex+k].time =
              FinalRightFootTraj_deq[CurrentIndex+k].time = Time+k*m_SamplingPeriod;
          FinalLeftFootTraj_deq[CurrentIndex+k].stepType =
              FinalRightFootTraj_deq[CurrentIndex+k].stepType = 10;
        }
    }
}



