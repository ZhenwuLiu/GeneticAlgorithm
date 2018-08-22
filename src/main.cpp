#include <iostream>
#include <algorithm>
#include <vector>
// #include <iomanip> //控制cout 输出格式
#include "cvplot.h"
#include "opencv2/opencv.hpp"

//生成随机0-1矩阵  Lind<2^32
cv::Mat crtbp(const int Nind,const int Lind){
	if(Lind>31){std::cout<<"编码影响基因小于32!\n";exit(1);}
	cv::Mat Population(cv::Size(Lind,Nind),CV_8UC1,cv::Scalar(0));
	// cv::randu(Population, 0, 2);//并不随机。。
	cv::RNG rng(time(NULL));
	rng.fill(Population, cv::RNG::UNIFORM, 0, 2);//UNIFORM or NORMAL
	return Population;
}
//计算适应度
void ranking(std::vector<float> &objV){
	std::sort(objV.begin(), objV.end(),[](float &a,float &b){if(a>b)return true;else return false;});
}
//选择优秀个体 bug集中地
void select(cv::Mat &Popula,std::vector<float> &rank){
	float sum=.0,base=std::abs(rank[rank.size()-1]);
	std::for_each(rank.begin(), rank.end(), [&](float & i){sum=sum+i+base;});
	std::vector<uchar> new_Popula;
	//保留最优个体+赌盘选择
	for(int i=0;i<Popula.cols;i++)
			new_Popula.push_back(Popula.at<uchar>(0,i));

	for(int i=0;i<rank.size()-1;i++){
		float get_one=(float)(rand()%(int)(sum*1000000))/1000000;//不能使用 float 除数可惜。。
		int _select=0;
		for(float add_sum=0;_select<rank.size();_select++)
			if(add_sum<get_one)add_sum+=(rank[_select]+base);
			else break;
		for(int j=0;j<Popula.cols;j++)
			new_Popula.push_back(Popula.at<uchar>(_select,j));
	}
	int rows=Popula.rows,cols=Popula.cols;
	Popula=cv::Mat(new_Popula);
	cv::resize(Popula, Popula, cv::Size(cols,rows));
}
//交叉  均匀交叉
void recombin(cv::Mat &Popula,const float opt=0.5){
	for(int i=0;i<Popula.rows;i++){
		if(rand()%100>opt*100){
			int j=rand()%Popula.rows;
			for(int k=0;k<Popula.cols;k++){
				if(rand()%2){
					Popula.at<uchar>(i,k)^=Popula.at<uchar>(j,k);
					Popula.at<uchar>(j,k)^=Popula.at<uchar>(i,k);
					Popula.at<uchar>(i,k)^=Popula.at<uchar>(j,k);
				}
			}	
		}
	}
}
//变异 因为概率建立在统计上，不使用迭代所有成员 均匀变异
void mut(cv::Mat &Popula,float opt=0.05){
	for(int i=opt*Popula.cols*Popula.rows;i>0;i--){
		int m=rand()%(Popula.cols*Popula.rows);
		Popula.at<uchar>(m/Popula.cols,m%Popula.cols)^=1;
	}
}
//二进制转十进制 限定区间范围
std::vector<float> bs2rv(cv::Mat &Popula,float min,float max){
	std::vector<float> _ost;
	for(int i=0;i<Popula.rows;i++){
		int sum=0;
		for(int j=0,m=1;j<Popula.cols;j++,m*=2)
			sum+=Popula.at<uchar>(i,j)*m;
		_ost.push_back(min+sum*((float)(max-min)/((1<<Popula.cols)-1))); 
	}
	return _ost;
}

//Y=sin(X)
int main(int argc, const char** argv)
{
	srand(time(NULL));
	cv::Mat Popula,AB;
	std::vector<float> ost;
	//种群越大，越稳定，基因编码越多迭代越稳定
	Popula= crtbp(20,31);
	
	std::vector<float> data;

	for (int i = 0; i < 20; ++i)
	{
		ost=bs2rv(Popula,1,2);
		std::for_each(ost.begin(), ost.end(), [](float &elem){elem=std::sin(10*3.14*elem)/elem;});
		ranking(ost);
		data.push_back(ost[0]);
		select(Popula, ost);
		recombin(Popula);
		mut(Popula);
	}
	cvplot::figure("myplot").series("max").setValue(data);
	cvplot::figure("myplot").show();
	cv::waitKey(0);
    return 0;
}