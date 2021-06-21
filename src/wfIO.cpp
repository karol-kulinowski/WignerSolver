#include "lib.hpp"
#include "WignerFunction.hpp"

// #define GET_VARIABLE_NAME(Variable) (#Variable)

using namespace wigner;

// ############################## Read parameters from file ##############################

template<typename T>
T ReadPar(std::string str, std::string val_name, std::size_t ln){
	size_t pp, eol;
	T val;
	eol = str.find(';');
	if(eol==std::string::npos) eol = str.size()-1;
	pp = str.substr(0, eol).find(val_name);
	if (pp!=std::string::npos){
			val = std::stod( str.substr(pp+val_name.size(), eol) );
			cout<<"found "+val_name+" at ("<<ln<<','<<pp<<") equal to "<<val<<endl;
			// pp = str.find(val_name, pp+1);
	}
	else val = -1;
	return val;
}


std::map<std::string, double> readParam(std::string filename){
	std::ifstream file (filename);
	std::map<std::string, double> val = {
		{"calc_mode", 0},
		{"contact_temp", 77},
		{"fermi_energy", .00316},  // 0.086 eV
		{"dop_con_left", 0},  // TODO: 0 -> 2e18 in a.u.
		{"dop_con_right", 0},
		{"effective_mass", 0.067},
		{"device_lenght", 3780.72},  // 200 nm
		{"contact_lenght", 0},
		{"max_k", -1},
		{"xspace_step_nr", 100},
		{"kspace_step_nr", 100},
		{"max_voltage", .0147},  // approx. 0.4 V
		{"voltage_step_nr", 40},
		{"courant_num", 1},
		{"gwp_x0", -1},  // 22
		{"gwp_dx", -1},  // 8
		{"gwp_p0", -1},
		{"gwp_dp", -1},
		{"pot_type", 0},
		{"part_num",1000},
		{"inelastic_sc", 0},
		{"elastic_sc", 0},
		{"spatial_decoh", 0},
		{"contact_diss", 0},
		{"contact_dist", 0},
		{"dconc_left", 0},
		{"dconc_right", 0},
		{"v_bias", 0}
	};
	double tmp;
	if (file.is_open()){
		std::string line;
		for (std::pair<std::string, double> ival : val){
			std::size_t ln=0;
			while ( getline (file,line) ){
				ln++;
				if (line[0] != '#'){
					// cout << line << ' ' << line[0] << endl;
					tmp = ReadPar<double>(line, ival.first, ln);
					if (tmp != -1)
							val[ival.first] = tmp;
				}
			}
			file.clear();
			file.seekg (0, std::ios::beg);
			if (val[ival.first] == -1)
					cout<<ival.first+" NOT FOUND, set to default value: "<<val[ival.first]<<endl;
		}
		file.close();
	}

	if (val["max_k"]==-1){
		val["max_k"] = M_PI/2./(val["device_lenght"]/val["xspace_step_nr"]);
		cout<<"max_k NOT FOUND or EQUAL TO -1, is set to "<<val["max_k"]<<" nm^-1"<<endl;
	}
	return val;
}


void WignerFunction::saveWignerFun() {
	// Saves wigner function to a file readable by gnuplot (wf.out),
	// a binary file (wf.bin) that may be loaded to a matrix later on
	// and a *.z format readable by GLE
	std::ofstream wf_out("out_data/wf.out");
	wf_out<<"# x [nm] k [a.u.] f [a.u.]\n";
	for (size_t i=0; i<nx_; ++i){
		for (size_t j=0; j<nk_; ++j)
			wf_out<<x_(i)<<' '<<k_(j)<<' '<<f_(i,j)<<'\n';
		wf_out<<"\n";
	}
	wf_out.close();
	f_.save("out_data/wf.bin");
	wf_out.open("out_data/wf.z", std::ios::out);
	wf_out<<"! nx "<<nx_<<" ny "<<nk_<<" xmin "<<0<<" xmax "<<l_*AU_nm<<" ymin "<<-kmax_<<" ymax "<<kmax_<<'\n';
	for (size_t j=0; j<nk_; ++j){
		for (size_t i=0; i<nx_; ++i)
			wf_out<<f_(i,j)<<' ';
		wf_out<<'\n';
	}
	wf_out.close();
}


