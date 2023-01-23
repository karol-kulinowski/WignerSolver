#ifndef LIB_H
#define LIB_H

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <ctime>
#include <map>

#include <omp.h>

#define ARMA_USE_SUPERLU 1
#define ARMA_OPENMP_THREADS 2
#define ARMA_PRINT_ERRORS 1
#define ARMA_WARN_LEVEL 3

#define OMP_NUM_THREADS 2

#include <armadillo>

// using namespace std;
using std::cout;
using std::endl;
// using namespace arma;

// ############################## TYPEDEFs ##############################
// TODO: put into a namespace
// typedef std::vector<double>::size_type size_t_vec_d;

// ############################## GLOBAL VARIABLES ##############################
// #define N_THREADS 4

// ############################## CONSTANTS ##############################

double const PI {M_PI};
double const KB {8.617333262145179E-05};  // Boltzmann constant [eV/K]
double const E0 {1.602176634E-19};  // Elementary charge [C]
double const HBAR_J {1.0545718176461565E-34};  // Reduced Planck constant [Js]
double const HBAR_eV {6.582119569509067E-16};  // Reduced Planck constant [eVs]
double const M0 {9.1093837015E-31};  // Electron mass [kg]
double const EPS0 {8.8541878128E-12};  // Electric constant
double const A_GaAs {0.565};  // GaAs lattice constant [nm]

// ############################## atomic units ##############################

double const AU_nm {0.0529177210903};  // Bohr radius [nm]
double const AU_m {0.0529177210903E-9};  // Bohr radius [m]
double const AU_eV {27.211386245988};  // Hartree energy [eV]
double const AU_s {2.4188843265857225E-17};  // Time [ps = 10^-12 s] (HBAR_eV/AU_eV*1e12)
double const AU_A {6.623618237509881E-3};  // [A]  _e0/_tau0
double const AU_Acm2 {2.365337010944052E14};  // [A/cm**2]  AU_A/AU_nm/AU_nm
double const AU_cm3 {1.481847114721628E-25};  // [cm**3]  (AU_nm*1e-7)**3
double const AU_cm2 {2.800285205390781E-17};  // [cm**2]  (AU_nm*1e-7)**2
double const AU_cm {5.2917721090299995E-09};  // [cm]  AU_nm*1e-7

// #################### klasa array ####################
template <class T>
class array {
	private:
		std::vector<T> a;
		size_t n_row;
	public:
		array() {}
		array(size_t n) : a (std::vector<T>(n)), n_row (n) {}
		array(size_t n, T v) : a (std::vector<T>(n, v)), n_row (n) {}
		~array() { a.clear(); }
		T& operator () (size_t i) { return a.at(i); }
		// Vector addition
		array<T> operator-(array<T> ai) {
			array<T> af(n_row);
			for (size_t i=0; i<n_row; ++i) af.at(i) = a.at(i)-ai.at(i);
			return af;
		}
		// Vector substraction
		array<T> operator+(array<T> ai) {
			array<T> af(n_row);
			for (size_t i=0; i<n_row; ++i) af.at(i) = a.at(i)+ai.at(i);
			return af;
		}
		// Vector multiplication
		array<T> operator*(array<T> ai) {
			array<T> af(n_row);
			for (size_t i=0; i<n_row; ++i) af.at(i) = a.at(i)*ai.at(i);
			return af;
		}
		// Vector division
		array<T> operator/(array<T> ai) {
			array<T> af(n_row);
			for (size_t i=0; i<n_row; ++i) af.at(i) = a.at(i)/ai.at(i);
			return af;
		}
		// Multiplying vector by scalar
		// array<T> operator*(Q s) {
		//   array<T> af(n_row);
		//   for (size_t i=0; i<n_row; ++i) af.at(i) = s*a.at(i);
		//   return af;
		// }
		T& at(size_t i) { return a.at(i); }
		size_t size() { return a.size(); }
		void add(T x) { a.push_back(x); }
		void copy(array<T> ac) { for (size_t i=0; i<n_row; ++i) a.at(i) = ac(i); }
		void zeros() { for (size_t i=0; i<n_row; ++i) a.at(i) = 0.; }
		double max() { return *max_element(a.begin(), a.end()); }
		double min() { return *min_element(a.begin(), a.end()); }
		double sum() {
			double sum = 0;
			for (size_t i=0; i<n_row; ++i) sum += a.at(i);
			return sum;
		}
};

