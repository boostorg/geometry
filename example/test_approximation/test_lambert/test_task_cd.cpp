/*************************************************************************
    > File Name: test_task_cd.cpp
    > Author: Rylynnn
    > Mail: jingry0321@163.com
    > Created Time: Tue 13 Jun 2017 03:34:06 AM CST
 ************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;
int main()
{
	freopen("testcase.in", "w", stdout);
	srand( time (NULL));
    int	t1 = 180, t2 = -180, tt1 = 90, tt2 = -90;
	int n = 100000;
	while(n--){
		double a, b, c, d;
		a = rand() % (t1 - t2 + 1) + t2 + rand() / double(RAND_MAX);	
		b = rand() % (tt1 - tt2 + 1) + tt2 + rand() / double(RAND_MAX);	
		c = rand() % (t1 - t2 + 1) + t2 + rand() / double(RAND_MAX);	
		d = rand() % (tt1 - tt2 + 1) + tt2 + rand() / double(RAND_MAX);
		if(a >= -180.0 && a <= 180.0 && b >= -90.0 && b<= 90.0 && c >= -180.0 && c <= 180.0 && d >= -90.0 && d <= 90.0){
			cout << a << ' ' << b << " " << c << " " << d << ' ';	
		}
		a = rand() % (t1 - t2 + 1) + t2 + rand() / double(RAND_MAX);	
		b = rand() % (tt1 - tt2 + 1) + tt2 + rand() / double(RAND_MAX);	
		c = rand() % (t1 - t2 + 1) + t2 + rand() / double(RAND_MAX);	
		d = rand() % (tt1 - tt2 + 1) + tt2 + rand() / double(RAND_MAX);
		if(a >= -180.0 && a <= 180.0 && b >= -90.0 && b<= 90.0 && c >= -180.0 && c <= 180.0 && d >= -90.0 && d <= 90.0){
			cout << a << ' ' << b << " " << c << " " << d << endl;	
		}
	}	
	return 0;
}
