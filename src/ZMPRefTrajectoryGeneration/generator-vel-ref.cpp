/*
 * Copyright 2010,
 *
 * Andrei   Herdt
 * Olivier  Stasse
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
/*! This object constructs a QP as proposed by Herdt IROS 2010.
 */


#include <ZMPRefTrajectoryGeneration/generator-vel-ref.hh>

using namespace PatternGeneratorJRL;


GeneratorVelRef::GeneratorVelRef(SimplePluginManager *lSPM, std::string DataFile,
				 CjrlHumanoidDynamicRobot *aHS) : MPCTrajectoryGeneration(lSPM)
{
  //TODO:
}
	
		
GeneratorVelRef::~GeneratorVelRef()
{
  //TODO:
}

	
void
GeneratorVelRef::CallMethod(std::string &Method, std::istringstream &strm)
{
  //GeneratorVelRef::CallMethod(Method,strm);
}

	
void 
GeneratorVelRef::setPonderation( IntermedQPMat & Matrices, double weight, const int aObjectiveType)
{

  IntermedQPMat::objective_variant_t & Objective = Matrices.Objective( aObjectiveType );
  Objective.weight = weight;

}


void 
GeneratorVelRef::setReference(std::istringstream &strm)
{
  //TODO:
}
	

void
GeneratorVelRef::preview(IntermedQPMat Matrices,
			 SupportFSM * FSM, std::deque<support_state_t> & deqSupportStates)
{

  const support_state_t & CurrentSupport = deqSupportStates[0];
  support_state_t PreviewedSupport;

  //initialize the previewed support state before previewing
  PreviewedSupport.Phase  = CurrentSupport.Phase;
  PreviewedSupport.Foot  = CurrentSupport.Foot;
  PreviewedSupport.StepsLeft  = CurrentSupport.StepsLeft;
  PreviewedSupport.TimeLimit = CurrentSupport.TimeLimit;
  PreviewedSupport.StepNumber  = 0;

  const reference_t & RefVel = Matrices.Reference();
  for(int i=1;i<=m_N;i++)
    {
      FSM->setSupportState(m_CurrentTime, i, PreviewedSupport, RefVel);

      deqSupportStates.push_back(PreviewedSupport);
    }

}

	
void 
GeneratorVelRef::computeGlobalReference(IntermedQPMat & Matrices, COMState TrunkStateT)
{

  reference_t & Ref = Matrices.Reference();

  MAL_VECTOR_RESIZE(Ref.global.X,m_N);
  MAL_VECTOR_RESIZE(Ref.global.Y,m_N);

  for( int i=0;i<m_N;i++)
    {
      Ref.global.X(i) = Ref.local.x*cos(TrunkStateT.yaw[0]+TrunkStateT.yaw[1]*i*m_T_Prw)-
	Ref.local.y*sin(TrunkStateT.yaw[0]+TrunkStateT.yaw[1]*i*m_T_Prw);
      Ref.global.Y(i) = Ref.local.y*cos(TrunkStateT.yaw[0]+TrunkStateT.yaw[1]*i*m_T_Prw)+
	Ref.local.x*sin(TrunkStateT.yaw[0]+TrunkStateT.yaw[1]*i*m_T_Prw);
    }

}


void 
GeneratorVelRef::initialize(IntermedQPMat & Matrices)
{

  IntermedQPMat::objective_variant_t & InstVel = Matrices.Objective( IntermedQPMat::INSTANT_VELOCITY );
  initializeMatrices( InstVel );
  IntermedQPMat::objective_variant_t & COPCent = Matrices.Objective( IntermedQPMat::COP_CENTERING );
  initializeMatrices( COPCent );
  IntermedQPMat::objective_variant_t & Jerk = Matrices.Objective( IntermedQPMat::JERK );
  initializeMatrices( Jerk );

}