void WignerFunction::readPotential(std::string input_file){
	std::ifstream input_pot (input_file);
	arma::vec x(nx_, arma::fill::zeros), u(nx_, arma::fill::zeros);
	size_t j = 0;
	std::string::size_type sz;
	if(!input_pot) {
		cout<<"# COULDN'T OPEN AN POTENTIAL INPUT FILE: "<<input_file<<endl;
		exit(0);
	}
	else {
		cout<<"# READING POTENTIAL FROM FILE: "<<input_file<<endl;
		std::string line;
		while ( getline (input_pot,line) ){
			if (line[0] != '#'){
				try {
					x(j) = std::stod(line, &sz), u(j) = std::stod(line.substr(sz));
					++j;
				} catch (const std::invalid_argument& ia) {
					cout << "## ERROR: EXCEPTION FOUND WHILE READING POTENTIAL FILE; TYPE: " << ia.what() << endl;
					exit(0);
				} catch (const std::logic_error& ie) {
					cout << "## ERROR: EXCEPTION FOUND WHILE READING POTENTIAL FILE; TYPE: " << ie.what() << endl;
					exit(0);
				}
			}
		}
		input_pot.close();
	}
	uStart_.zeros();
	uStart_ = u;
	std::ofstream test ("out_data/input_potential_test.out");
	for (size_t i=0; i<x_.size(); ++i)
		test<<x_(i)<<' '<<uStart_(i)<<'\n';
	test.close();
	// if (x.size() != x_.size()){
	// 	double device_lenght = l_-2*lC_;
	// 	for (size_t p=0; p<x_.size(); ++p){
	// 		if (x_(p)<=x(0))
	// 			uStart_(p) = u(0);
	//
	// 		else if (x_(p)>x(x.size()-1))
	// 			uStart_(p) = u(x.size()-1);
	//
	// 		else {
	// 			for (size_t i=0; i<x.size()-1; ++i)
	// 				if (x_(p)>x(i) && x_(p)<=x(i+1))
	// 					uStart_(p) = u(i+1);
	// 		}
	//
	// 	}
	// 	std::ofstream test ("out_data/potential.out");
	// 	for (size_t i=0; i<x_.size(); ++i)
	// 		test<<x_(i)<<' '<<u_(i)<<'\n';
	// 	test.close();
	// 	if (x(x.size()-1)>device_lenght)
	// 		cout<<"WARNING: Potential exceeds device lenght\n";
	// }
	// else{
	// 	for (size_t i=0; i<x.size(); ++i)
	// 		uStart_(i) = u(i);
	// 	std::ofstream test ("out_data/potential.out");
	// 	for (size_t_vec_d i=0; i<x_.size(); ++i)
	// 		test<<x_(i)<<' '<<uStart_(i)<<'\n';
	// 	test.close();
	// }
}