// #################### klasa matrix ####################
template <class T>
class matrix {
	private:
		array< array<T> > m;
		size_t n_row, n_col;
	public:
		matrix() {}
		matrix(size_t n1, size_t n2) : n_row (n1), n_col (n2)
				{ for (size_t i=0; i<n_row; ++i) m.add( array<T>(n_col) ); }
		~matrix() {}
		array<T>& operator () (size_t i) { return m.at(i); }
		T& operator () (size_t i, size_t j) { return m.at(i).at(j); }
		array<T>& at(size_t i) { return m.at(i); }
		matrix<T> operator-(matrix<T> mi) {
			matrix<T> mf(n_row, n_col);
			for (size_t i=0; i<n_row; ++i)
				for (size_t j=0; j<n_col; ++j)
					mf.at(i) = m.at(i)-mi.at(i);
			return mf;
		}
		matrix<T> operator+(matrix<T> mi) {
			matrix<T> mf(n_row, n_col);
			for (size_t i=0; i<n_row; ++i)
				for (size_t j=0; j<n_col; ++j)
					mf.at(i) = m.at(i)+mi.at(i);
			return mf;
		}
		T& at(size_t i, size_t j) { return m.at(i).at(j); }
		size_t size() { return n_col*n_row; }
		void copy(matrix<T> mc) { for (size_t i=0; i<n_row; ++i) m(i).copy(mc(i)); }
		// void add(T x){ m.push_back(x); }
		void zeros() { for (size_t i=0; i<n_row; ++i) m(i).zeros(); }
};


//
// Funkcje do liczenia całek i pochodnych
// Definicje tych funkcji znajdują się w pliku wignerTools.cpp
//


// #################### funkcja do liczenia całki ####################
template <class T>
double calcInt(arma::vec f, T h){
	size_t n = f.size();
	T ig = 0;
	for (size_t i=1; i<n/2; ++i)
		// ig += (f(i-1) + f(i))*h/2.;  // trapezoid
		ig += (f(2*i-2)+4*f(2*i-1)+f(2*i))*h/3.;  // simpson
	return ig;
}


// #################### funkcja do liczenia pierwszej pochodnej ####################
template <class T>
arma::vec calcFirstDer(arma::vec f, T h){
	size_t n = f.size();
	arma::vec df(n, arma::fill::zeros);
	for (size_t i=2; i<n-2; ++i)
		df(i) = (1/12.*f(i-2)-2/3.*f(i-1)+2/3.*f(i+1)-1/12.*f(i+2))/h;
		// df(i) = (-f(i-1)+f(i+1))/h/2.;
	df(0) = (-3.*f(0)+4.*f(1)-f(2))/h/2.;
	df(n-1) = (3.*f(n-1)-4.*f(n-2)+f(n-3))/h/2.;
	df(1) = (-f(0)+f(2))/h/2.;
	df(n-2) = (-f(n-3)+f(n-1))/h/2.;
	// df(0) = (-f(0)+f(1))/h;
	// df(n-1) = (f(n-1)-f(n-2))/h;
	// df(0) = df(1);
	// df(n-1) = df(n-2);
	return df;
}


// #################### funkcja do liczenia drugiej pochodnej ####################
template <class T>
arma::vec calcSecondDer(arma::vec f, T h){
	size_t n = f.size();
	arma::vec df(n, arma::fill::zeros);
	for (size_t i=1; i<n-1; ++i)
		df(i) = (f(i-1)-2.*f(i)+f(i+1))/h/h;
	df(0) = (2*f(0)-5.*f(1)+4*f(2)-f(3))/h/h;
	df(n-1) = (2*f(n-1)-5.*f(n-2)+4*f(n-3)-f(n-4))/h/h;
	return df;
}


// #################### funkcja do liczenia trzeciej pochodnej ####################
template <class T>
arma::vec calcThirdDer(arma::vec f, T h){
	size_t n = f.size();
	arma::vec df(n, arma::fill::zeros);
	for (size_t i=2; i<n-2; ++i)
		df(i) = (-f(i-2)/2.+f(i-1)-f(i+1)+f(i+2)/2.)/h/h/h;
	df(0) = (-f(0)+3.*f(1)-3*f(2)+f(3))/h/h/h;
	df(1) = (-f(1)+3.*f(2)-3*f(3)+f(4))/h/h/h;
	df(n-1) = (f(n-1)-3.*f(n-2)+3.*f(n-3)-f(n-4))/h/h/h;
	df(n-2) = (f(n-2)-3.*f(n-3)+3.*f(n-4)-f(n-5))/h/h/h;
	return df;
}


double calcFermiEn(double, double, double);
std::map<std::string, double> readParam(std::string);


#endif
