#include <rfftw.h>
#include <iostream.h>
#include <fstream.h>
#define N 32

int main()
{
     fftw_real in[N], out[N], f[N];
     rfftw_plan p_in;
     p_in = rfftw_create_plan(N, FFTW_FORWARD, FFTW_ESTIMATE);
     rfftw_plan p_out;
     p_out = rfftw_create_plan(N, FFTW_BACKWARD, FFTW_ESTIMATE);

     for(int i = 0; i < N; i++)
     {
       in[i] = (double)(i%(N/2))/(double)(N/2) - 0.5;
       //cout << i << ", " << in[i] << endl;
     }
     cout << "\n\n\n";
     rfftw_one(p_in, in, out);
     for(int i = 3*N/4; i < N; i++)
     {
        out[i] = 0;
     }
     rfftw_one(p_out, out, f);
     ofstream fout("fft-output.dat");
     for(int i = 0; i < N; i++)
     {
       f[i] = f[i]/(double)N;
       fout << i << '\t' << in[i] << '\t' << f[i] << endl;
     }
     fout.close();
     fftw_destroy_plan(p_in); 
     fftw_destroy_plan(p_out); 
     return(0);
}
