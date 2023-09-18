#include "src/lib.hpp"
#include "src/WignerFunction.hpp"
#include "src/poisson1D.hpp"

#include <chrono>


using namespace wigner;
using namespace poisson;
using namespace std::chrono;


void dissDecoh(WignerFunction&, double, double, size_t, double, double, size_t);
void calcTimeChar(WignerFunction&);
void gaussBarrier(WignerFunction&);


int main(){

	// TODO: ALL OUTPUT AND INPUT TO .CSV FORMAT, USE SAVE/LOAD FUNCTIONS

	// omp_set_dynamic(0);
	// omp_set_num_threads(1);
	// cout<<"omp_get_max_threads(): "<<omp_get_max_threads() <<endl;
	// cout<<"omp_get_num_threads(): "<<omp_get_num_threads()<<endl;

	size_t nx = 200, nk = 200;
	double lD = 1000, lC = 0/AU_nm;
	double k_max = 0.05;  // -1, 0.15

	WignerFunction f(nx, lD, lC, nk, k_max);

	arma::vec x_val = f.get_x_arr(), k_val = f.get_k_arr();
	// double dx = f.get_dx(), dk = f.get_dk();

	// constants are given in src/lib.hpp
	f.set_m(M_GaAs);
	f.set_temp(TEMP);
	f.set_epsilonR(EPS_GaAs);
	f.set_cD(ND*AU_cm3);
	f.set_uL( calcFermiEn(f.get_cD(), f.get_m(), f.get_temp()) );
	f.set_uR( calcFermiEn(f.get_cD(), f.get_m(), f.get_temp()) ); // calcFermiEn(f.get_cD(), f.get_m(), f.get_temp())
	f.set_dt(0.1e-15/AU_s);  // .005*1e-15/AU_s

	f.set_rR(0), f.set_rM(0); // 1./(1e-12/AU_s)  // Dissipation terms
	f.set_rG(0), f.set_rF(0), f.set_lambda(0);

	f.set_useQC(false);  // Quantum correction term (third 'p' derivative)?
	f.set_useNLP(false);  // Calculations with non-local potential?
	f.set_uBias_BC(false);  // Voltage bias given through BC?

	//
	// "CDS1": 0, "UDS1": 1, "UDS2": 2, "UDS3": 3, "HDS22": 3
	f.set_diffSch_K("UDS2");
	f.set_diffSch_P("UDS2");
	f.set_diffSch_J("UDS2");

	// Warunek brzegowy
	// 0 -> 0, 1 -> SF, 2:4 -> splot with SF, -1 -> Gauss, -2:-4 splot with Gauss
	cout<<"# Setting up BC"<<endl;
	f.set_bcType(1);

	//
	// Potential
	// cout<<"# Setting up potential"<<endl;
	f.set_uBias(0.0/AU_eV);
	// f.setPotBias(0.1/AU_eV);
	//
	// f.addRectBarr(0.3/AU_eV, 300, 100, 10);
	// f.addRectBarr(0.3/AU_eV, 700, 100, 10);
	//
	// f.addRectBarr(0.3/AU_eV, 1750/AU_nm, 200/AU_nm, 10);
	// f.addRectBarr(0.3/AU_eV, 2250/AU_nm, 200/AU_nm, 10);
	// f.addGaussBarr(0.3/AU_eV, 500/AU_nm, 100/AU_nm);
	//
	// f.load_poisson_pot("poisson_pot_100meV_4e4it.bin");
	// f.set_uC(f.get_uStart());
	//
	// Read potential from poisson_test
	// mat a; a.load("IV_char/out_data_10meV_2e5it/poisson_test.csv", csv_ascii);
	// f.set_uC(a.col(1)/AU_eV);
	//
	// Barriera gaussowska
	// f.addGaussBarr(0.3/AU_eV, 500, 100);
	// @TODO: Setting up potential in separate function


	//
	// FUNKCJA RÓWNOWAGOWA
	// f.setEquilibriumFunction("out_data/wf_feq_BP.bin", true);

	f.printParam();

	high_resolution_clock::time_point t_start, t_end;
	duration<double> t_elapsed;
	t_start = high_resolution_clock::now();

	//
	// Boltzmann
	// cout<<"# Solving BTE"<<endl;
	// f.solveWignerEq();
	// f.saveWignerFun();

	// Wave packet time evolution
	// f.addWavePacket(500/AU_nm, 100/AU_nm, 0.05, 0.005);  // sqrt(2*f.get_m()*f.get_uL())
	// f.addWavePacket(3500/AU_nm, 100/AU_nm, -0.05, 0.005);  // sqrt(2*f.get_m()*f.get_uL())
	// double t_total = 1000e-15/AU_s, t = 0, dt = f.get_dt();
	// while (t <= t_total) {
	// 	t += dt;
	// 	f.solveTimeEv();
	// 	f.saveWignerFun();
	// 	cout<<t*AU_s*1e15<<' '<<f.calcEK()<<' '<<sqrt(f.calcEK2())<<' '<<f.calcEX()*AU_nm<<endl;
	// }

	//
	// Boltzmann-Poisson
	// cout<<"# Solving B-P set of equations"<<endl;
	// (uBias, alpha, beta, n_max, timeDependent)
	// alpha - density mixing, beta - potential mixing
    // f.solveWignerPoisson(0.1/AU_eV, 1E-3, 1, 1, false);
	// f.saveWignerFun();

	//
	// Schrödinger equation
	cout<<"# Solving Schrödinger equation"<<endl;
	f.solveSchrEq();

	//
	// Results
	//
	f.saveTest();
	f.printResults();

	//
	// Calculation time
	//
	t_end = high_resolution_clock::now();
	t_elapsed =  duration_cast<duration<double>>(t_end - t_start);
	if (t_elapsed.count() < 60)
		cout<<"# RUN TIME: "<<t_elapsed.count()<<" s, "<<endl;
	else
		cout<<"# RUN TIME: "<<t_elapsed.count()/60.<<" min, "<<endl;

	return 0;

}
