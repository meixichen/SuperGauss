/// @file Toeplitz.h

///////////////////////////////////////////////
// Toeplitz matrix class
///////////////////////////////////////////////

#ifndef NormalToeplitz_h
#define NormalToeplitz_h 1

#include "Toeplitz.h"
# define log2Pi 1.83787706641L // log of 2 PI value


/// Function for vector multiplication
///
/// @param[in] v1 First input vector.
/// @param[in] v2 Second input vector.
/// @param[in] n Length of input vector.
/// @param[out] double number that equals v1 * v2.
inline double vecProd(double* v1, double* v2, int n) {
  double ans = 0;
  for (int ii = 0; ii < n; ++ii) {
    ans += v1[ii] * v2[ii];
  }
  return ans;
}

/// Class for Computation involving Toeplitz matrix.
///
/// Model: z ~ N_(0, V), where V is a Toeplitz matrix with parameter theta
class NormalToeplitz: public Toeplitz {
  int p; // number of parameters theta
	
  // storages for temporary vectors
  double* vec1;
  double* vec2;
  double* vec3;
  double* vec4;
  double* phi;

public:
  /// Constructor.
  NormalToeplitz(int, int);

  /// Destructor
  ~NormalToeplitz();

  /// Size
  int size();

  /// Dimension
  int dim();

  /// Log-Density.
  double logdens(double* z, double* acf);

  /// Log-likelihood Gradient.
  void grad(double* dldt, double* z, double* dzdt, double* acf, double* dacfdt);

  /// Hessian matrix of Log-likelihood.
  void hess(double* d2ldt, double* z, double* dzdt, double* d2zdt,
	    double* acf, double* dacfdt, double* d2acfdt);

  /// Full Gradient of the Log-Density for Auto-Differentiation Algorithms.
  void grad_full(double* dldz, double* dldacf, double* z, double* acf);

  /*
 /// Overloaded Log-Density.
 double logdens(double* z, Toeplitz* Tz);

 /// Overloaded Log-likelihood Gradient.
 void grad(double* dldt,
 double* z, double* dzdt,
 Toeplitz* Tz, double* dacfdt);

 /// Overloaded function for the Hessian matrix of Log-likelihood.
 void hess(double* d2ldt,
 double* z, double* dzdt, double* d2zdt,
 Toeplitz* Tz, double* dacfdt, double* d2acfdt);

 /// Overloaded function for the Full Gradient of the Log-Density for Auto-Differentiation Algorithms.
 void grad_full(double* dldz, double* dldacf,
 double* z, Toeplitz* Tz);
  */
};

/// Constructor.
///
/// @param N_ length of observation.
/// @param p_ number of parameters.
/// @param hasToep whether Toeplitz object Tz is imported from outside or generated within the class.
inline NormalToeplitz::NormalToeplitz(int N_, int p_) : Toeplitz(N_) {
  p = p_;

  vec1 = new double[ntz_];
  vec2 = new double[ntz_];
  vec3 = new double[ntz_];
  vec4 = new double[ntz_];
  phi = new double[ntz_];	
}

/// Destructor
inline NormalToeplitz::~NormalToeplitz() {
  delete vec1;
  delete vec2;
  delete vec3;
  delete vec4;
  delete phi;

}

inline int NormalToeplitz::size() {
  return ntz_;
}

inline int NormalToeplitz::dim() {
  return p;
}

/// Log-Density.
///
/// @param[in] z length-N vector of observation.
/// @param[in] acf length-N vector of auto-covariance.
/// @param[out] double number of the log-density.
inline double NormalToeplitz::logdens(double* z, double* acf) {
  double ldens = 0;
  setAcf(acf); // Tz = Toeplitz(acf)
  solve(vec1, z); // vec1 = Tz^{-1} * z
  ldens = vecProd(z, vec1, ntz_); // ldens = t(z) * Tz^{-1} * z
  ldens += logDet() + ntz_ * log2Pi;
  ldens *= -0.5;
  return ldens;
}


