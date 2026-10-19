/*****************************************************************************
 *
 * ALPS DMFT Project
 *
 * Copyright (C) 2005 - 2009 by Emanuel Gull <gull@phys.columbia.edu>
 *                              Philipp Werner <werner@itp.phys.ethz.ch>,
 *                              Sebastian Fuchs <fuchs@theorie.physik.uni-goettingen.de>
 *                              Matthias Troyer <troyer@comp-phys.org>
 *
 *
* ALPS Project: https://alps.comp-phys.org/
* SPDX-License-Identifier: MIT
*
*****************************************************************************/

#include <utility> // Boost.Math 1.76 includes this inside a namespace.
#include <boost/math/constants/constants.hpp>
#include "interaction_expansion.hpp"
#include <ctime>
#include "xml.h"

//global variables

frequency_t c_or_cdagger::nm_;
bool c_or_cdagger::use_static_exp_;
unsigned int c_or_cdagger::ntau_;
double c_or_cdagger::beta_;
double *c_or_cdagger::omegan_;
std::complex<double> *c_or_cdagger::exp_iomegan_tau_;


InteractionExpansionRun::InteractionExpansionRun(const alps::ProcessList &where, const alps::Parameters &parms, int node)
  : InteractionExpansionRun(where, parms, node, alps::input::read_ctint_parameters(parms))
{}

InteractionExpansionRun::InteractionExpansionRun(const alps::ProcessList &where, const alps::Parameters &parms, int node,
                                                 const alps::input::ctint_parameters &input_)
  : alps::scheduler::MCRun(where,parms,node),
    max_order(input_.max_order),       
    n_flavors(input_.flavors),        
    n_site(input_.sites),        
    n_matsubara(input_.n_matsubara),
    n_matsubara_measurements(input_.n_matsubara_measurements),
    n_tau(input_.n_tau),
    n_tau_inv(1./n_tau),
    n_self(input_.n_self),
    mc_steps(input_.mc_steps),
    therm_steps(input_.therm_steps),        
    nruns(input_.nruns),
    max_time_in_seconds(input_.max_time),        
    beta(input_.beta),                        
    temperature(1./beta),
    onsite_U(input_.U),                        
    alpha(input_.alpha),
    U(parms),                         
    recalc_period(input_.recalc_period),
    measurement_period(input_.measurement_period),        
    convergence_check_period(input_.convergence_check_period),        
    almost_zero(input_.almost_zero),                
    seed(input_.seed),
    green_matsubara(n_matsubara, n_site, n_flavors),
    bare_green_matsubara(n_matsubara,n_site, n_flavors), 
    bare_green_itime(n_tau+1, n_site, n_flavors),
    green_itime(n_tau+1, n_site, n_flavors),
    pert_hist(max_order)
{
  //initialize measurement method
  if (input_.histogram_measurement) 
    measurement_method=selfenergy_measurement_itime_rs;
  else 
    measurement_method=selfenergy_measurement_matsubara;
  for(unsigned int i=0;i<n_flavors;++i)
    g0.push_back(green_matrix(n_tau, 20));
  //other parameters
  weight=0;
  sign=1;
  step=0;
  start_time=time(NULL);
  measurement_time=0;
  update_time=0;
  thermalized=therm_steps==0?true:false;
  if(!input_.atomic) {
    std::istringstream in_omega(parms["G0(omega)"]);
    read_freq(in_omega, bare_green_matsubara);
    FourierTransformer::generate_transformer(parms, fourier_ptr);
    fourier_ptr->backward_ft(bare_green_itime, bare_green_matsubara);
  }
  else {
    for(spin_t flavor=0; flavor<n_flavors; ++flavor) 
      for(site_t site1=0; site1<n_site; ++site1) 
        for(site_t site2=0; site2<n_site; ++site2) 
          for(itime_index_t t=0; t<=n_tau; ++t)
            bare_green_itime(t, site1, site2, flavor)=-0.5;
    for(spin_t flavor=0; flavor<n_flavors; ++flavor) 
      for(site_t site1=0; site1<n_site; ++site1) 
        for(site_t site2=0; site2<n_site; ++site2) 
          for(unsigned int k=0; k<n_matsubara; ++k) 
            bare_green_matsubara(k, site1, site2, flavor)=std::complex<double>(0, -beta/((2*k+1)*boost::math::constants::pi<double>()));
    //fourier transform of -1/2
  }
  //initialize the simulation variables
  initialize_simulation(parms);
  print(std::cout);
  vertex_histograms=new simple_hist *[n_flavors*n_flavors];
  vertex_histogram_size=100;
  for(unsigned int i=0;i<n_flavors*n_flavors;++i){
    vertex_histograms[i]=new simple_hist(vertex_histogram_size);
  }
  c_or_cdagger::initialize_simulation(parms);

  if(n_site !=1) throw std::invalid_argument("you're trying to run this code for more than one site. Do you know what you're doing?!?");
}



