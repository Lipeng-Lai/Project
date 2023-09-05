#include <iostream>
#include <fstream>
#include <algorithm>

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;

int WWD_LLP_YEAR = 2023;
int WWD_LLP_MONTH = 8;
int WWD_LLP_DAY = 21;

//                    1   2  3   4   5   6   7   8   9   10  11   12
int Months[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

bool Is_leapYear(int year)
{
    return (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0));
}

int main()
{
    cout << "🥚子和🐭子在一起的纪念日为2023年8月21日" << endl;

    ofstream outputFile("test.txt"); // 创建输出文件流

    int cnt = 0;
    for (int i = 99; i <= 29999; i += 100)
    {
        outputFile << i << "天" << std::endl;
        int days = i - cnt * 100;
        while (days > 0)
        {
            if (Is_leapYear(WWD_LLP_YEAR)) Months[2] = 29;
            else Months[2] = 28;

            if (WWD_LLP_DAY == Months[WWD_LLP_MONTH])
            {
                if (WWD_LLP_MONTH == 12) WWD_LLP_YEAR++, WWD_LLP_MONTH = 1, WWD_LLP_DAY = 1;
                else WWD_LLP_MONTH++, WWD_LLP_DAY = 1;
            }
            else WWD_LLP_DAY++;

            days--;
        }

        outputFile << WWD_LLP_YEAR << "年" << WWD_LLP_MONTH << "月" << WWD_LLP_DAY << "日" << std::endl;
        cnt++;
    }

    outputFile.close(); // 关闭文件流

    return 0;
}