void
GeneratorVelRef::initializeMatrices( IntermedQPMat::objective_variant_t & Objective)
{

  MAL_MATRIX_RESIZE(Objective.U,m_N,m_N);
  MAL_MATRIX_RESIZE(Objective.UT,m_N,m_N);
  MAL_MATRIX_RESIZE(Objective.S,m_N,3);

  switch(Objective.type)
    {
    case IntermedQPMat::MEAN_VELOCITY || IntermedQPMat::INSTANT_VELOCITY:
      for(int i=0;i<m_N;i++)
	{
	  Objective.S(i,0) = 0.0; Objective.S(i,1) = 1.0; Objective.S(i,2) = (i+1)*m_T_Prw;
	  for(int j=0;j<m_N;j++)
            if (j<=i)
              Objective.U(i,j) = Objective.UT(j,i) = (2*(i-j)+1)*m_T_Prw*m_T_Prw*0.5 ;
            else
	      Objective.U(i,j) = Objective.UT(j,i) = 0.0;
	}
      break;

    case IntermedQPMat::COP_CENTERING:
      for(int i=0;i<m_N;i++)
        {
          Objective.S(i,0) = 1.0; Objective.S(i,1) = (i+1)*m_T_Prw; Objective.S(i,2) = (i+1)*(i+1)*m_T_Prw*m_T_Prw*0.5-m_CoMHeight/9.81;
          for(int j=0;j<m_N;j++)
	    if (j<=i)
	      Objective.U(i,j) = Objective.UT(j,i) = (1 + 3*(i-j) + 3*(i-j)*(i-j)) * m_T_Prw*m_T_Prw*m_T_Prw/6.0 - m_T_Prw*m_CoMHeight/9.81;
	    else
	      Objective.U(i,j) = Objective.UT(j,i) = 0.0;
        }
      break;

    case IntermedQPMat::JERK:
      for(int i=0;i<m_N;i++)
        {
          Objective.S(i,0) = 0.0; Objective.S(i,1) = 0.0; Objective.S(i,2) = 0.0;
          for(int j=0;j<m_N;j++)
	    if (j==i)
	      Objective.U(i,j) = Objective.UT(j,i) = 1.0;
	    else
	      Objective.U(i,j) = Objective.UT(j,i) = 0.0;
        }
      break;

    }

}


void 
GeneratorVelRef::addEqConstraint(std::deque<LinearConstraintInequalityFreeFeet_t> ConstraintsDeque,
				 MAL_MATRIX (&DU, double), MAL_MATRIX (&DS, double))
{
  //TODO:
}
	  

void
GeneratorVelRef::addIneqConstraint(std::deque<LinearConstraintInequalityFreeFeet_t> ConstraintsDeque,
				   MAL_MATRIX (&DU, double), MAL_MATRIX (&DS, double))
{
  //TODO:
}
	  

void 
GeneratorVelRef::generateZMPConstraints (CjrlFoot & Foot,
					 std::deque < double > & PreviewedSupportAngles,
					 SupportFSM & FSM,
					 support_state_t & CurrentSupportState,
					 QPProblem & Pb)
{
  //TODO:
}


void 
GeneratorVelRef::generateFeetPosConstraints (CjrlFoot & Foot,
					     std::deque < double > & PreviewedSupportAngles,
					     SupportFSM & ,
					     support_state_t &,
					     QPProblem & Pb)
{
  //TODO:
}


void 
GeneratorVelRef::buildInvariantPart(QPProblem & Pb, IntermedQPMat & Matrices)
{

  MAL_MATRIX(weightMTM,double);

  const IntermedQPMat::objective_variant_t & Jerk = Matrices.Objective(IntermedQPMat::JERK);
  // Final scaled products that are added to the QP
  computeTerm(weightMTM, Jerk.weight, Jerk.UT, Jerk.U);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 0, 0);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, m_N, m_N);

  const IntermedQPMat::objective_variant_t & InstVel = Matrices.Objective(IntermedQPMat::INSTANT_VELOCITY);
  computeTerm(weightMTM, InstVel.weight, InstVel.UT, InstVel.U);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 0, 0);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, m_N, m_N);

  const IntermedQPMat::objective_variant_t & COPCent = Matrices.Objective(IntermedQPMat::COP_CENTERING);
  computeTerm(weightMTM, COPCent.weight, COPCent.UT, COPCent.U);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 0, 0);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, m_N, m_N);

}