void InteractionExpansionRun::dostep()
{
    for(unsigned int i=0;i<convergence_check_period;++i){
      //recompute inverse matrix M from scratch to avoid roundoff errors.
      if(step % recalc_period ==0)
        reset_perturbation_series();
      if(step> therm_steps)
        thermalized=true; 
      interaction_expansion_step();                
      if(thermalized && step % measurement_period ==0)
        measure_observables();
      
      //increase perturbation order histogram
      if(vertices.size()<max_order)
        pert_hist[vertices.size()]++;
      ++step;
    }
}



double InteractionExpansionRun::work_done() const{
  //check for error convergence
  if (!thermalized) 
    return 0.;
  if(time(NULL)-start_time> max_time_in_seconds){
    std::cout<<"we ran out of time!"<<std::endl;
    return 1;
  }
  return ((step-therm_steps) / (double) mc_steps)/nruns;
}



///do all the setup that has to be done before running the simulation.
void InteractionExpansionRun::initialize_simulation(const alps::Parameters& parms)
{
  weight=0;
  sign=1;
  //set the right dimensions:
  for(spin_t flavor=0;flavor<n_flavors;++flavor)
    M.push_back(inverse_m_matrix());
  vertices.clear();
  pert_hist.clear();
  //initialize ALPS observables
  initialize_observables();
  green_matsubara=bare_green_matsubara;
  green_itime=bare_green_itime;
}



void c_or_cdagger::initialize_simulation(const alps::Parameters &p)
{
  const alps::input::ctint_parameters input = alps::input::read_ctint_parameters(p);
  beta_=input.beta;
  nm_=input.n_matsubara_measurements;
  omegan_ = new double[nm_];
  for(unsigned int i=0;i<nm_;++i) {
    omegan_[i]=(2.*i+1.)*boost::math::constants::pi<double>()/beta_;
  }
  if(input.tau_discretization_for_exp) {
    ntau_=*input.tau_discretization_for_exp;
    use_static_exp_=true;
    exp_iomegan_tau_=new std::complex<double> [2*nm_*ntau_];
    if(exp_iomegan_tau_==0){
      std::cout<<"not enough memory for computing exp!"<<std::endl;
      exit(0);
    }
    std::cout<<"starting computation of exp values for measurement"<<std::endl;
    for(unsigned int i=0;i<ntau_;++i){
      double tau=i*beta_/(double)ntau_;
      for(unsigned int o=0;o<nm_;++o)
        exp_iomegan_tau_[2*nm_*i + o] = std::complex<double>(cos(omegan_[o]*tau), sin(omegan_[o]*tau));
      for(unsigned int o=0;o<nm_;++o)
        exp_iomegan_tau_[2*nm_*i + nm_ + o] = std::complex<double>(cos(omegan_[o]*tau), -sin(omegan_[o]*tau));
    }
    std::cout<<"done exp computation."<<std::endl;
  } else {
    use_static_exp_=false;
  }
}
