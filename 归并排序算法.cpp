#include <iostream>
using namespace std;

//合并数组,合并是将两个数组排序
void mergeArray(int a[], int first,int mid,int last, int temp[])
{
    
    int i = first, j =  mid + 1, m = mid, n = last, k = 0;
    
    while(i <= m && j <= n)
    {
        if(a[i] <= a[j])
        {
            temp[k++] = a[i++];
        }
        else
        {
            temp[k++] = a[j++];    
        }
    }
    
    while (i <= m)
    {
        temp[k++] = a[i++];
    }
    
    while (j <= n)
    {
        temp[k++] = a[j++];
    }
    
    for(i = 0; i < k; i++)
    {
        a[first + i] = temp[i];
    }
}

//将数组分割
void mergeSort(int a[], int first, int last, int temp[])
{
    if(first < last)
    {
        int mid = (first + last) / 2;
        mergeSort(a, first, mid, temp);
        mergeSort(a, mid + 1, last, temp);
        mergeArray(a, first, mid, last, temp);
    }
}


int main() {
    int arr[10] = {38,2,45,21,54,12,25,45,21,87};
    int *p = new int[10];
    if(p == NULL)
         cout  <<"aaaa"<<endl;
    mergeSort(arr,0,9,p);
    int i;
    for(i = 0; i < 10; i++)
    {
        cout  << arr[i]<<endl;
    }
	
	delete[] p;
	return 0;
}