void
GeneratorVelRef::updateProblem(QPProblem & Pb, IntermedQPMat & Matrices,
			       std::deque<support_state_t> deqSupportStates)
{

  //Intermediate vector
  MAL_VECTOR(MV,double);

  // Final scaled products that are added to the QP
  MAL_MATRIX(weightMTM,double);
  MAL_VECTOR(weightMTV,double);

  const IntermedQPMat::state_variant_t & State = Matrices.State();

  const IntermedQPMat::objective_variant_t & InstVel = Matrices.Objective(IntermedQPMat::INSTANT_VELOCITY);
  computeTerm(weightMTV, InstVel.weight, InstVel.UT, MV, InstVel.S, State.CoM.x);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, 0);
  computeTerm(weightMTV, InstVel.weight, InstVel.UT, MV, InstVel.S, State.CoM.y);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, m_N);
  computeTerm(weightMTV, -InstVel.weight, InstVel.UT, State.Ref.global.X);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, 0);
  computeTerm(weightMTV, -InstVel.weight, InstVel.UT, State.Ref.global.Y);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, m_N);

  const IntermedQPMat::objective_variant_t & COPCent = Matrices.Objective(IntermedQPMat::COP_CENTERING);
  computeTerm(weightMTM, -COPCent.weight, COPCent.UT, State.V);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 0, 2*m_N);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, m_N, 2*m_N+deqSupportStates[m_N].StepNumber);
  computeTerm(weightMTM, -COPCent.weight, State.VT, COPCent.U);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 2*m_N, 0);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 2*m_N+deqSupportStates[m_N].StepNumber, m_N);
  computeTerm(weightMTM, COPCent.weight, State.VT, State.V);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 2*m_N, 2*m_N);
  Pb.addTerm(weightMTM, QPProblem::MATRIX_Q, 2*m_N+deqSupportStates[m_N].StepNumber, 2*m_N+deqSupportStates[m_N].StepNumber);

  // Linear part of the objective
  computeTerm(weightMTV, -COPCent.weight, State.VT, MV, COPCent.S, State.CoM.x);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, 2*m_N);
  computeTerm(weightMTV, -COPCent.weight, State.VT, MV, COPCent.S, State.CoM.y);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, 2*m_N+deqSupportStates[m_N].StepNumber);
  computeTerm(weightMTV, COPCent.weight, COPCent.UT, State.Vc, State.SupportFoot.x);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, 2*m_N);
  computeTerm(weightMTV, COPCent.weight, COPCent.UT, State.Vc, State.SupportFoot.y);
  Pb.addTerm(weightMTV, QPProblem::VECTOR_D, 2*m_N+deqSupportStates[m_N].StepNumber);

}


void
GeneratorVelRef::computeTerm(MAL_MATRIX (&weightMM, double), const double & weight,
			     const MAL_MATRIX (&M1, double), const MAL_MATRIX (&M2, double))
{
  weightMM = weight*MAL_RET_A_by_B(M1,M2);
}


void
GeneratorVelRef::computeTerm(MAL_VECTOR (&weightMV, double), const double weight,
			     const MAL_MATRIX (&M, double), const MAL_VECTOR (&V, double))
{
  weightMV = weight*MAL_RET_A_by_B(M,V);
}


void
GeneratorVelRef::computeTerm(MAL_VECTOR (&weightMV, double),
			     const double weight, const MAL_MATRIX (&M, double),
			     const MAL_VECTOR (&V, double), const double scalar)
{
  weightMV = weight*scalar*MAL_RET_A_by_B(M,V);
}


void
GeneratorVelRef::computeTerm(MAL_VECTOR (&weightMV, double),
			     const double weight, const MAL_MATRIX (&M1, double), MAL_VECTOR (&V1, double),
			     const MAL_MATRIX (&M2, double), const MAL_VECTOR (&V2, double))
{
  V1 = MAL_RET_A_by_B(M2,V2);
  weightMV = weight*MAL_RET_A_by_B(M1,V1);
}


void
GeneratorVelRef::addTerm(MAL_MATRIX(&Mat, double), double * target, int row, int col, int nrows, int ncols)
{
  for(int i = 0;i < nrows; i++)
    {
      for(int j = 0;j < ncols; j++)
	{
	  target[row+i+(col+j)*m_NbVariables] = Mat(i,j);
	}
    }
}


void
GeneratorVelRef::addTerm(MAL_VECTOR (&Vec, double), double * target, int index, int nelem)
{
  for(int i = index;i < nelem; i++)
    target[i] = Vec(i);
}


int 
GeneratorVelRef::buildConstantPartOfConstraintMatrices()
{
  //TODO:
}

	
int 
GeneratorVelRef::buildConstantPartOfTheObjectiveFunction()
{
  //TODO:
}


int 
GeneratorVelRef::initializeMatrixPbConstants()
{
  //TODO:
}

	  
int 
GeneratorVelRef::dumpProblem(MAL_VECTOR(& xk,double),
			     double Time)
{
  //TODO:
}