/// Log-likelihood Gradient.
///
/// @param[out] dldt length-p vector of the gradient.
/// @param[in] z length-N vector of observation.
/// @param[in] dzdt length-N*p vector of observation derivatives, elememts dzdt[0:N + ii*N] is the derivative of z with respect to the ii-th parameter.
/// @param[in] acf length-N vector of auto-covariance.
/// @param[in] dacfdt length-N*p vector of observation derivatives, elememts dacfdt[0:N + ii*N] is the derivative of acf with respect to the ii-th parameter.
inline void NormalToeplitz::grad(double* dldt, double* z, double* dzdt, 
				 double* acf, double* dacfdt) {
  setAcf(acf); // Tz = Toeplitz(acf)
  solve(vec1, z); // vec1 = Tz^{-1} * z
  for (int ii = 0; ii < p; ++ii) {
    product(vec2, vec1, &dacfdt[ii * ntz_]);
    dldt[ii] = vecProd(vec1, vec2, ntz_) / 2;
    dldt[ii] -= vecProd(&dzdt[ii * ntz_], vec1, ntz_);
    dldt[ii] -= trace_deriv(&dacfdt[ii * ntz_]) / 2;
  }
}



/// Hessian matrix of Log-likelihood.
///
/// @param[out] d2ldt length-p*p vector of the Hessian, elememts d2ldt[0:p + ii*p] is the (ii+1)-th column of Hessian matrix.
/// @param[in] z length-N vector of observation.
/// @param[in] dzdt length-N*p vector of observation derivatives, elememts dzdt[0:N + ii*N] is the derivative of z with respect to the ii-th parameter.
/// @param[in] d2zdt length-N*p*p vector of observation derivatives, elememts d2zdt[0:N + (ii*p+jj)*N] is the second derivative of z with respect to the ii-th parameter and then the jj-th parameter.
/// @param[in] acf length-N vector of auto-covariance.
/// @param[in] dacfdt length-N*p vector of observation derivatives, elememts dacfdt[0:N + ii*N] is the derivative of acf with respect to the ii-th parameter.
/// @param[in] d2acfdt length-N*p*p vector of observation derivatives, elememts d2acfdt[0:N + (ii*p+jj)*N] is the second derivative of acf with respect to the ii-th parameter and then the jj-th parameter.
inline void NormalToeplitz::hess(double* d2ldt,
				 double* z, double* dzdt, double* d2zdt,
				 double* acf, double* dacfdt, double* d2acfdt) {
  setAcf(acf);
  solve(vec1, z);
  double ans;
  std::fill(d2ldt, d2ldt + p * p, 0);
  for (int ii = 0; ii < p; ++ii) {
    for (int jj = 0; jj <= ii; ++jj) {
      product(vec4, vec1, &dacfdt[jj * ntz_]);
      product(vec3, vec1, &dacfdt[ii * ntz_]);
      ans = vecProd(&d2zdt[(ii * p + jj) * ntz_], vec1, ntz_);

      solve(vec2, vec4);
      ans -= vecProd(&dzdt[ii * ntz_], vec2, ntz_);
      ans += vecProd(vec3, vec2, ntz_);
      solve(vec2, vec3);
      ans -= vecProd(&dzdt[jj * ntz_], vec2, ntz_);
      solve(vec2, &dzdt[jj * ntz_]);
      ans += vecProd(&dzdt[ii * ntz_], vec2, ntz_);
      ans *= 2;
			
      product(vec2, vec1, &d2acfdt[(ii * p + jj) * ntz_]);
      ans -= vecProd(vec1, vec2, ntz_);
      ans += trace_deriv(&d2acfdt[(ii * p + jj) * ntz_]);
      ans -= trace_hess(&dacfdt[ii * ntz_], &dacfdt[jj * ntz_]);

      d2ldt[ii * p + jj] = -ans / 2;
    }
  }

  if (p > 1) {
    for (int ii = 0; ii < p; ++ii) {
      for (int jj = ii + 1; jj < p; ++jj) {
	d2ldt[ii * p + jj] = d2ldt[jj * p + ii];
      }
    }
  }

}


