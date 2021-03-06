/*
########################################################################
#  Netflix Prize Tools
#  Copyright (C) 2007-8 Ehud Ben-Reuven
#  udi@benreuven.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation version 2.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
########################################################################
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "basic.h"
#include "netflix.h"
#include "utest.h"
#include "weight.h"
static char *fnameV="data/usvdsppV.bin";
static char *fnameU="data/usvdsppU.bin";

#define E (0.00020) // stop condition
//#define E (0.0001) // stop condition
//#define E (0.00008) // stop condition
//#define E2 (0.00003) // stop condition
#define E2 (0.00002) // stop condition
int score_argv(char **argv) {return 0;}
extern double rmse(int k);



#define NFEATURES (50)
#define GLOBAL_MEAN (3.6033)
double sU[NUSERS][NFEATURES];
double sV[NMOVIES][NFEATURES];
double sY[NMOVIES][NFEATURES];
double bU[NUSERS]; 
double bV[NMOVIES];
int    mbin[NMOVIES];
float sUbin[NUSERS][9][NFEATURES];
float bUbin[NUSERS][9]; 

//#define G0 (0.005) // for biases
////#define L4 (0.02) // for biases
//#define L4 (0.002) // for biases
//#define L6 (0.005) // for biases
//#define G2 (0.007) 
//#define G2_ORIG (0.007) 
//#define G2_FAST (0.8) 
//#define L7 (0.015) 
////#define G2_Y (0.8) 
////#define L7_Y (0.015) 
////#define L7 (0.028) 
////#define L8 (0.035) 
//#define L8 (0.015) 


//#define G1 (0.007) 
//#define G2 (0.007) 
#define G1 (0.007) 
#define G2 (0.007) 
//#define L6 (0.005) // for biases
//#define L7 (0.015) 
#define L6 (0.005) // for biases
#define L7 (0.015) 

double mavg[NMOVIES];

FILE *fpV=NULL, *fpU=NULL;
int moviecount[NMOVIES];


int n[NMOVIES];
double mean[NMOVIES];
double M2[NMOVIES];
double variance[NMOVIES];
double stddev[NMOVIES];




void score_setup()
{
	int i,u;
    //weight_time_setup();
	if(load_model) {
		fpV=fopen(fnameV,"rb");
		fpU=fopen(fnameU,"rb");
		if(fpV || fpU) {
			lg("Loading %s and %s\n",fnameV,fnameU);
			if(!fpV || !fpU)
				error("Cant open both files");
		}
	}
	
	ZERO(moviecount);
	//for(u=0;u<NUSERS;u++) {
		//int base=useridx[u][0];
		//int d=UNTRAIN(u);
		//int i;
		//for(i=0; i<d;i++) {
			//int m=userent[base+i]&USER_MOVIEMASK;
			//moviecount[m]++;
            //int r=(userent[base+i]>>USER_LMOVIEMASK)&7;
			//r++;
			//mavg[m] += r;
		//}
	//}

	//int above=0, below=0, middle=0;
	//for (i=0; i < NMOVIES; i++) {
		//mavg[i] /= moviecount[i];
		////if ( mavg[i] > (GLOBAL_MEAN + 0.5) )
		//if ( mavg[i] >= 3.85 ) {
			//above++;
			//mbin[i] = 2;
		//} else if ( mavg[i] < 2.95 ) {
			//below++;
			//mbin[i] = 0;
		//} else {
			//middle++;
			//mbin[i] = 1;
		//}
	//}
	//printf("Above: %d, Below: %d, Middle: %d\n", above, below, middle);
	//printf("Above: %d, Below: %d, Middle: %d\n", 100*above/NMOVIES, 100*below/NMOVIES, 100*middle/NMOVIES);
	//fflush(stdout);

	ZERO(n);
	ZERO(mean);
	ZERO(M2);

	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d=UNTRAIN(u);
		int i;
		for(i=0; i<d;i++) {
			int m=userent[base+i]&USER_MOVIEMASK;
			moviecount[m]++;
            int r=(userent[base+i]>>USER_LMOVIEMASK)&7;
			r++;
			mavg[m] += r;
 
    		double delta = r - mean[m];
    		mean[m] = mean[m] + delta/moviecount[m];
    		M2[m] = M2[m] + delta*(r - mean[m]);  // This expression uses the new value of mean
		}
	}

	int lvl[10];
	ZERO(lvl);
	for (i=0; i < NMOVIES; i++) {
		mavg[i] /= moviecount[i];
		variance[i] = M2[i]/(moviecount[i] - 1);
		stddev[i] = sqrt(variance[i]);
		int sdevv;
		int combo;
		if ( stddev[i] >= 1.20 ) {
			sdevv = 2;
		} else if ( stddev[i] < 1.0 ) {
			sdevv = 0;
		} else {
			sdevv = 1;
		}
		if ( mavg[i] >= 3.85 ) {
			combo = sdevv+ 3*2;
			mbin[i] = combo;
			lvl[combo]++;
		} else if ( mavg[i] < 2.95 ) {
			combo = sdevv+ 3*0;
			mbin[i] = combo;
			lvl[combo]++;
		} else {
			combo = sdevv+ 3*1;
			mbin[i] = combo;
			lvl[combo]++;
		}

	}
	for (i=0; i < 10; i++) {
		printf("lvl: %d\tCnt: %d\tPct: %f\n", i, lvl[i], 100.0 * ((double)lvl[i]) / NMOVIES);
	}
	fflush(stdout);

}


/*
double rmseprobe()
{
	int k=2;
    int u, f;
    int n=0;
    double s=0.;
    int i;

	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];

		int dall=UNALL(u);
		double NuS = 1.0/sqrt(dall);
		double lNuSY[NFEATURES];
		double sumY[NFEATURES];
		ZERO(sumY);
		ZERO(lNuSY);
		int j;
		for(j=0;j<dall;j++) {
			int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++)
				sumY[f]+=sY[mm][f];
		}
		int d0=UNTRAIN(u);
		//for(j=0;j<d0;j++) {
			//int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++) 
				lNuSY[f] = NuS * sumY[f]; 
		//}

		int base=useridx[u][0];
		for(i=1;i<k;i++) base+=useridx[u][i];
		int d=useridx[u][k];
		//s+=fvsqr(&err[base],d);

		for(i=0; i<d;i++) {
			int m=userent[base+i]&USER_MOVIEMASK;
			double e=err[base+i];
			e-=(bU[u] + bV[m]);
			for (f=0; f<NFEATURES; f++)
			    e-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
			s+=e*e;
		}
		n+=d;
	}
    return sqrt(s/n);
}
*/

