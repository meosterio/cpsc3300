#include <stdio.h>
#include <stdlib.h>

double a[1000][1000],b[1000][1000],c[1000][1000],d[1000][1000];

int main(){
  int i,j,k;
  double temp,sum,row_sum;
/* set coefficients so that result matrix should have
 * row entries equal to (1/2)*n*(n-1)*i in row i
 */
  for (i=0;i<1000;i++){
    for (j=0;j<1000;j++){
      a[i][j] = b[i][j] = (double) i;
    }
  }
/* try to flush cache */
  for(i=0;i<1000;i++){
    for (j=0;j<1000;j++){
      d[i][j] = 0.0;
    }
  }
  for(i=0;i<1000;i++){
    for(j=0;j<1000;j++){
      c[i][j]=0.0;
    }
  }
  for(i=0;i<1000;i++){
    for(k=0;k<1000;k++){
      temp = a[i][k];
      for(j=0;j<1000;j++){
        c[i][j] += temp * b[k][j];
      }
    }
  }
/* check result */
  sum = 0.5*((double)(1000*(1000-1)));
  for (i=0;i<1000;i++){
    row_sum = ((double)i)*sum;
    for (j=0;j<1000;j++){
      if (c[i][j]!=row_sum){
        printf("error in result c[%d][%d]: %e != %e\n",i,j,c[i][j],row_sum);
        exit(1);
      }
    }
  }
  return(0);
}
