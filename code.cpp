#include<bits/stdc++.h>
using namespace std;
struct point{
        double x;
        double y;
    };
double distant(point p1,point p2){
    return sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
}
double jiajiao(double d1,double d2,double d3){
    return acos((d1*d1+d2*d2-d3*d3)/(2*d1*d2));
}
int main(){
    srand(time(0));
    const double PI=M_PI;
    double r=40.0;
    double a1,a2,a3;//三个角度
    a1=(double)rand()/RAND_MAX*2*PI;
    a2=(double)rand()/RAND_MAX*2*PI;
    a3=(double)rand()/RAND_MAX*2*PI;
   point p1,p2,p3;//三个点
   p1.x=r*cos(a1);
   p1.y=r*sin(a1);
   p2.x=r*cos(a2);
   p2.y=r*sin(a2);
   p3.x=r*cos(a3);
   p3.y=r*sin(a3);
   double d12,d13,d23;
    d12=distant(p1,p2);
    d13=distant(p1,p3);
    d23=distant(p2,p3);
    cout<<"p1的夹角为： "<<jiajiao(d12,d13,d23)<<endl;
    cout<<"p2的夹角为： "<<jiajiao(d12,d23,d13)<<endl;
    cout<<"p3的夹角为： "<<jiajiao(d13,d23,d12)<<endl;
    return 0;
}