void removeUV()
{
	int u,f;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNALL(u);
		int i;

		int dall=UNALL(u);
		double NuS = 1.0/sqrt(dall);
		double lNuSY[NFEATURES];
		double sumY[NFEATURES];
		ZERO(sumY);
		ZERO(lNuSY);
		int j;
		for(j=0;j<dall;j++) {
			int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++)
				sumY[f]+=sY[mm][f];
		}
		int d0=UNTRAIN(u);
		//for(j=0;j<d0;j++) {
			//int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++) 
				lNuSY[f] = NuS * sumY[f]; 
		//}


        double bUu=bU[u];
		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;
			//err[base0+i]-=(bU[u] + bV[m]);
			//for (f=0; f<NFEATURES; f++)
			    //err[base0+i]-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
			err[base0+i]-= (bU[u] + bUbin[u][mbin[m]] + bV[m]);
			for (f=0; f<NFEATURES; f++)
			    err[base0+i]-= ((sU[u][f]+sUbin[u][mbin[m]][f]+lNuSY[f])*sV[m][f]);
		}
	}
}

int score_train(int loop)
{
	if (loop == 0)
		return doAllFeatures();
	
	return 1;
}

int doAllFeatures()
{
	/* Initial biases */
	{
		int u,m;
		
		for(u=0;u<NUSERS;u++) {
			//bU[u]=drand48()*0.01-0.005;
			bU[u]=0.0;
			bUbin[u][0]=0.0;
			bUbin[u][1]=0.0;
			bUbin[u][2]=0.0;
		}
		for(m=0;m<NMOVIES;m++) {
			//bV[m]=drand48()*0.01-0.005;
			bV[m]=0.0;
		}
	}
	
	
	/* Initial estimation for current feature */
	{
		int u,m,f;
		
		double uvInit = sqrt(GLOBAL_MEAN/NFEATURES);
		for(u=0;u<NUSERS;u++) {
			for(f=0;f<NFEATURES;f++) {
			    //sU[u][f]=drand48()*0.1-0.04;
			    sU[u][f]= uvInit * (rand()%14000 + 2000) * 0.000001235f;
			    sUbin[u][0][f]= uvInit * (rand()%14000 + 2000) * 0.000001235f * 0.001;
			    sUbin[u][1][f]= uvInit * (rand()%14000 + 2000) * 0.000001235f * 0.001;
			    sUbin[u][2][f]= uvInit * (rand()%14000 + 2000) * 0.000001235f * 0.001;
			}
		}
		for(m=0;m<NMOVIES;m++) {
			for(f=0;f<NFEATURES;f++) {
			    //sV[m][f]=drand48()*0.05-0.025;
			    sV[m][f]= uvInit * (rand()%14000 + 2000) * -0.000001235f;
			    //sY[m][f]=drand48()*0.02-0.01;
			    sY[m][f]=0.0;
			}
		}
	}
	
	/* Optimize current feature */
	double nrmse=2., last_rmse=10.;
	double prmse = 0, last_prmse=0;
	double thr=sqrt(1.-E);
	int loopcount=0;
	    //thr=sqrt(1.-E2);
	//double Gamma2 = G2;
	//double Gamma0 = G0;
	double Gamma1 = G1;
	double Gamma2 = G2;
	while( ((nrmse < (last_rmse-E) && prmse<last_prmse) || loopcount < 20) && loopcount < 40  )  {
		last_rmse=nrmse;
		last_prmse=prmse;
		clock_t t0=clock();
		loopcount++;

		int u,m, f;
		for(u=0;u<NUSERS;u++) {

			// Calculate sumY and NuSY for each factor
			double sumY[NFEATURES];
			ZERO(sumY);
			double lNuSY[NFEATURES];
			ZERO(lNuSY);
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;
			int f;
			int dall=UNALL(u);
			double NuS = 1.0/sqrt(dall);
			for(j=0;j<dall;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++)
					sumY[f]+=sY[mm][f];
			}
//if ( loopcount > 1 ) {
//printf("sumY: %f\n", sumY);
//fflush(stdout);
//}
			//for(j=0;j<d0;j++) {
				//int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++) {
					lNuSY[f] = NuS * sumY[f]; 
//if ( loopcount > 1 ) {
//printf("lNuSY: %f\n", lNuSY[f]);
//fflush(stdout);
//}
				}
			//}

			double ycontrib[NFEATURES];
			ZERO(ycontrib);

			// For all rated movies
			//double bdampen = d0/1.4;
			double bdampen = 1.0;
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;

				// Figure out the current error
				double ee=err[base0+j];
				double e2 = ee;
				e2 -= (bU[u] + bUbin[u][mbin[m]] + bV[m]);
				for (f=0; f<NFEATURES; f++)
					e2 -= ((sU[u][f]+sUbin[u][mbin[m]][f]+lNuSY[f])*sV[m][f]);

                int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
                r++;
                double rui = r - e2;
                if ( rui > 5.00 )
                    e2 += (rui-5.0);
                else if (rui < 1.0)
                    e2 -= (1.0 - rui);

				// Train the biases
				double bUu = bU[u];
				double bUubin = bUbin[u][mbin[m]];
				double bVm = bV[m];
				bU[u] += Gamma1 * (e2 - bUu * L6) / bdampen;
				//bUbin[u][mbin[m]] += Gamma1 * (e2 - bUubin * L6) / bdampen / 2.5;
				bUbin[u][mbin[m]] += Gamma1 * (e2 - bUubin * L6) / bdampen / 9.0;
				bV[m] += Gamma1 * (e2 - bVm * L6) / bdampen;
//printf("bU: %f\n", bU[u]);
//printf("bV: %f\n", bV[m]);
//fflush(stdout);

				// update U V and slope component of Y
				//double yfactor = NuS/sqrt(moviecount[m]); 
				//double yfactor = NuS;
				//double yfactor = NuS/sqrt(d0);
				double yfactor = NuS;
				for (f=0; f<NFEATURES; f++) {
					double sUu = sU[u][f];
					double sUubin = sUbin[u][mbin[m]][f];
					double sVm = sV[m][f];

					sU[u][f] += ((Gamma2) * ((e2 * sVm) - L7 * sUu));
					sUbin[u][mbin[m]][f] += (((Gamma2) / 9.0) * ((e2 * sVm) - L7 * sUubin));
					//sUbin[u][mbin[m]][f] += (((Gamma2*1.2) / 1.1) * ((e2 * sVm) - L7 * sUubin));
					//sV[m][f] += ((Gamma2/2.5) * ((e2 * (sUu + sUubin + lNuSY[f])) - L7 * sVm)); comb version with this
					sV[m][f] += ((Gamma2) * ((e2 * (sUu + sUubin + lNuSY[f])) - L7 * sVm));
//printf("sU: %f\n", sU[u][f]);
//printf("sV: %f\n", sV[m][f]);
//fflush(stdout);

					ycontrib[f] += e2 * sVm * yfactor;
//printf("ycont: %f\n", ycontrib[f]);
//fflush(stdout);
				}
			}

			// Train Ys over all known movies for user
			for(j=0;j<dall;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				for (f=0; f<NFEATURES; f++) {
					double sYm = sY[m][f];
					sY[m][f] += Gamma2 * (ycontrib[f] - L7 * sYm);
//printf("before sY: %f\tycon: %f\tG2*ycon: %f\treg: %f\n", sY[m][f], ycontrib[f], (G2_Y*ycontrib[f]), G2_Y*L7_Y*sYm);
//printf("after sY: %f\tycon: %f\tG2*ycon: %f\treg: %f\n", sY[m][f], ycontrib[f], (G2_Y*ycontrib[f]), G2_Y*L7_Y*sYm);
//printf("sY: %f\tycon: %f\tG2*ycon: %f\n", sY[m][f], ycontrib[f], (G2*ycontrib[f]));
//fflush(stdout);
				}
			}
		}

		// Report rmse for main loop
		nrmse=0.;
		int ntrain=0;
		int elcnt=0;
		int k=2;
		int n=0;
		double s=0.;
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;

			// Setup the Ys again
			double sumY[NFEATURES];
			ZERO(sumY);
			double lNuSY[NFEATURES];
			ZERO(lNuSY);
			int dall=UNALL(u);
			double NuS = 1.0/sqrt(dall);
			for(j=0;j<dall;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++)
					sumY[f]+=sY[mm][f];
			}
			//for(j=0;j<d0;j++) {
				//int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++) 
					lNuSY[f] = NuS * sumY[f]; 
			//}

			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				double ee = err[base0+j];
				double e2 = ee;
				//e2 -= (bU[u] + bV[m]);
				//for (f=0; f<NFEATURES; f++)
					//e2 -= ( (sU[u][f] + lNuSY[f]) * sV[m][f]);
				e2 -= (bU[u] + bUbin[u][mbin[m]] + bV[m]);
				for (f=0; f<NFEATURES; f++)
					e2 -= ((sU[u][f]+sUbin[u][mbin[m]][f]+lNuSY[f])*sV[m][f]);

                int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
                r++;
                double rui = r - e2;
                if ( rui > 5.00 )
                    e2 += (rui-5.0);
                else if (rui < 1.0)
                    e2 -= (1.0 - rui);

