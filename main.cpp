#include <iostream>
#include <chrono>		//chrono::milliseconds(1000); need scope chrono
#include <thread>
#include "ads1015.h"

using namespace std;

int main()
{
    cout << "Hello world!" << endl;

    ads1015 myTest(1,0x48);
    int loopcount = 0;
    float conversion =0;
    cout<<"===read_config_reg() "<< myTest.read_config_reg()<<endl;
    do
    {
        conversion = myTest.read_conversion();
        cout<<"conversion: "<<conversion<<endl;
        loopcount++;
        this_thread::sleep_for(chrono::seconds(1));
    }
    while(loopcount <3);


    return 0;
}
