#include <stdio.h>
#include <math.h>
#define M_PAM 4
#define N_GH 10

// Gauss-Hermite parameters
const double x[N_GH] = {-3.436159118837737603327,
-2.532731674232789796409,
-1.756683649299881773451,
-1.036610829789513654178,
-0.3429013272237046087892,
0.3429013272237046087892,
1.036610829789513654178,
1.756683649299881773451,
2.532731674232789796409,	
3.436159118837737603327};	
const double w[N_GH] = {7.64043285523262062916E-6,
0.001343645746781232692202,
0.0338743944554810631362,
0.2401386110823146864165,
0.6108626337353257987836,
0.6108626337353257987836,
0.2401386110823146864165,
0.03387439445548106313617,
0.001343645746781232692202,
7.64043285523262062916E-6};

double symbol_energy(const double *C, const double *Pk, int M);
unsigned int insert_zero(unsigned int i, unsigned int k, unsigned int nb);
double pam_eval_mi(const double *C, int M, double s, const double *Pk);
double pam_eval_gmi(const double *C, int M, double s);

int main(int argc, char *argv[])
{
	// Initialize MI and GMI	
	double MI, GMI;
	
	// Constellation (natural bit mapping order)
	double C[M_PAM] = {-3.0, -1.0, 3.0, 1.0};
	double Pk[M_PAM] = {0.25, 0.25, 0.25, 0.25};

	// Signal-to-noise ratio (dB)
	double snr = 10;
	double Es = symbol_energy(C, Pk, M_PAM);
	double  s = sqrt(Es)*pow(10.0,-snr/20.0);
	
	// Calculate
	GMI = pam_eval_gmi(C, M_PAM, s);
	MI= pam_eval_mi(C, M_PAM, s, Pk);
	
	// Print results
	printf("MI=%f, GMI=%f\n",MI,GMI);	

	return 0;
}

// Helper function to calculate symbol energy
double symbol_energy(const double *C, const double *Pk, int M)
{
	int i;
	double Es = 0.0;
	
	for(i=0;i<M;i++)
	{
		Es = Es + Pk[i]*pow(C[i],2.0);
	}	
	
	return Es;
}

// Helper function to insert a zero inside a number
unsigned int insert_zero(unsigned int i, unsigned int k, unsigned int nb)
{
  unsigned int b0, left, right;
  
  left = (i<<1) & (  ( (1<<(nb-k)) - 1)<<(k+1) );
  right = i & ((1<<k)-1);
  b0 = left | right;
  
  return b0;
}

// Calculate AWGN mutual information for PAM
double pam_eval_mi(const double *C, int M, double s, const double *Pk)
{
  double MI = 0.0;
  double tmp;
  int i, l, j;

  // Cycle through constellation point
  for(i=0; i<M; i++)
  {
     // Cycle through Gauss-Hermite parameters
     for(l=0; l<N_GH; l++)
     {
       tmp = 0.0;
       
       // Cycle through constellation point for the logarithm
       for(j=0; j<M; j++)
       {
         tmp += Pk[j]*exp(-(pow(C[j]-C[i],2.0) - sqrt(8.0)*x[l]*s*(C[j]-C[i]))/(2*pow(s,2.0)));
       }
       
       MI -= Pk[i]*w[l]*log2(tmp);
     }
  }
  
  MI /= sqrt(M_PI);  
  return MI;
}

// Calculate BICM mutual information (GMI) for equiprobable PAM
double pam_eval_gmi(const double *C, int M, double s)
{
  const unsigned int m = log2(M);
  
  unsigned int i, l, j, k, b;
  unsigned int bi, bj;
  double GMI = m;
  double tmp_num, tmp_den;

  // Cycle through constellation bit
  for(k=0; k<m; k++)
  {  
  // Bit can be either 0 or 1
    for(b=0; b<=1; b++)
    {
      // Constellation points where k-th bit is equal to b
      for(i=0; i<M/2; i++)
      {
        bi = insert_zero(i, k, m) + (b<<k);
        
        // Cycle through Gauss-Hermite parameters
        for(l=0; l<N_GH; l++)
        {
          // Initialize numerator and denominator of the logarithm
          tmp_num = 0.0;
          tmp_den = 0.0;
       
          // Numerator of the logarithm
          for(j=0; j<M; j++)
          {
            tmp_num += exp(-(pow(C[bi]-C[j],2.0) - sqrt(8.0)*x[l]*s*(C[bi]-C[j]))/(2*pow(s,2.0)));
          }
        
          // Denominator of the logarithm
          for(j=0; j<M/2; j++)
          {
            bj = insert_zero(j, k, m) + (b<<k);
            tmp_den += exp(-(pow(C[bi]-C[bj],2.0) - sqrt(8.0)*x[l]*s*(C[bi]-C[bj]))/(2*pow(s,2.0)));
          }
       
          // Evaluate GMI
          GMI -= w[l]*log2(tmp_num/tmp_den)/(M*sqrt(M_PI));
        } 
      }
    }
  }
  
  return GMI;
}

