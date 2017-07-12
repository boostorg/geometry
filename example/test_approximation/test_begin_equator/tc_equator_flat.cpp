/*************************************************************************
    > File Name: tc_equator_flat.cpp
    > Author: Rylynnn
    > Mail: jingry0321@gmail.com
    > Created Time: 2017年07月13日 星期四 00时55分03秒
 ************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;
int main()
{
	freopen("tc_begin_equator_flat.in", "w", stdout);
    double a, b, c, d, e, f, g, h;
    for (int i=0; i<90; i++){
        a = 0;
        b = 0;
        c = 180;
		d = i;
        for (int j=0; j<90; j++){
		    e = 0;
            f = 0;
            g = 170;
            h = j;
			cout << a << ' ' << b << " " 
                 << c << " " << d << ' ' 
                 << e << ' ' << f << ' ' 
                 << g << ' ' << h << endl;	
        }
        for (int j=90; j>0; j--){
		    e = 0;
            f = 0;
            g = 10;
            h = j;
			cout << a << ' ' << b << " " 
                 << c << " " << d << ' ' 
                 << e << ' ' << f << ' ' 
                 << g << ' ' << h << endl;	
        }
	}
   for (int i=90; i>0; i--){
        a = 0;
        b = 0;
        c = 0;
		d = i;
        for (int j=0; j<90; j++){
		    e = 0;
            f = 0;
            g = 170;
            h = j;
			cout << a << ' ' << b << " " 
                 << c << " " << d << ' ' 
                 << e << ' ' << f << ' ' 
                 << g << ' ' << h << endl;	
        }
        for (int j=90; j>0; j--){
		    e = 0;
            f = 0;
            g = 10;
            h = j;
			cout << a << ' ' << b << " " 
                 << c << " " << d << ' ' 
                 << e << ' ' << f << ' ' 
                 << g << ' ' << h << endl;	
        }
	}
    return 0;
}