// #################### Print parameters ####################
void WignerFunction::printParam()
{
	int cw_n = 25, cw_v = 25;

	cout << std::left;

	cout<<endl;
	cout.fill('=');
	cout.width(cw_n+2*cw_v);
	cout<<'#'<<'#'<<endl;

	cout.fill(' ');

	cout.width(cw_n+2*cw_v); cout<<"# SET PARAMETERS"<<'#'<<endl;
	cout.width(cw_n); cout<<"# Variable name";
	cout.width(cw_v); cout<<"variable's value (a.u.)";
	cout.width(cw_v); cout<<"variable's value (SI)"<<'#'<<endl;
	// ////////// Effective mass //////////
	cout.width(cw_n); cout<<"# m*";
	cout.width(cw_v); cout<<m_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	// ////////// Temperature //////////
	cout.width(cw_n); cout<<"# contact_temp_";
	cout.width(cw_v); cout<<temp_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	// ////////// Lenght //////////
	cout.width(cw_n); cout<<"# L";
	cout.width(cw_v); cout<<l_;
	cout.width(cw_v); cout<<l_*AU_nm<<'#'<<endl;
	cout.width(cw_n); cout<<"# L_C";
	cout.width(cw_v); cout<<lC_;
	cout.width(cw_v); cout<<lC_*AU_nm<<'#'<<endl;
	cout.width(cw_n); cout<<"# L_D";
	cout.width(cw_v); cout<<lD_;
	cout.width(cw_v); cout<<lD_*AU_nm<<'#'<<endl;
	cout.width(cw_n); cout<<"# L_YZ";
	cout.width(cw_v); cout<<lYZ_;
	cout.width(cw_v); cout<<lYZ_*AU_nm*AU_nm<<'#'<<endl;
	cout.width(cw_n); cout<<"# kmax";
	cout.width(cw_v); cout<<kmax_;
	cout.width(cw_v); cout<<kmax_/AU_nm<<'#'<<endl;
	// ////////// Numerical grid parameters //////////
	cout.width(cw_n); cout<<"# Nx";
	cout.width(cw_v); cout<<nx_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	cout.width(cw_n); cout<<"# dx";
	cout.width(cw_v); cout<<dx_;
	cout.width(cw_v); cout<<dx_*AU_nm<<'#'<<endl;
	cout.width(cw_n); cout<<"# Nk";
	cout.width(cw_v); cout<<nk_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	cout.width(cw_n); cout<<"# dk";
	cout.width(cw_v); cout<<dk_;
	cout.width(cw_v); cout<<dk_/AU_nm<<'#'<<endl;
	cout.width(cw_n); cout<<"# dx*dk";
	cout.width(cw_v); cout<<dx_*dk_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	// ////////// Time dependency parameters //////////
	cout.width(cw_n); cout<<"# courant_num";
	cout.width(cw_v); cout<<courant_num_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	cout.width(cw_n); cout<<"# dt";
	cout.width(cw_v); cout<<dt_;
	cout.width(cw_v); cout<<dt_*AU_s<<'#'<<endl;
	// ////////// I-V analisys //////////
	cout.width(cw_n); cout<<"# fermi_energy";
	cout.width(cw_v); cout<<uF_;
	cout.width(cw_v); cout<<uF_*AU_eV<<'#'<<endl;
	// cout.width(cw_n); cout<<"# set_uF";
	// cout.width(cw_v); cout<<(set_uF_ ? "true" : "false");
	// cout.width(cw_v); cout<<(set_uF_ ? "true" : "false")<<'#'<<endl;
	// ////////// Potential //////////
	cout.width(cw_n); cout<<"# use NLP?";
	cout.width(cw_v); cout<<(useNLP_ ? "true" : "false");
	cout.width(cw_v); cout<<(useNLP_ ? "true" : "false")<<'#'<<endl;
	// ////////// Dissipation //////////
	cout.width(cw_n); cout<<"# rR";
	cout.width(cw_v); cout<<rR_;
	cout.width(cw_v); cout<<rR_/AU_s<<'#'<<endl;
	cout.width(cw_n); cout<<"# rM";
	cout.width(cw_v); cout<<rM_;
	cout.width(cw_v); cout<<rM_/AU_s<<'#'<<endl;
	cout.width(cw_n); cout<<"# lambda";
	cout.width(cw_v); cout<<lambda_;
	cout.width(cw_v); cout<<lambda_*AU_nm*AU_nm*AU_s<<'#'<<endl;
	cout.width(cw_n); cout<<"# rG";
	cout.width(cw_v); cout<<rG_;
	cout.width(cw_v); cout<<rG_/AU_s<<'#'<<endl;
	cout.width(cw_n); cout<<"# dist";
	cout.width(cw_v); cout<<bcType_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	cout.width(cw_n); cout<<"# cD";
	cout.width(cw_v); cout<<cD_;
	cout.width(cw_v); cout<<cD_/AU_cm3<<'#'<<endl;
	// cout.width(cw_n); cout<<"# cR";
	// cout.width(cw_v); cout<<cR_;
	// cout.width(cw_v); cout<<'-'<<'#'<<endl;
	cout.width(cw_n); cout<<"# v_bias";
	cout.width(cw_v); cout<<uBias_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	//
	cout.width(cw_n); cout<<"# BC type";
	cout.width(cw_v); cout<<bcType_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	cout.width(cw_n); cout<<"# DS - diffusion";
	cout.width(cw_v); cout<<diffSch_K_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;
	cout.width(cw_n); cout<<"# DS - drift";
	cout.width(cw_v); cout<<diffSch_P_;
	cout.width(cw_v); cout<<'-'<<'#'<<endl;

	cout.fill('=');
	cout.width(cw_n+2*cw_v);
	cout<<'#'<<'#'<<endl;
	cout.fill(' ');
	cout<<endl;
}