if( elcnt++ == 5000 ) {
    //printf("0 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[0], sV[m][0], sU[u][0], bU[u], bV[m], sY[m][0],u, m);
    printf("0 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\tsUb %f %f %f\tbUb: %f %f %f\n", ee, e2, lNuSY[0], sV[m][0], sU[u][0], bU[u], bV[m], sY[m][0],u,
	  m, (double)sUbin[u][0][0],(double)sUbin[u][1][0],(double)sUbin[u][2][0],(double)bUbin[u][0],(double)bUbin[u][1],(double)bUbin[u][2]);
    printf("1 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[1], sV[m][1], sU[u][1], bU[u], bV[m], sY[m][1],u, m);
    printf("2 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[2], sV[m][2], sU[u][2], bU[u], bV[m], sY[m][2],u, m);
    printf("3 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[3], sV[m][3], sU[u][3], bU[u], bV[m], sY[m][3],u, m);
	fflush(stdout);
}
/*
if( e > 5.0 || e < -5.0 ) {
    printf("bad EE: %f\tU: %d\tM: %d\tNuSY: %f\te: %f\t sV: %f\tsU: %f\tbU: %f\tbV: %f\n", ee, u, m, NuSY, e, new_sV[m], new_sU[u], bUu, bVm);
	fflush(stdout);
}
*/

				nrmse+=e2*e2;
			}
			ntrain+=d0;

			// Sum up probe rmse
			int i;
			int base=useridx[u][0];
			for(i=1;i<k;i++) base+=useridx[u][i];
			int d=useridx[u][k];
			for(i=0; i<d;i++) {
				int m=userent[base+i]&USER_MOVIEMASK;
				double e=err[base+i];
				//e-=(bU[u] + bV[m]);
				//for (f=0; f<NFEATURES; f++)
					//e-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
				e -= (bU[u] + bUbin[u][mbin[m]] + bV[m]);
				for (f=0; f<NFEATURES; f++)
					e -= ((sU[u][f]+sUbin[u][mbin[m]][f]+lNuSY[f])*sV[m][f]);

                int r=(userent[base+i]>>USER_LMOVIEMASK)&7;
                r++;
                double rui = r - e;
                if ( rui > 5.00 )
                    e += (rui-5.0);
                else if (rui < 1.0)
                    e -= (1.0 - rui);

				s+=e*e;
			}
			n+=d;
		}
		nrmse=sqrt(nrmse/ntrain);
		//double prmse = rmseprobe();
		prmse = sqrt(s/n);
		
		lg("%f\t%f\t%f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
		//rmse_print(0);
		if ( loopcount < 5 ) {
		    Gamma1 *= 0.90;
		    Gamma2 *= 0.90;
		} else if ( loopcount < 10 ) {
		    Gamma1 *= 0.90;
		    Gamma2 *= 0.90;
		} else if ( loopcount < 14 ) {
		    Gamma1 *= 0.90;
		    Gamma2 *= 0.90;
		} else if ( loopcount < 18 ) {
		    Gamma1 *= 0.90;
		    Gamma2 *= 0.90;
		} else if ( loopcount < 22 ) {
		    Gamma1 *= 0.90;
		    Gamma2 *= 0.90;
		} else if ( loopcount < 26 ) {
		    Gamma1 *= 0.90;
		    Gamma2 *= 0.90;
		} else {
		    Gamma1 *= 0.90;
		    Gamma2 *= 0.90;
		}
		/*
		if ( loopcount < 14 ) {
		    Gamma1 *= 0.98;
		    Gamma2 *= 0.98;
		} else if ( loopcount < 18 ) {
		    Gamma1 *= 0.97;
		    Gamma2 *= 0.97;
		} else if ( loopcount < 26 ) {
		    Gamma1 *= 0.96;
		    Gamma2 *= 0.96;
		} else if ( loopcount < 34 ) {
		    Gamma1 *= 0.95;
		    Gamma2 *= 0.95;
		} else if ( loopcount < 37 ) {
		    Gamma1 *= 0.94;
		    Gamma2 *= 0.94;
		} else if ( loopcount < 38 ) {
		    Gamma1 *= 0.93;
		    Gamma2 *= 0.93;
		} else {
		    Gamma1 *= 0.91;
		    Gamma2 *= 0.91;
		}
		*/
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	//if(save_model) {
		//dappend_bin(fnameV,sV,NMOVIES);
		//dappend_bin(fnameU,sU,NUSERS);
	//}
	
	return 1;
}
