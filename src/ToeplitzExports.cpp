#include <Rcpp.h>
using namespace Rcpp;
#include "SuperGauss/Toeplitz.h"

// R wapper functions to Toeplitz methods using XPtr

// [[Rcpp::export]]
SEXP Toeplitz_ctor(int n) {
  Toeplitz *Toep = new Toeplitz(n);
  XPtr<Toeplitz> Toep_ptr(Toep, true);
  return Toep_ptr;
}

// [[Rcpp::export]]
void Toeplitz_set_acf(SEXP Toep_ptr, NumericVector acf) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  Toep->set_acf(REAL(acf));
  return;
}

// [[Rcpp::export]]
NumericVector Toeplitz_get_acf(SEXP Toep_ptr) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  NumericVector acf(Toep->size());
  Toep->get_acf(REAL(acf));
  return acf;
}

// //[[Rcpp::export(".Toeplitz_getPhi")]]
// NumericVector Toeplitz_getPhi(SEXP Toep_ptr) {
//   XPtr<Toeplitz> Toep(Toep_ptr);
//   NumericVector phi(Toep->size());
//   Toep->getPhi(REAL(phi));
//   return phi;
// }

// [[Rcpp::export]]
NumericMatrix Toeplitz_prod(SEXP Toep_ptr, NumericMatrix X) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  int p = X.ncol();
  int n = X.nrow();
  NumericMatrix Y(n,p);
  for(int ii=0; ii<p; ii++) {
    Toep->prod(&REAL(Y)[n*ii], &REAL(X)[n*ii]);
  }
  return Y;
}

// [[Rcpp::export]]
NumericMatrix Toeplitz_solve(SEXP Toep_ptr, NumericMatrix X) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  int p = X.ncol();
  int n = X.nrow();
  NumericMatrix Y(n,p);
  for(int ii=0; ii<p; ii++) {
    Toep->solve(&REAL(Y)[n*ii], &REAL(X)[n*ii]);
  }
  return Y;
}

// [[Rcpp::export]]
double Toeplitz_log_det(SEXP Toep_ptr) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  return Toep->log_det();
}

// [[Rcpp::export]]
double Toeplitz_trace_grad(SEXP Toep_ptr, NumericVector acf2) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  return Toep->trace_grad(REAL(acf2));
}

// [[Rcpp::export]]
double Toeplitz_trace_hess(SEXP Toep_ptr,
			   NumericVector acf2, NumericVector acf3) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  return Toep->trace_hess(REAL(acf2), REAL(acf3));
}

// [[Rcpp::export]]
bool Toeplitz_has_acf(SEXP Toep_ptr) {
  XPtr<Toeplitz> Toep(Toep_ptr);
  return Toep->has_acf();
}