/// Full Gradient of the Log-Density for Auto-Differentiation Algorithms.
///
/// @param[out] dldz length-N vector of the log-density with respect to the observation z.
/// @param[out] dldacf length-N vector of the log-density with respect to the auto-covariance acf.
/// @param[in] z length-N vector of observation.
/// @param[in] acf length-N vector of auto-covariance.
inline void NormalToeplitz::grad_full(double* dldz, double* dldacf, double* z, double* acf) {
  setAcf(acf);

  // gradient with respect to z
  solve(dldz, z);
  for (int ii = 0; ii < ntz_; ii++) {
    dldz[ii] = -dldz[ii];
  }

  // gradient with respect to acf
  solve(vec1, z);	
  vec2[0] = 1;
  std::fill(vec2 + 1, vec2 + ntz_, 0);
  solve(vec3, vec2);
  double tau1 = vec3[0];
  std::fill(phi, phi + ntz_, 0);
  phi[0] = vec1[0];
  product(dldacf, vec1, phi, vec1); // dldacf = upper.toep(Vz) %*% Vz = ip
  vec4[0] = 0;
  for (int ii = 1; ii < ntz_; ++ii) {
    vec4[ii] = vec3[ntz_ - ii];
  }
  for (int ii = 0; ii < ntz_; ++ii) {
    vec2[ii] = (ntz_ - ii) * vec3[ii];
  } // vec2 = (ntz_:1 * tau)

  phi[0] = vec3[0];
  product(vec1, vec2, phi, vec3); // vec1 = upper.toep(tau) %*% (ntz_:1 * tau) = tr

  for (int ii = 0; ii < ntz_; ++ii) {
    vec2[ii] = (ntz_ - ii) * vec4[ii];
  } // vec2 = (ntz_:1 * tau2)

  phi[0] = vec4[0];
  product(vec3, vec2, phi, vec4); // vec3 = upper.toep(tau2) %*% (ntz_:1 * tau2)

  for (int ii = 0; ii < ntz_; ++ii) {
    vec1[ii] -= vec3[ii];
    vec1[ii] /= tau1;
    dldacf[ii] -= vec1[ii];
  } // vec1 = (vec1 - vec3) / tau[1] = tr, dldacf = ip - tr

  dldacf[0] /= 2;
}


