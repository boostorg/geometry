/*************************************************************************
    > File Name: tc_equator.cpp
    > Author: Rylynnn
    > Mail: jingry0321@gmail.com
    > Created Time: 2017年07月12日 星期三 19时29分46秒
 ************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;
int main()
{
	freopen("tc_begin_equator.in", "w", stdout);
    double a, b, c, d, e, f, g, h;
    for (int i=0; i<90; i++){
        a = 0;
        b = 0;
        c = 180;
		d = i;
        for (int j=0; j<90; j++){
		    e = 10;
            f = 0;
            g = 170;
            h = j;
			cout << a << ' ' << b << " " 
                 << c << " " << d << ' ' 
                 << e << ' ' << f << ' ' 
                 << g << ' ' << h << endl;	
        }
        for (int j=90; j>0; j--){
		    e = 10;
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
		    e = 10;
            f = 0;
            g = 170;
            h = j;
			cout << a << ' ' << b << " " 
                 << c << " " << d << ' ' 
                 << e << ' ' << f << ' ' 
                 << g << ' ' << h << endl;	
        }
        for (int j=90; j>0; j--){
		    e = 10;
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
