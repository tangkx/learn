#include <iostream>
#include <vector>
using namespace std;
#define N 5
void backpack(vector<int>& weightV, vector<int>& valueV, int weight, vector<vector<int>>& result)
{
    int wSize = weightV.size();
    for(int i = 1; i <= wSize; ++i)
    {
        for(int j = 1; j <= weight; ++j)
        {
            if(weightV[i] > j)
            {
                result[i][j] = result[i-1][j];
            }
            else
            {
                int packV = result[i-1][j-weightV[i]] + valueV[i];
                int notPackV = result[i-1][j];
                result[i][j] = packV>notPackV?packV:notPackV;
            }
        }
    }
}
int main() {
    int weight = 8;
	vector<int> weightV(1,0);
	weightV.push_back(0);
	weightV.push_back(1);
	weightV.push_back(2);
	weightV.push_back(3);
	weightV.push_back(4);
	weightV.push_back(5);
	vector<int> valueV(1,0);
	valueV.push_back(0);
	valueV.push_back(2);
	valueV.push_back(3);
	valueV.push_back(5);
	valueV.push_back(6);
	valueV.push_back(9);
	vector<vector<int>> result(6,vector<int>(weight+1,0));
	backpack(weightV,valueV,weight,result);
	return 0;
}