/*

//////////////////////////////// overloaded version //////////////////////////////////////////

/// Overloaded Log-Density.
///
/// @param[in] z length-N vector of observation.
/// @param[in] Tz size-N Toeplitz object with acf inputed.
/// @param[out] double number of the log-density.
inline double NormalToeplitz::logdens(double* z, Toeplitz* Tz) {
double ldens = 0;
// acf is already stored in Tz
Tz->solveVec(vec1, z); // vec1 = Tz^{-1} * z
ldens = vecProd(z, vec1, N_); // ldens = t(z) * Tz^{-1} * z
ldens += Tz->logDet() + N_ * log2Pi;
ldens *= -0.5;
return ldens;
}

/// Overloaded Log-likelihood Gradient.
///
/// @param[out] dldt length-p vector of the gradient.
/// @param[in] z length-N vector of observation.
/// @param[in] dzdt length-N*p vector of observation derivatives, elememts dzdt[0:N + ii*N] is the derivative of z with respect to the ii-th parameter.
/// @param[in] Tz size-N Toeplitz object with acf inputed.
/// @param[in] dacfdt length-N*p vector of observation derivatives, elememts dacfdt[0:N + ii*N] is the derivative of acf with respect to the ii-th parameter.
inline void NormalToeplitz::grad(double* dldt, double* z, double* dzdt,
Toeplitz* Tz, double* dacfdt) {
// acf is already stored in Tz
Tz->solveVec(vec1, z); // vec1 = Tz^{-1} * z
double tmp;
for (int ii = 0; ii < p_; ++ii) {
T2_->setAcf(&dacfdt[ii * N_]);
T2_->multVec(vec2, vec1);
dldt[ii] = vecProd(vec1, vec2, N_) / 2;
dldt[ii] -= vecProd(&dzdt[ii * N_], vec1, N_);
dldt[ii] -= Tz->traceProd(&dacfdt[ii * N_]) / 2;
}
}

/// Overloaded function for the Hessian matrix of Log-likelihood.
///
/// @param[out] d2ldt length-p*p vector of the Hessian, elememts d2ldt[0:p + ii*p] is the (ii+1)-th column of Hessian matrix.
/// @param[in] z length-N vector of observation.
/// @param[in] dzdt length-N*p vector of observation derivatives, elememts dzdt[0:N + ii*N] is the derivative of z with respect to the ii-th parameter.
/// @param[in] d2zdt length-N*p*p vector of observation derivatives, elememts d2zdt[0:N + (ii*p+jj)*N] is the second derivative of z with respect to the ii-th parameter and then the jj-th parameter.
/// @param[in] Tz size-N Toeplitz object with acf inputed.
/// @param[in] dacfdt length-N*p vector of observation derivatives, elememts dacfdt[0:N + ii*N] is the derivative of acf with respect to the ii-th parameter.
/// @param[in] d2acfdt length-N*p*p vector of observation derivatives, elememts d2acfdt[0:N + (ii*p+jj)*N] is the second derivative of acf with respect to the ii-th parameter and then the jj-th parameter.
inline void NormalToeplitz::hess(double* d2ldt,
double* z, double* dzdt, double* d2zdt,
Toeplitz* Tz, double* dacfdt, double* d2acfdt) {
// acf is already stored in Tz
Tz->solveVec(vec1, z);
double ans;
for (int ii = 0; ii < p_; ++ii) {
for (int jj = 0; jj <= ii; ++jj) {
T2_->setAcf(&dacfdt[jj * N_]);
T2_->multVec(vec4, vec1);
T2_->setAcf(&dacfdt[ii * N_]);
T2_->multVec(vec3, vec1);

ans = vecProd(&d2zdt[(ii * p_ + jj) * N_], vec1, N_);
// temp0 = solve(Tz, temp2)
Tz->solveVec(vec2, vec4);
ans -= vecProd(&dzdt[ii * N_], vec2, N_);
ans += vecProd(vec3, vec2, N_);
// temp0 = solve(Tz, temp1)
Tz->solveVec(vec2, vec3);
ans -= vecProd(&dzdt[jj * N_], vec2, N_);
// temp0 = solve(Tz, dZ[,jj])
Tz->solveVec(vec2, &dzdt[jj * N_]);
ans += vecProd(&dzdt[ii * N_], vec2, N_);
ans *= 2;

T2_->setAcf(&d2acfdt[(ii * p_ + jj) * N_]);
T2_->multVec(vec2, vec1);
ans -= vecProd(vec1, vec2, N_);
ans += Tz->traceProd(&d2acfdt[(ii * p_ + jj) * N_]);
ans -= Tz->traceDeriv(&dacfdt[ii * N_], &dacfdt[jj * N_]);

d2ldt[ii * p_ + jj] = -ans / 2;
}
}

if (p_ > 1) {
for (int ii = 0; ii < p_; ++ii) {
for (int jj = ii + 1; jj < p_; ++jj) {
d2ldt[ii * p_ + jj] = d2ldt[jj * p_ + ii];
}
}
}

}

/// Overloaded function for the Full Gradient of the Log-Density for Auto-Differentiation Algorithms.
///
/// @param[out] dldz length-N vector of the log-density with respect to the observation z.
/// @param[out] dldacf length-N vector of the log-density with respect to the auto-covariance acf.
/// @param[in] z length-N vector of observation.
/// @param[in] Tz size-N Toeplitz object with acf inputed.
inline void NormalToeplitz::grad_full(double* dldz, double* dldacf,
double* z, Toeplitz* Tz) {
// acf is already stored in Tz

// gradient with respect to z
Tz->solveVec(dldz, z);
for (int ii = 0; ii < N_; ii++) {
dldz[ii] = -dldz[ii];
}

// gradient with respect to acf
Tz->solveVec(vec1, z); // vec1 = Vz

vec2[0] = 1;
std::fill(vec2 + 1, vec2 + N_, 0); // vec2 = [1,0,0,...,0]
Tz->solveVec(vec3, vec2); // vec3 = tau
double tau1 = vec3[0];

T2_->setAcf(vec1);
T2_->mult0Vec(dldacf, vec1); // dldacf = upper.toep(Vz) %*% Vz = ip

vec4[0] = 0;
for (int ii = 1; ii < N_; ++ii) {
vec4[ii] = vec3[N_ - ii];
} // vec4 = tau2

for (int ii = 0; ii < N_; ++ii) {
vec2[ii] = (N_ - ii) * vec3[ii];
} // vec2 = (N_:1 * tau)

T2_->setAcf(vec3);
T2_->mult0Vec(vec1, vec2); // vec1 = upper.toep(tau) %*% (N_:1 * tau) = tr

for (int ii = 0; ii < N_; ++ii) {
vec2[ii] = (N_ - ii) * vec4[ii];
} // vec2 = (N_:1 * tau2)
T2_->setAcf(vec4);
T2_->mult0Vec(vec3, vec2); // vec3 = upper.toep(tau2) %*% (N_:1 * tau2)

for (int ii = 0; ii < N_; ++ii) {
vec1[ii] -= vec3[ii];
vec1[ii] /= tau1;
dldacf[ii] -= vec1[ii];
} // vec1 = (vec1 - vec3) / tau[1] = tr
// dldacf = ip - tr

dldacf[0] /= 2;
}

*/

#endif
