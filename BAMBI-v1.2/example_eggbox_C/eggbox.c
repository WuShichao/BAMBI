#include "eggbox.h"


/******************************************** getphysparams routine ****************************************************/

void getphysparams(double *Cube, int *ndim)
{
	int i;
	for(i = 0; i < *ndim; i++) Cube[i] = Cube[i]*10.0*M_PI;
	//for(int i = 0; i < *ndim; i++) Cube[i] = 2.0*M_PI+(Cube[i]-0.5)*M_PI*0.5;
}

/******************************************** getallparams routine ****************************************************/

void getallparams(double *Cube, int *ndim)
{
	getphysparams(Cube,ndim);
}

/******************************************** loglikelihood routine ****************************************************/

// Now an example, sample an egg box likelihood

// Input arguments
// ndim 						= dimensionality (total number of free parameters) of the problem
// npars 						= total number of free plus derived parameters
//
// Input/Output arguments
// Cube[npars] 						= on entry has the ndim parameters in unit-hypercube
//	 						on exit, the physical parameters plus copy any derived parameters you want to store with the free parameters
//	 
// Output arguments
// lnew 						= loglikelihood

void getLogLike(double *Cube, int *ndim, int *npars, double *lnew, void *context)
{
	double chi = 1.0;
	int i;
	getallparams(Cube,ndim);
	for(i = 0; i < *ndim; i++)
	{
		chi *= cos(Cube[i]/2.0);
	}
	*lnew = powf(chi + 2.0, 5.0);
}


/************************************************* dumper routine ******************************************************/

// The dumper routine will be called every updInt*10 iterations
// MultiNest doesn not need to the user to do anything. User can use the arguments in whichever way he/she wants
//
//
// Arguments:
//
// nSamples 						= total number of samples in posterior distribution
// nlive 						= total number of live points
// nPar 						= total number of parameters (free + derived)
// physLive[1][nlive * (nPar + 1)] 			= 2D array containing the last set of live points (physical parameters plus derived parameters) along with their loglikelihood values
// posterior[1][nSamples * (nPar + 2)] 			= posterior distribution containing nSamples points. Each sample has nPar parameters (physical + derived) along with the their loglike value & posterior probability
// paramConstr[1][4*nPar]:
// paramConstr[0][0] to paramConstr[0][nPar - 1] 	= mean values of the parameters
// paramConstr[0][nPar] to paramConstr[0][2*nPar - 1] 	= standard deviation of the parameters
// paramConstr[0][nPar*2] to paramConstr[0][3*nPar - 1] = best-fit (maxlike) parameters
// paramConstr[0][nPar*4] to paramConstr[0][4*nPar - 1] = MAP (maximum-a-posteriori) parameters
// maxLogLike						= maximum loglikelihood value
// logZ							= log evidence value
// logZerr						= error on log evidence value
// context						void pointer, any additional information

void dumper(int *nSamples, int *nlive, int *nPar, double **physLive, double **posterior, double **paramConstr, double *maxLogLike, double *logZ, double *logZerr, void *context)
{
	// convert the 2D Fortran arrays to C arrays
	
	
	// the posterior distribution
	// postdist will have nPar parameters in the first nPar columns & loglike value & the posterior probability in the last two columns
	
	int i, j;
	
	double postdist[*nSamples][*nPar + 2];
	for( i = 0; i < *nPar + 2; i++ )
		for( j = 0; j < *nSamples; j++ )
			postdist[j][i] = posterior[0][i * (*nSamples) + j];
	
	
	
	// last set of live points
	// pLivePts will have nPar parameters in the first nPar columns & loglike value in the last column
	
	double pLivePts[*nlive][*nPar + 1];
	for( i = 0; i < *nPar + 1; i++ )
		for( j = 0; j < *nlive; j++ )
			pLivePts[j][i] = physLive[0][i * (*nlive) + j];
}

/***********************************************************************************************************************/




/************************************************** Main program *******************************************************/



int main(int argc, char *argv[])
{
#ifdef PARALLEL
 	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
#endif
	
	int i;
	
	// set the MultiNest sampling parameters
	
	int mmodal = 0;					// do mode separation?
	
	int ceff = 0;					// run in constant efficiency mode?
	
	int nlive = 4000;				// number of live points
	
	double efr = 0.8;				// set the required efficiency
	
	double tol = 0.5;				// tol, defines the stopping criteria
	
	int ndims = 2;					// dimensionality (no. of free parameters)
	
	int nPar = 2;					// total no. of parameters including free & derived parameters
	
	int nClsPar = 2;				// no. of parameters to do mode separation on
	
	int updInt = 4000;				// after how many iterations feedback is required & the output files should be updated
							// note: posterior files are updated & dumper routine is called after every updInt*10 iterations
	
	double Ztol = -1E90;				// all the modes with logZ < Ztol are ignored
	
	int maxModes = 1;				// expected max no. of modes (used only for memory allocation)
	
	int pWrap[ndims];				// which parameters to have periodic boundary conditions?
	for(i = 0; i < ndims; i++) pWrap[i] = 0;
	
	strcpy(root, "chains/eggboxC_");			// root for output files
	strcpy(networkinputs, "example_eggbox_C/eggbox_net.inp");			// file with input parameters for network training
	
	int seed = -1;					// random no. generator seed, if < 0 then take the seed from system clock
	
	int fb = 1;					// need feedback on standard output?
	
	resume = 0;					// resume from a previous job?
	
	int outfile = 1;				// write output files?
	
	int initMPI = 0;				// initialize MPI routines?, relevant only if compiling with MPI
							// set it to F if you want your main program to handle MPI initialization
	
	logZero = -1E90;				// points with loglike < logZero will be ignored by MultiNest
	
	int maxiter = 0;				// max no. of iterations, a non-positive value means infinity. MultiNest will terminate if either it 
							// has done max no. of iterations or convergence criterion (defined through tol) has been satisfied
	
	void *context = 0;				// not required by MultiNest, any additional information user wants to pass
	
	doBAMBI = 1;					// BAMBI?

	useNN = 0;
	
	// calling MultiNest

	BAMBIrun(mmodal, ceff, nlive, tol, efr, ndims, nPar, nClsPar, maxModes, updInt, Ztol, root, seed, pWrap, fb, resume, outfile, initMPI,
	logZero, maxiter, LogLikeFctn, dumper, BAMBIfctn, context);
	
#ifdef PARALLEL
 	MPI_Finalize();
#endif
}

/***********************************************************************************************************************/
